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

#ifndef IRCBUFFERMODEL_P_H
#define IRCBUFFERMODEL_P_H

#include "ircbuffer.h"
#include "ircbuffermodel.h"

IRC_BEGIN_NAMESPACE

class IrcBase;

class IrcBufferModelPrivate
{
    Q_DECLARE_PUBLIC(IrcBufferModel)

public:
    IrcBufferModelPrivate();

    IrcBase* createBaseHelper(IrcConnection* connection);
    IrcBuffer* createBufferHelper(IrcConnection* connection, const QString& title);
    IrcChannel* createChannelHelper(IrcConnection* connection, const QString& title);

    IrcBase* insertBase(int index, IrcConnection* connection);
    bool removeBase(IrcConnection* connection);

    void _irc_baseDestroyed(IrcBase* base);

    static IrcBufferModelPrivate* get(IrcBufferModel* model)
    {
        return model->d_func();
    }

    IrcBufferModel* q_ptr;
    Irc::DataRole role;
    Irc::SortMethod sortMethod;
    Qt::SortOrder sortOrder;
    QList<IrcBase*> bases;
    QHash<IrcConnection*, IrcBase*> connections;
};

IRC_END_NAMESPACE

#endif // IRCBUFFERMODEL_P_H
