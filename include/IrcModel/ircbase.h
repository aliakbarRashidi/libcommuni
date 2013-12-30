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

#ifndef IRCBASE_H
#define IRCBASE_H

#include <IrcGlobal>
#include <IrcBuffer>

IRC_BEGIN_NAMESPACE

class IrcBasePrivate;

class IRC_MODEL_EXPORT IrcBase : public IrcBuffer
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)
    Q_PROPERTY(QStringList channels READ channels NOTIFY channelsChanged)
    Q_PROPERTY(QList<IrcBuffer*> buffers READ buffers NOTIFY buffersChanged)

public:
    IrcBase(QObject* parent = 0);
    ~IrcBase();

    int count() const;
    bool isEmpty() const;
    QStringList channels() const;
    QList<IrcBuffer*> buffers() const;
    IrcBuffer* get(int index) const;
    IrcBuffer* find(const QString& title) const;
    bool contains(const QString& title) const;
    int indexOf(IrcBuffer* buffer) const;

    Q_INVOKABLE IrcBuffer* add(const QString& title);
    Q_INVOKABLE void add(IrcBuffer* buffer);
    Q_INVOKABLE void remove(const QString& title);
    Q_INVOKABLE void remove(IrcBuffer* buffer);

public Q_SLOTS:
    void clear();
    void sort(Qt::SortOrder order);

Q_SIGNALS:
    void countChanged(int count);
    void emptyChanged(bool empty);
    void added(IrcBuffer* buffer);
    void removed(IrcBuffer* buffer);
    void aboutToBeAdded(IrcBuffer* buffer);
    void aboutToBeRemoved(IrcBuffer* buffer);
    void buffersChanged(const QList<IrcBuffer*>& buffers);
    void channelsChanged(const QStringList& channels);
    void messageIgnored(IrcMessage* message);

private:
    QScopedPointer<IrcBasePrivate> d_ptr;
    Q_DECLARE_PRIVATE(IrcBase)
    Q_DISABLE_COPY(IrcBase)

    Q_PRIVATE_SLOT(d_func(), void _irc_connected())
    Q_PRIVATE_SLOT(d_func(), void _irc_disconnected())
    Q_PRIVATE_SLOT(d_func(), void _irc_bufferDestroyed(IrcBuffer*))
};

IRC_END_NAMESPACE

#endif // IRCBASE_H
