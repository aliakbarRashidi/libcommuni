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

#include "ircbuffermodel.h"
#include "ircbuffermodel_p.h"
#include "ircchannel_p.h"
#include "ircbase_p.h"
#include "ircbuffer_p.h"
#include "ircnetwork.h"
#include "ircchannel.h"
#include "ircmessage.h"
#include "irccommand.h"
#include "ircconnection.h"
#include <qdatastream.h>
#include <qmetaobject.h>
#include <qmetatype.h>
#include <qvariant.h>

IRC_BEGIN_NAMESPACE

/*!
    \file ircbuffermodel.h
    \brief \#include &lt;IrcBufferModel&gt;
 */

/*!
    \class IrcBufferModel ircbuffermodel.h <IrcBufferModel>
    \ingroup models
    \brief Keeps track of buffers.

    IrcBufferModel automatically keeps track of channel and query buffers
    and manages IrcBuffer instances for them. It will notify via signals
    when channel and query buffers are added and/or removed. IrcBufferModel
    can be used directly as a data model for Qt's item views - both in C++
    and QML.

    \code
    IrcConnection* connection = new IrcConnection(this);
    IrcBufferModel* model = new IrcBufferModel(connection);
    connect(model, SIGNAL(added(IrcBuffer*)), this, SLOT(onBufferAdded(IrcBuffer*)));
    connect(model, SIGNAL(removed(IrcBuffer*)), this, SLOT(onBufferRemoved(IrcBuffer*)));
    listView->setModel(model);
    \endcode
 */

/*!
    \fn void IrcBufferModel::added(IrcBuffer* buffer)

    This signal is emitted when a \a buffer is added to the list of buffers.
 */

/*!
    \fn void IrcBufferModel::removed(IrcBuffer* buffer)

    This signal is emitted when a \a buffer is removed from the list of buffers.
 */

/*!
    \fn void IrcBufferModel::aboutToBeAdded(IrcBuffer* buffer)

    This signal is emitted just before a \a buffer is added to the list of buffers.
 */

/*!
    \fn void IrcBufferModel::aboutToBeRemoved(IrcBuffer* buffer)

    This signal is emitted just before a \a buffer is removed from the list of buffers.
 */

/*!
    \fn void IrcBufferModel::messageIgnored(IrcMessage* message)

    This signal is emitted when a message was ignored.

    IrcBufferModel handles only buffer specific messages and delivers
    them to the appropriate IrcBuffer instances. When applications decide
    to handle IrcBuffer::messageReceived(), this signal makes it easy to
    implement handling for the rest, non-buffer specific messages.

    \sa IrcConnection::messageReceived(), IrcBuffer::messageReceived()
 */

#ifndef IRC_DOXYGEN
IrcBufferModelPrivate::IrcBufferModelPrivate() : q_ptr(0), role(Irc::TitleRole),
    sortMethod(Irc::SortByHand), sortOrder(Qt::AscendingOrder) //, bufferProto(0), channelProto(0), defaultParent(0)
{
}

IrcBase* IrcBufferModelPrivate::createBaseHelper(IrcConnection* connection)
{
    Q_Q(IrcBufferModel);
    IrcBase* parent = 0;
    const QMetaObject* metaObject = q->metaObject();
    int idx = metaObject->indexOfMethod("createBase(QVariant)");
    if (idx != -1) {
        // QML: QVariant createBase(QVariant)
        QVariant ret;
        QMetaMethod method = metaObject->method(idx);
        method.invoke(q, Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, QVariant::fromValue(connection)));
        parent = ret.value<IrcBase*>();
    } else {
        // C++: IrcBase* createBase(IrcConnection*)
        idx = metaObject->indexOfMethod("createBase(IrcConnection*)");
        QMetaMethod method = metaObject->method(idx);
        method.invoke(q, Q_RETURN_ARG(IrcBase*, parent), Q_ARG(IrcConnection*, connection));
    }
    return parent;
}

IrcBuffer* IrcBufferModelPrivate::createBufferHelper(IrcConnection* connection, const QString& title)
{
    Q_Q(IrcBufferModel);
    IrcBuffer* buffer = 0;
    const QMetaObject* metaObject = q->metaObject();
    int idx = metaObject->indexOfMethod("createBuffer(QVariant,QVariant)");
    if (idx != -1) {
        // QML: QVariant createBuffer(QVariant,QVariant)
        QVariant ret;
        QMetaMethod method = metaObject->method(idx);
        method.invoke(q, Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, QVariant::fromValue(connection)), Q_ARG(QVariant, title));
        buffer = ret.value<IrcBuffer*>();
    } else {
        // C++: IrcBuffer* createBuffer(IrcConnection*,QString)
        idx = metaObject->indexOfMethod("createBuffer(IrcConnection*,QString)");
        QMetaMethod method = metaObject->method(idx);
        method.invoke(q, Q_RETURN_ARG(IrcBuffer*, buffer), Q_ARG(IrcConnection*, connection), Q_ARG(QString, title));
    }
    return buffer;
}

IrcChannel* IrcBufferModelPrivate::createChannelHelper(IrcConnection* connection, const QString& title)
{
    Q_Q(IrcBufferModel);
    IrcChannel* channel = 0;
    const QMetaObject* metaObject = q->metaObject();
    int idx = metaObject->indexOfMethod("createChannel(QVariant,QVariant)");
    if (idx != -1) {
        // QML: QVariant createChannel(QVariant,QVariant)
        QVariant ret;
        QMetaMethod method = metaObject->method(idx);
        method.invoke(q, Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, QVariant::fromValue(connection)), Q_ARG(QVariant, title));
        channel = ret.value<IrcChannel*>();
    } else {
        // C++: IrcChannel* createChannel(IrcConnectio*,QString)
        idx = metaObject->indexOfMethod("createChannel(IrcConnection*,QString)");
        QMetaMethod method = metaObject->method(idx);
        method.invoke(q, Q_RETURN_ARG(IrcChannel*, channel), Q_ARG(IrcConnection*, connection), Q_ARG(QString, title));
    }
    return channel;
}

IrcBase* IrcBufferModelPrivate::insertBase(int index, IrcConnection* connection)
{
    Q_Q(IrcBufferModel);
    IrcBase* base = connections.value(connection);
    if (!base) {
        base = createBaseHelper(connection);
        connections.insert(connection, base);
        if (index < 0 || index > bases.count())
            index = bases.count();
        bases.insert(index, base);
        q->connect(base, SIGNAL(destroyed(IrcBase*)), q, SLOT(_irc_baseDestroyed(IrcBase*)));
    }
    return base;
}

bool IrcBufferModelPrivate::removeBase(IrcConnection* connection)
{
    Q_Q(IrcBufferModel);
    IrcBase* base = connections.take(connection);
    if (base) {
        bases.removeOne(base);
        q->disconnect(base, SIGNAL(destroyed(IrcBase*)), q, SLOT(_irc_baseDestroyed(IrcBase*)));
        delete base;
        return true;
    }
    return false;
}

void IrcBufferModelPrivate::_irc_baseDestroyed(IrcBase* base)
{
    removeBase(base->connection());
}
#endif // IRC_DOXYGEN

/*!
    Constructs a new model with \a parent.

    \note If \a parent is an instance of IrcConnection, it will be
    automatically assigned to \ref IrcBufferModel::connection "connection".
 */
IrcBufferModel::IrcBufferModel(QObject* parent)
    : QAbstractListModel(parent), d_ptr(new IrcBufferModelPrivate)
{
    Q_D(IrcBufferModel);
    d->q_ptr = this;
//    d->defaultParent = d->createParent();
//    setBufferPrototype(new IrcBuffer(this));
//    setChannelPrototype(new IrcChannel(this));
//    setConnection(qobject_cast<IrcConnection*>(parent));
}

/*!
    Destructs the model.
 */
IrcBufferModel::~IrcBufferModel()
{
    emit destroyed(this);
}

/*!
    This property holds the number of bases.

    \par Access function:
    \li int <b>count</b>() const

    \par Notifier signal:
    \li void <b>countChanged</b>(int count)
 */
int IrcBufferModel::count() const
{
    Q_D(const IrcBufferModel);
    return d->bases.count();
}

/*!
    \since 3.1
    \property bool IrcBufferModel::empty

    This property holds the whether the model is empty.

    \par Access function:
    \li bool <b>isEmpty</b>() const

    \par Notifier signal:
    \li void <b>emptyChanged</b>(bool empty)
 */
bool IrcBufferModel::isEmpty() const
{
    Q_D(const IrcBufferModel);
    return d->bases.isEmpty();
}

/*!
    This property holds the list of bases.

    \par Access function:
    \li QList<\ref IrcBase*> <b>bases</b>() const

    \par Notifier signal:
    \li void <b>basesChanged</b>(const QList<\ref IrcBase*>& bases)
 */
QList<IrcBase*> IrcBufferModel::bases() const
{
    Q_D(const IrcBufferModel);
    return d->bases;
}

/*!
    Returns the base object at \a index.
 */
IrcBase* IrcBufferModel::get(int index) const
{
    Q_D(const IrcBufferModel);
    return d->bases.value(index);
}

/*!
    Returns the index of the specified \a parent,
    or \c -1 if the model does not contain the \a parent.
 */
int IrcBufferModel::indexOf(IrcBase* parent) const
{
    Q_D(const IrcBufferModel);
    return d->bases.indexOf(parent);
}

/*!
    Returns the base object for \a connection or \c 0 if not found.
 */
IrcBase* IrcBufferModel::find(IrcConnection* connection) const
{
    Q_D(const IrcBufferModel);
    return d->connections.value(connection);
}

/*!
    Adds a \a connection to the model and returns the base object.
 */
IrcBase* IrcBufferModel::add(IrcConnection* connection)
{
    Q_D(IrcBufferModel);
    return d->insertBase(-1, connection);
}

/*!
    Inserts a \a connection to the model at \a index and returns the base object.
 */
IrcBase* IrcBufferModel::insert(int index, IrcConnection* connection)
{
    Q_D(IrcBufferModel);
    return d->insertBase(index, connection);
}

/*!
    Removes and destroys a parent for \a connection from the model.
 */
void IrcBufferModel::remove(IrcConnection* connection)
{
    Q_D(IrcBufferModel);
    d->removeBase(connection);
}

/*!
    Creates a base object for \a connection.

    IrcBufferModel will automatically call this factory method when a
    need for the base object occurs ie. a connection is added.

    The default implementation creates an instance of IrcBase.
    Reimplement this function in order to alter the default behavior.
 */
IrcBase* IrcBufferModel::createBase(IrcConnection* connection)
{
    Q_UNUSED(connection);
    return new IrcBase(this);
}

/*!
    Creates a buffer object with \a title for \a connection.

    IrcBufferModel will automatically call this factory method when a
    need for the buffer object occurs ie. a private message is received.

    The default implementation creates an instance of IrcBuffer.
    Reimplement this function in order to alter the default behavior.
 */
IrcBuffer* IrcBufferModel::createBuffer(IrcConnection* connection, const QString& title)
{
    Q_UNUSED(connection);
    Q_UNUSED(title);
    return new IrcBuffer(this);
}

/*!
    Creates a channel object with \a title for \a connection.

    IrcBufferModel will automatically call this factory method when a
    need for the channel object occurs ie. a channel is being joined.

    The default implementation creates an instance of IrcChannel.
    Reimplement this function in order to alter the default behavior.
 */
IrcChannel* IrcBufferModel::createChannel(IrcConnection* connection, const QString& title)
{
    Q_UNUSED(connection);
    Q_UNUSED(title);
    return new IrcChannel(this);
}

/*!
    This property holds the display role.

    The specified data role is returned for Qt::DisplayRole.

    The default value is \ref Irc::TitleRole.

    \par Access functions:
    \li \ref Irc::DataRole <b>displayRole</b>() const
    \li void <b>setDisplayRole</b>(\ref Irc::DataRole role)
 */
Irc::DataRole IrcBufferModel::displayRole() const
{
    Q_D(const IrcBufferModel);
    return d->role;
}

void IrcBufferModel::setDisplayRole(Irc::DataRole role)
{
    Q_D(IrcBufferModel);
    d->role = role;
}

/*!
    This property holds the model sort order.

    The default value is \c Qt::AscendingOrder.

    \par Access functions:
    \li Qt::SortOrder <b>sortOrder</b>() const
    \li void <b>setSortOrder</b>(Qt::SortOrder order)

    \sa sort(), lessThan()
 */
Qt::SortOrder IrcBufferModel::sortOrder() const
{
    Q_D(const IrcBufferModel);
    return d->sortOrder;
}

void IrcBufferModel::setSortOrder(Qt::SortOrder order)
{
    Q_D(IrcBufferModel);
    if (d->sortOrder != order) {
        d->sortOrder = order;
        if (d->sortMethod != Irc::SortByHand && !d->bases.isEmpty())
            sort(d->sortMethod, d->sortOrder);
    }
}

/*!
    This property holds the model sort method.

    The default value is \c Irc::SortByHand.

    Method           | Description                                                       | Example
    -----------------|-------------------------------------------------------------------|-------------------------------------------------
    Irc::SortByHand  | Buffers are not sorted automatically, but only by calling sort(). | -
    Irc::SortByName  | Buffers are sorted alphabetically, ignoring any channel prefix.   | "bot", "#communi", "#freenode", "jpnurmi", "#qt"
    Irc::SortByTitle | Buffers are sorted alphabetically, and channels before queries.   | "#communi", "#freenode", "#qt", "bot", "jpnurmi"

    \par Access functions:
    \li Irc::SortMethod <b>sortMethod</b>() const
    \li void <b>setSortMethod</b>(Irc::SortMethod method)

    \sa sort(), lessThan()
 */
Irc::SortMethod IrcBufferModel::sortMethod() const
{
    Q_D(const IrcBufferModel);
    return d->sortMethod;
}

void IrcBufferModel::setSortMethod(Irc::SortMethod method)
{
    Q_D(IrcBufferModel);
    if (d->sortMethod != method) {
        d->sortMethod = method;
        if (d->sortMethod != Irc::SortByHand && !d->bases.isEmpty())
            sort(d->sortMethod, d->sortOrder);
    }
}

/*!
    Clears the model.

    All buffers except \ref IrcBuffer::persistent "persistent" buffers are removed and destroyed.

    In order to remove a persistent buffer, either explicitly call remove() or delete the buffer.
 */
void IrcBufferModel::clear()
{
    Q_D(IrcBufferModel);
    // TODO: d->defaultParent->clear();
}

/*!
    Sorts the model using the given \a order.
 */
void IrcBufferModel::sort(int column, Qt::SortOrder order)
{
    Q_D(IrcBufferModel);
    if (column == 0)
        sort(d->sortMethod, order);
}

/*!
    Sorts the model using the given \a method and \a order.

    \sa lessThan()
 */
void IrcBufferModel::sort(Irc::SortMethod method, Qt::SortOrder order)
{
    Q_D(IrcBufferModel);
    if (method == Irc::SortByHand)
        return;

    emit layoutAboutToBeChanged();

    QList<IrcBuffer*> persistentBuffers;
    QModelIndexList oldPersistentIndexes = persistentIndexList();
    foreach (const QModelIndex& index, oldPersistentIndexes)
        persistentBuffers += static_cast<IrcBuffer*>(index.internalPointer());

    foreach (IrcBase* parent, d->bases)
        parent->sort(order);

//    TODO:
//    QModelIndexList newPersistentIndexes;
//    foreach (IrcBuffer* buffer, persistentBuffers)
//        newPersistentIndexes += index(d->defaultParent->indexOf(buffer));
//    changePersistentIndexList(oldPersistentIndexes, newPersistentIndexes);

    emit layoutChanged();
}

/*!
    Returns the model index for \a buffer.
 */
QModelIndex IrcBufferModel::index(IrcBuffer* buffer) const
{
    Q_D(const IrcBufferModel);
    return QModelIndex(); // TODO: index(d->bases->indexOf(buffer));
}

/*!
    Returns the buffer for model \a index.
 */
IrcBuffer* IrcBufferModel::buffer(const QModelIndex& index) const
{
    if (!hasIndex(index.row(), index.column()))
        return 0;

    return static_cast<IrcBuffer*>(index.internalPointer());
}

/*!
    The following role names are provided by default:

    Role             | Name       | Type        | Example
    -----------------|------------|-------------|--------
    Qt::DisplayRole  | "display"  | 1)          | -
    Irc::BufferRole  | "buffer"   | IrcBuffer*  | &lt;object&gt;
    Irc::ChannelRole | "channel"  | IrcChannel* | &lt;object&gt;
    Irc::NameRole    | "name"     | QString     | "communi"
    Irc::PrefixRole  | "prefix"   | QString     | "#"
    Irc::TitleRole   | "title"    | QString     | "#communi"

    1) The type depends on \ref displayRole.
 */
QHash<int, QByteArray> IrcBufferModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Irc::BufferRole] = "buffer";
    roles[Irc::ChannelRole] = "channel";
    roles[Irc::NameRole] = "name";
    roles[Irc::PrefixRole] = "prefix";
    roles[Irc::TitleRole] = "title";
    return roles;
}

/*!
    Returns the number of buffers.
 */
int IrcBufferModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    Q_D(const IrcBufferModel);
    return d->bases.count();
}

/*!
    Returns the data for specified \a role and user referred to by by the \a index.
 */
QVariant IrcBufferModel::data(const QModelIndex& index, int role) const
{
    if (!hasIndex(index.row(), index.column(), index.parent()))
        return QVariant();

    IrcBase* base = static_cast<IrcBase*>(index.internalPointer());
    Q_ASSERT(base);
    return base->data(role);
}

/*!
    Returns the index of the item in the model specified by the given \a row, \a column and \a parent index.
 */
QModelIndex IrcBufferModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_D(const IrcBufferModel);
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, d->bases.value(row));
}

/*!
    Returns \c true if \a one buffer is "less than" \a another,
    otherwise returns \c false.

    The default implementation sorts according to the specified sort method.
    Reimplement this function in order to customize the sort order.

    \sa sort(), sortMethod
 */
bool IrcBufferModel::lessThan(IrcBuffer* one, IrcBuffer* another, Irc::SortMethod method) const
{
    if (one->isSticky() != another->isSticky())
        return one->isSticky();

    if (method == Irc::SortByTitle) {
        const QStringList prefixes = one->network()->channelTypes();

        const QString p1 = one->prefix();
        const QString p2 = another->prefix();

        const int i1 = !p1.isEmpty() ? prefixes.indexOf(p1.at(0)) : -1;
        const int i2 = !p2.isEmpty() ? prefixes.indexOf(p2.at(0)) : -1;

        if (i1 >= 0 && i2 < 0)
            return true;
        if (i1 < 0 && i2 >= 0)
            return false;
        if (i1 >= 0 && i2 >= 0 && i1 != i2)
            return i1 < i2;
    }

    // Irc::SortByName
    const QString n1 = one->name();
    const QString n2 = another->name();
    return n1.compare(n2, Qt::CaseInsensitive) < 0;
}

#if 0

/*!
    \since 3.1

    Saves the state of the model. The \a version number is stored as part of the state data.

    To restore the saved state, pass the return value and \a version number to restoreState().
 */
QByteArray IrcBufferModel::saveState(int version) const
{
    Q_D(const IrcBufferModel);
    QVariantMap args;
    args.insert("version", version);
    args.insert("sortOrder", d->sortOrder);
    args.insert("sortMethod", d->sortMethod);
    args.insert("displayRole", d->role);

    QVariantList bufs;
    foreach (IrcBuffer* buffer, d->defaultParent->buffers()) {
        QVariantMap b;
        b.insert("channel", buffer->isChannel());
        b.insert("name", buffer->name());
        b.insert("prefix", buffer->prefix());
        b.insert("title", buffer->title());
        if (IrcChannel* channel = buffer->toChannel()) {
            IrcChannelPrivate* p = IrcChannelPrivate::get(channel);
            b.insert("modes", QStringList(p->modes.keys()));
            b.insert("args", QStringList(p->modes.values()));
            b.insert("topic", channel->topic());
        }
        b.insert("stick", buffer->isSticky());
        b.insert("persistent", buffer->isPersistent());
        bufs += b;
    }
    args.insert("buffers", bufs);

    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << args;
    return state;
}

/*!
    \since 3.1

    Restores the \a state of the model. The \a version number is compared with that stored in \a state.
    If they do not match, the model state is left unchanged, and this function returns \c false; otherwise,
    the state is restored, and \c true is returned.

    \sa saveState()
 */
bool IrcBufferModel::restoreState(const QByteArray& state, int version)
{
    Q_D(IrcBufferModel);
    QVariantMap args;
    QDataStream in(state);
    in >> args;
    if (in.status() != QDataStream::Ok || args.value("version", -1).toInt() != version)
        return false;

    setSortOrder(static_cast<Qt::SortOrder>(args.value("sortOrder", sortOrder()).toInt()));
    setSortMethod(static_cast<Irc::SortMethod>(args.value("sortMethod", sortMethod()).toInt()));
    setDisplayRole(static_cast<Irc::DataRole>(args.value("displayRole", displayRole()).toInt()));

    QVariantList buffers = args.value("buffers").toList();
    foreach (const QVariant& v, buffers) {
        QVariantMap b = v.toMap();
        IrcBuffer* buffer = find(b.value("title").toString());
        if (!buffer) {
            if (b.value("channel").toBool())
                buffer = d->defaultParent->createChannelHelper(b.value("title").toString());
            else
                buffer = d->defaultParent->createBufferHelper(b.value("title").toString());
            buffer->setName(b.value("name").toString());
            buffer->setPrefix(b.value("prefix").toString());
            buffer->setSticky(b.value("sticky").toBool());
            buffer->setPersistent(b.value("persistent").toBool());
            add(buffer);
        }
        IrcChannel* channel = buffer->toChannel();
        if (channel && !channel->isActive()) {
            IrcChannelPrivate* p = IrcChannelPrivate::get(channel);
            const QStringList modes = b.value("modes").toStringList();
            const QStringList args = b.value("args").toStringList();
            for (int i = 0; i < modes.count(); ++i)
                p->modes.insert(modes.at(i), args.value(i));
            channel->join();
        }
    }
    return true;
}

#endif // 0

#include "moc_ircbuffermodel.cpp"

IRC_END_NAMESPACE
