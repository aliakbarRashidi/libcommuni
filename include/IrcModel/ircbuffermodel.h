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

#ifndef IRCBUFFERMODEL_H
#define IRCBUFFERMODEL_H

#include <Irc>
#include <IrcGlobal>
#include <QtCore/qstringlist.h>
#include <QtCore/qabstractitemmodel.h>

IRC_BEGIN_NAMESPACE

class IrcBase;
class IrcBuffer;
class IrcChannel;
class IrcMessage;
class IrcNetwork;
class IrcConnection;
class IrcBufferModelPrivate;

class IRC_MODEL_EXPORT IrcBufferModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder)
    Q_PROPERTY(Irc::SortMethod sortMethod READ sortMethod WRITE setSortMethod)
    Q_PROPERTY(Irc::DataRole displayRole READ displayRole WRITE setDisplayRole)
    Q_PROPERTY(QList<IrcBase*> bases READ bases NOTIFY basesChanged)

public:
    explicit IrcBufferModel(QObject* parent = 0);
    virtual ~IrcBufferModel();

    int count() const;
    bool isEmpty() const;
    QList<IrcBase*> bases() const;

    Q_INVOKABLE IrcBase* get(int index) const;
    Q_INVOKABLE int indexOf(IrcBase* parent) const;
    Q_INVOKABLE IrcBase* find(IrcConnection* connection) const;

    Q_INVOKABLE IrcBase* add(IrcConnection* connection);
    Q_INVOKABLE IrcBase* insert(int index, IrcConnection* connection);
    Q_INVOKABLE void remove(IrcConnection* connection);

    Qt::SortOrder sortOrder() const;
    void setSortOrder(Qt::SortOrder order);

    Irc::SortMethod sortMethod() const;
    void setSortMethod(Irc::SortMethod method);

    Irc::DataRole displayRole() const;
    void setDisplayRole(Irc::DataRole role);

    QModelIndex index(IrcBuffer* buffer) const;
    IrcBuffer* buffer(const QModelIndex& index) const;

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;

//    Q_INVOKABLE QByteArray saveState(int version = 0) const;
//    Q_INVOKABLE bool restoreState(const QByteArray& state, int version = 0);

public Q_SLOTS:
    void clear();
    void sort(int column = 0, Qt::SortOrder order = Qt::AscendingOrder);
    void sort(Irc::SortMethod method, Qt::SortOrder order = Qt::AscendingOrder);

Q_SIGNALS:
    void countChanged(int count);
    void emptyChanged(bool empty);
    void added(IrcBase* base);
    void removed(IrcBase* base);
    void basesChanged(const QList<IrcBase*>& bases);
    void destroyed(IrcBufferModel* model);

protected Q_SLOTS:
    virtual IrcBase* createBase(IrcConnection* connection);
    virtual IrcBuffer* createBuffer(IrcConnection* connection, const QString& title);
    virtual IrcChannel* createChannel(IrcConnection* connection, const QString& title);

protected:
    virtual bool lessThan(IrcBuffer* one, IrcBuffer* another, Irc::SortMethod method) const;

private:
    friend class IrcBase;
    friend class IrcBasePrivate;
    friend class IrcBufferLessThan;
    friend class IrcBufferGreaterThan;
    QScopedPointer<IrcBufferModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(IrcBufferModel)
    Q_DISABLE_COPY(IrcBufferModel)

    Q_PRIVATE_SLOT(d_func(), void _irc_baseDestroyed(IrcBase*))
};

IRC_END_NAMESPACE

#endif // IRCBUFFERMODEL_H
