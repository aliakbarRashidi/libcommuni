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

#ifndef IRCBUFFER_P_H
#define IRCBUFFER_P_H

#include "ircbuffer.h"
#include "ircmessage.h"
#include <qstringlist.h>
#include <qlist.h>
#include <qmap.h>

IRC_BEGIN_NAMESPACE

class IrcUser;
class IrcBase;
class IrcUserModel;

class IrcBufferPrivate
{
    Q_DECLARE_PUBLIC(IrcBuffer)

public:
    IrcBufferPrivate();
    virtual ~IrcBufferPrivate();

    virtual void init(const QString& title, IrcConnection* connection, IrcBufferModel* model);
    virtual void connected();
    virtual void disconnected();

    void setName(const QString& name);
    void setPrefix(const QString& prefix);

    void setModel(IrcBufferModel* model);
    void setConnection(IrcConnection* connection);

    bool processMessage(IrcMessage* message);

    virtual bool processJoinMessage(IrcJoinMessage* message);
    virtual bool processKickMessage(IrcKickMessage* message);
    virtual bool processModeMessage(IrcModeMessage* message);
    virtual bool processNamesMessage(IrcNamesMessage* message);
    virtual bool processNickMessage(IrcNickMessage* message);
    virtual bool processNoticeMessage(IrcNoticeMessage* message);
    virtual bool processNumericMessage(IrcNumericMessage* message);
    virtual bool processPartMessage(IrcPartMessage* message);
    virtual bool processPrivateMessage(IrcPrivateMessage* message);
    virtual bool processQuitMessage(IrcQuitMessage* message);
    virtual bool processTopicMessage(IrcTopicMessage* message);
    virtual bool processWhoReplyMessage(IrcWhoReplyMessage* message);

    static IrcBufferPrivate* get(IrcBuffer* buffer)
    {
        return buffer->d_func();
    }

    IrcBuffer* q_ptr;
    IrcConnection* connection;
    IrcBufferModel* model;
    IrcBase* base;
    QString name;
    QString prefix;
    bool persistent;
    bool sticky;
};

IRC_END_NAMESPACE

#endif // IRCBUFFER_P_H
