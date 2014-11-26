/*
  Copyright (C) 2008-2014 The Communi Project

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IRCDEBUG_P_H
#define IRCDEBUG_P_H

#include <IrcGlobal>
#include <QtCore/qdebug.h>
#include <QtCore/qstring.h>

IRC_BEGIN_NAMESPACE

#ifndef IRC_DOXYGEN
static const bool irc_dbg_enabled = qgetenv("IRC_DEBUG").toInt();

static void irc_debug(IrcConnection* connection, const char* msg)
{
    if (irc_dbg_enabled) {
        const QString desc = QString::fromLatin1("IrcConnection(%1)").arg(connection->displayName());
        qDebug() << qPrintable(desc) << msg;
    }
}

template<typename T>
static void irc_debug(IrcConnection* connection, const char* msg, const T& arg1)
{
    if (irc_dbg_enabled) {
        QString str; QDebug(&str) << msg << arg1;
        irc_debug(connection, qPrintable(str));
    }
}

template<typename T1, typename T2>
static void irc_debug(IrcConnection* connection, const char* msg, const T1& arg1, const T2& arg2)
{
    if (irc_dbg_enabled) {
        QString str; QDebug(&str) << msg << arg1 << arg2;
        irc_debug(connection, qPrintable(str));
    }
}

template<typename T1, typename T2, typename T3>
static void irc_debug(IrcConnection* connection, const char* msg, const T1& arg1, const T2& arg2, const T3& arg3)
{
    if (irc_dbg_enabled) {
        QString str; QDebug(&str) << msg << arg1 << arg2 << arg3;
        irc_debug(connection, qPrintable(str));
    }
}
#endif // IRC_DOXYGEN

IRC_END_NAMESPACE

#endif // IRCDEBUG_P_H