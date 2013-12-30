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

#ifndef IRCBASE_P_H
#define IRCBASE_P_H

#include "ircbase.h"
#include "ircfilter.h"
#include "ircbuffer_p.h"
#include <qpointer.h>

IRC_BEGIN_NAMESPACE

class IrcBasePrivate : public QObject, public IrcBufferPrivate, public IrcMessageFilter, public IrcCommandFilter
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(IrcBase)
    Q_INTERFACES(IrcMessageFilter IrcCommandFilter)

public:
    IrcBasePrivate();

    virtual void init(const QString& title, IrcConnection* connection, IrcBufferModel* model);

    bool messageFilter(IrcMessage* message);
    bool commandFilter(IrcCommand* command);

    IrcBuffer* createBuffer(const QString& title);
    void destroyBuffer(const QString& title, bool force = false);

    void addBuffer(IrcBuffer* buffer, bool notify = true);
    void insertBuffer(int index, IrcBuffer* buffer, bool notify = true);
    void removeBuffer(IrcBuffer* buffer, bool notify = true);
    bool renameBuffer(const QString& from, const QString& to);

    bool deliverMessage(const QString& title, IrcMessage* message, bool create = false);

    void _irc_connected();
    void _irc_disconnected();
    void _irc_bufferDestroyed(IrcBuffer* buffer);

    static IrcBasePrivate* get(IrcBase* base)
    {
        return base->d_func();
    }

    QList<IrcBuffer*> bufferList;
    QMap<QString, IrcBuffer*> bufferMap;
    QHash<QString, QString> keys;
    QStringList channels;
};

IRC_END_NAMESPACE

#endif // IRCBASE_P_H
