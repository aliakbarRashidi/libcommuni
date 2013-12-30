/*
* Copyright (C) 2008-2014 The Communi Project
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
* License for more details.
*/

#include "ircbase.h"
#include "ircbase_p.h"
#include "ircbuffermodel_p.h"
#include "ircconnection.h"
#include "ircchannel_p.h"
#include "ircbuffer_p.h"
#include "ircmessage.h"
#include "irccommand.h"
#include <qmetatype.h>
#include <qmetaobject.h>
#include <qpointer.h>

IRC_BEGIN_NAMESPACE

class IrcBufferLessThan
{
public:
    IrcBufferLessThan(IrcBufferModel* model, Irc::SortMethod method) : model(model), method(method) { }
    bool operator()(IrcBuffer* b1, IrcBuffer* b2) const { return model->lessThan(b1, b2, method); }
private:
    IrcBufferModel* model;
    Irc::SortMethod method;
};

class IrcBufferGreaterThan
{
public:
    IrcBufferGreaterThan(IrcBufferModel* model, Irc::SortMethod method) : model(model), method(method) { }
    bool operator()(IrcBuffer* b1, IrcBuffer* b2) const { return model->lessThan(b2, b1, method); }
private:
    IrcBufferModel* model;
    Irc::SortMethod method;
};

IrcBasePrivate::IrcBasePrivate()
{
}

void IrcBasePrivate::init(const QString& title, IrcConnection* c, IrcBufferModel* m)
{
    Q_Q(IrcBase);
    IrcBufferPrivate::init(title, c, m);

    c->installMessageFilter(this);
    c->installCommandFilter(this);
    c->connect(c, SIGNAL(connected()), q, SLOT(_irc_connected()));
    c->connect(c, SIGNAL(disconnected()), q, SLOT(_irc_disconnected()));
}

bool IrcBasePrivate::messageFilter(IrcMessage* msg)
{
    Q_Q(IrcBase);
    if (msg->type() == IrcMessage::Join && msg->flags() & IrcMessage::Own)
        createBuffer(static_cast<IrcJoinMessage*>(msg)->channel());

    bool delivered = false;
    switch (msg->type()) {
        case IrcMessage::Nick:
        case IrcMessage::Quit:
            foreach (IrcBuffer* buffer, bufferList) {
                if (buffer->isActive())
                    processMessage(msg);
            }
            delivered = true;
            break;

        case IrcMessage::Join:
        case IrcMessage::Part:
        case IrcMessage::Kick:
        case IrcMessage::Names:
        case IrcMessage::Topic:
            delivered = deliverMessage(msg->property("channel").toString(), msg);
            break;

        case IrcMessage::WhoReply:
            delivered = deliverMessage(static_cast<IrcWhoReplyMessage*>(msg)->mask(), msg);

        case IrcMessage::Private:
            if (IrcPrivateMessage* pm = static_cast<IrcPrivateMessage*>(msg))
                delivered = !pm->isRequest() && (deliverMessage(pm->target(), pm) || deliverMessage(pm->nick(), pm, true));
            break;

        case IrcMessage::Notice:
            if (IrcNoticeMessage* no = static_cast<IrcNoticeMessage*>(msg))
                delivered = !no->isReply() && (deliverMessage(no->target(), no) || deliverMessage(no->nick(), no));
            break;

        case IrcMessage::Mode:
            delivered = deliverMessage(static_cast<IrcModeMessage*>(msg)->target(), msg);
            break;

        case IrcMessage::Numeric:
            // TODO: any other special cases besides RPL_NAMREPLY?
            if (static_cast<IrcNumericMessage*>(msg)->code() == Irc::RPL_NAMREPLY) {
                const int count = msg->parameters().count();
                const QString channel = msg->parameters().value(count - 2);
                delivered = deliverMessage(channel, msg);
            } else {
                delivered = deliverMessage(msg->parameters().value(1), msg);
            }
            break;

        default:
            break;
    }

    if (!delivered)
        emit q->messageIgnored(msg);

    if (msg->type() == IrcMessage::Part && msg->flags() & IrcMessage::Own) {
        destroyBuffer(static_cast<IrcPartMessage*>(msg)->channel());
    } else if (msg->type() == IrcMessage::Kick) {
        const IrcKickMessage* kickMsg = static_cast<IrcKickMessage*>(msg);
        if (!kickMsg->user().compare(msg->connection()->nickName(), Qt::CaseInsensitive))
            destroyBuffer(kickMsg->channel());
    }

    return false;
}

bool IrcBasePrivate::commandFilter(IrcCommand* cmd)
{
    if (cmd->type() == IrcCommand::Join) {
        const QString channel = cmd->parameters().value(0).toLower();
        const QString key = cmd->parameters().value(1);
        if (!key.isEmpty())
            keys.insert(channel, key);
        else
            keys.remove(channel);
    }
    return false;
}

IrcBuffer* IrcBasePrivate::createBuffer(const QString& title)
{
    IrcBuffer* buffer = bufferMap.value(title.toLower());
    if (!buffer) {
        if (model && connection && connection->network()->isChannel(title))
            buffer = IrcBufferModelPrivate::get(model)->createChannelHelper(connection, title);
        else
            buffer = IrcBufferModelPrivate::get(model)->createBufferHelper(connection, title);
        if (buffer) {
            IrcBufferPrivate::get(buffer)->init(title, connection, model);
            addBuffer(buffer);
        }
    }
    return buffer;
}

void IrcBasePrivate::destroyBuffer(const QString& title, bool force)
{
    IrcBuffer* buffer = bufferMap.value(title.toLower());
    if (buffer && (force || !buffer->isPersistent())) {
        removeBuffer(buffer);
        buffer->deleteLater();
    }
}

void IrcBasePrivate::addBuffer(IrcBuffer* buffer, bool notify)
{
    insertBuffer(-1, buffer, notify);
}

void IrcBasePrivate::insertBuffer(int index, IrcBuffer* buffer, bool notify)
{
    Q_Q(IrcBase);
    if (buffer && !bufferList.contains(buffer)) {
        const QString title = buffer->title();
        const QString lower = title.toLower();
        if (bufferMap.contains(lower)) {
            qWarning() << "IrcBufferModel: ignored duplicate buffer" << title;
            return;
        }
        IrcBufferPrivate::get(buffer)->setModel(model);
        IrcBufferPrivate::get(buffer)->setConnection(connection);
        const bool isChannel = buffer->isChannel();
        if (model->sortMethod() != Irc::SortByHand) {
            QList<IrcBuffer*>::iterator it;
            if (model->sortOrder() == Qt::AscendingOrder)
                it = qUpperBound(bufferList.begin(), bufferList.end(), buffer, IrcBufferLessThan(model, model->sortMethod()));
            else
                it = qUpperBound(bufferList.begin(), bufferList.end(), buffer, IrcBufferGreaterThan(model, model->sortMethod()));
            index = it - bufferList.begin();
        } else if (index == -1) {
            index = bufferList.count();
        }
        if (notify)
            emit q->aboutToBeAdded(buffer);
        model->beginInsertRows(QModelIndex(), index, index);
        bufferList.insert(index, buffer);
        bufferMap.insert(lower, buffer);
        if (isChannel) {
            channels += title;
            if (keys.contains(lower))
                IrcChannelPrivate::get(buffer->toChannel())->setKey(keys.take(lower));
        }
        q->connect(buffer, SIGNAL(destroyed(IrcBuffer*)), q, SLOT(_irc_bufferDestroyed(IrcBuffer*)));
        model->endInsertRows();
        if (notify) {
            emit q->added(buffer);
            if (isChannel)
                emit q->channelsChanged(channels);
            emit q->buffersChanged(bufferList);
            emit q->countChanged(bufferList.count());
        }
    }
}

void IrcBasePrivate::removeBuffer(IrcBuffer* buffer, bool notify)
{
    Q_Q(IrcBase);
    int idx = bufferList.indexOf(buffer);
    if (idx != -1) {
        const bool isChannel = buffer->isChannel();
        if (notify)
            emit q->aboutToBeRemoved(buffer);
        model->beginRemoveRows(QModelIndex(), idx, idx);
        bufferList.removeAt(idx);
        bufferMap.remove(buffer->title().toLower());
        if (isChannel)
            channels.removeOne(buffer->title());
        model->endRemoveRows();
        if (notify) {
            emit q->removed(buffer);
            if (isChannel)
                emit q->channelsChanged(channels);
            emit q->buffersChanged(bufferList);
            emit q->countChanged(bufferList.count());
        }
    }
}

bool IrcBasePrivate::renameBuffer(const QString& from, const QString& to)
{
    Q_Q(IrcBase);
    const QString fromLower = from.toLower();
    const QString toLower = to.toLower();
    if (bufferMap.contains(toLower))
        destroyBuffer(toLower, true);
    if (bufferMap.contains(fromLower)) {
        IrcBuffer* buffer = bufferMap.take(fromLower);
        bufferMap.insert(toLower, buffer);

        const int idx = bufferList.indexOf(buffer);
        QModelIndex index = model->index(idx);
        emit model->dataChanged(index, index);

        if (model->sortMethod() != Irc::SortByHand) {
            QList<IrcBuffer*> buffers = bufferList;
            const bool notify = false;
            removeBuffer(buffer, notify);
            insertBuffer(-1, buffer, notify);
            if (buffers != bufferList)
                emit q->buffersChanged(bufferList);
        }
        return true;
    }
    return false;
}

bool IrcBasePrivate::deliverMessage(const QString& title, IrcMessage* message, bool create)
{
    IrcBuffer* buffer = bufferMap.value(title.toLower());
    if (!buffer && create)
        buffer = createBuffer(title);
    if (buffer)
        return IrcBufferPrivate::get(buffer)->processMessage(message);
    return false;
}

void IrcBasePrivate::_irc_connected()
{
    foreach (IrcBuffer* buffer, bufferList)
        IrcBufferPrivate::get(buffer)->connected();
}

void IrcBasePrivate::_irc_disconnected()
{
    foreach (IrcBuffer* buffer, bufferList)
        IrcBufferPrivate::get(buffer)->disconnected();
}

void IrcBasePrivate::_irc_bufferDestroyed(IrcBuffer* buffer)
{
    removeBuffer(buffer);
}

/*!
    Constructs a new base object with \a parent.
 */
IrcBase::IrcBase(QObject* parent)
    : IrcBuffer(*new IrcBasePrivate, parent)
{
}

IrcBase::~IrcBase()
{
    Q_D(IrcBase);
    foreach (IrcBuffer* buffer, d->bufferList) {
        buffer->disconnect(this);
        delete buffer;
    }
    d->bufferList.clear();
    d->bufferMap.clear();
    d->channels.clear();
}

int IrcBase::count() const
{
    Q_D(const IrcBase);
    return d->bufferList.count();
}

bool IrcBase::isEmpty() const
{
    Q_D(const IrcBase);
    return d->bufferList.isEmpty();
}

QStringList IrcBase::channels() const
{
    Q_D(const IrcBase);
    return d->channels;
}

QList<IrcBuffer*> IrcBase::buffers() const
{
    Q_D(const IrcBase);
    return d->bufferList;
}

IrcBuffer* IrcBase::get(int index) const
{
    Q_D(const IrcBase);
    return d->bufferList.value(index);
}

IrcBuffer* IrcBase::find(const QString& title) const
{
    Q_D(const IrcBase);
    return d->bufferMap.value(title.toLower());
}

bool IrcBase::contains(const QString& title) const
{
    Q_D(const IrcBase);
    return d->bufferMap.contains(title.toLower());
}

int IrcBase::indexOf(IrcBuffer* buffer) const
{
    Q_D(const IrcBase);
    return d->bufferList.indexOf(buffer);
}

/*!
    Adds a buffer with \a title to the model and returns it.
 */
IrcBuffer* IrcBase::add(const QString& title)
{
    Q_D(IrcBase);
    return d->createBuffer(title);
}

/*!
    Adds the \a buffer to the model.
 */
void IrcBase::add(IrcBuffer* buffer)
{
    Q_D(IrcBase);
    d->addBuffer(buffer);
}

/*!
    Removes and destroys a buffer with \a title from the model.
 */
void IrcBase::remove(const QString& title)
{
    Q_D(IrcBase);
    d->destroyBuffer(title, true);
}

/*!
    Removes and destroys a \a buffer from the model.
 */
void IrcBase::remove(IrcBuffer* buffer)
{
    delete buffer;
}

void IrcBase::clear()
{
    Q_D(IrcBase);
    if (!d->bufferList.isEmpty()) {
        bool bufferRemoved = false;
        bool channelRemoved = false;
        foreach (IrcBuffer* buffer, d->bufferList) {
            if (!buffer->isPersistent()) {
                if (!bufferRemoved) {
                    d->model->beginResetModel();
                    bufferRemoved = true;
                }
                channelRemoved |= buffer->isChannel();
                buffer->disconnect(this);
                d->bufferList.removeOne(buffer);
                d->channels.removeOne(buffer->title());
                d->bufferMap.remove(buffer->title().toLower());
                delete buffer;
            }
        }
        if (bufferRemoved) {
            d->model->endResetModel();
            if (channelRemoved)
                emit channelsChanged(d->channels);
            emit buffersChanged(d->bufferList);
            emit countChanged(d->bufferList.count());
        }
    }
}

void IrcBase::sort(Qt::SortOrder order)
{
    Q_D(IrcBase);
    if (order == Qt::AscendingOrder)
        qSort(d->bufferList.begin(), d->bufferList.end(), IrcBufferLessThan(d->model, d->model->sortMethod()));
    else
        qSort(d->bufferList.begin(), d->bufferList.end(), IrcBufferGreaterThan(d->model, d->model->sortMethod()));
}

#include "moc_ircbase.cpp"

IRC_END_NAMESPACE
