/*
* Copyright (C) 2008-2013 The Communi Project
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

#include "ircprotocol.h"
#include "ircconnection_p.h"
#include "ircmessagebuilder_p.h"
#include "ircnetwork_p.h"
#include "ircconnection.h"
#include "ircmessage.h"
#include "irccommand.h"
#include "irc.h"
#include <QDebug>

IRC_BEGIN_NAMESPACE

class IrcProtocolPrivate
{
    Q_DECLARE_PUBLIC(IrcProtocol)

public:
    IrcProtocolPrivate();

    void readLines(const QByteArray& delimiter);
    void processLine(const QByteArray& line);

    void handleNumericMessage(IrcNumericMessage* msg);
    void handlePrivateMessage(IrcPrivateMessage* msg);
    void handleCapabilityMessage(IrcCapabilityMessage* msg);

    IrcProtocol* q_ptr;
    IrcConnection* connection;
    IrcMessageBuilder* builder;
    QByteArray buffer;
};

IrcProtocolPrivate::IrcProtocolPrivate() : q_ptr(0)
{
}

void IrcProtocolPrivate::readLines(const QByteArray& delimiter)
{
    int i = -1;
    while ((i = buffer.indexOf(delimiter)) != -1) {
        QByteArray line = buffer.left(i).trimmed();
        buffer = buffer.mid(i + delimiter.length());
        if (!line.isEmpty())
            processLine(line);
    }
}

void IrcProtocolPrivate::processLine(const QByteArray& line)
{
    Q_Q(IrcProtocol);
    static bool dbg = qgetenv("IRC_DEBUG").toInt();
    if (dbg) qDebug() << line;

    if (line.startsWith("AUTHENTICATE") && !connection->saslMechanism().isEmpty()) {
        const QList<QByteArray> args = line.split(' ');
        if (args.count() == 2)
            q->authenticate(true, args.at(1));
        if (!connection->isConnected())
            connection->sendData("CAP END");
        return;
    }

    IrcMessage* msg = IrcMessage::fromData(line, connection);
    if (msg) {
        msg->setEncoding(connection->encoding());

        switch (msg->type()) {
        case IrcMessage::Capability:
            handleCapabilityMessage(static_cast<IrcCapabilityMessage*>(msg));
            break;
        case IrcMessage::Nick:
            if (msg->flags() & IrcMessage::Own)
                q->setNick(static_cast<IrcNickMessage*>(msg)->newNick());
            break;
        case IrcMessage::Numeric:
            handleNumericMessage(static_cast<IrcNumericMessage*>(msg));
            break;
        case IrcMessage::Ping:
            connection->sendRaw("PONG " + static_cast<IrcPingMessage*>(msg)->argument());
            break;
        case IrcMessage::Private:
            handlePrivateMessage(static_cast<IrcPrivateMessage*>(msg));
            break;
        default:
            break;
        }

        q->receiveMessage(msg);
        if (msg->type() == IrcMessage::Numeric)
            builder->processMessage(static_cast<IrcNumericMessage*>(msg));
    }
}

void IrcProtocolPrivate::handleNumericMessage(IrcNumericMessage* msg)
{
    Q_Q(IrcProtocol);
    switch (msg->code()) {
    case Irc::RPL_WELCOME:
        q->setNick(msg->parameters().value(0));
        q->setConnected(true);
        break;
    case Irc::RPL_ISUPPORT: {
        QHash<QString, QString> info;
        foreach (const QString& param, msg->parameters().mid(1)) {
            QStringList keyValue = param.split("=", QString::SkipEmptyParts);
            info.insert(keyValue.value(0), keyValue.value(1));
        }
        q->setInfo(info);
        break;
    }
    case Irc::ERR_NICKNAMEINUSE:
    case Irc::ERR_NICKCOLLISION: {
        QString alternate;
        emit connection->nickNameReserved(&alternate);
        if (!alternate.isEmpty())
            connection->setNickName(alternate);
        break;
    }
    default:
        break;
    }
}

void IrcProtocolPrivate::handlePrivateMessage(IrcPrivateMessage* msg)
{
    if (msg->isRequest()) {
        IrcCommand* reply = IrcConnectionPrivate::get(connection)->createCtcpReply(msg);
        if (reply)
            connection->sendCommand(reply);
    }
}

static void handleCapability(QSet<QString>* caps, const QString& cap)
{
    Q_ASSERT(caps);
    if (cap.startsWith(QLatin1Char('-')) || cap.startsWith(QLatin1Char('=')))
        caps->remove(cap.mid(1));
    else if (cap.startsWith(QLatin1Char('~')))
        caps->insert(cap.mid(1));
    else
        caps->insert(cap);
}

void IrcProtocolPrivate::handleCapabilityMessage(IrcCapabilityMessage* msg)
{
    Q_Q(IrcProtocol);
    const bool connected = connection->isConnected();
    const QString subCommand = msg->subCommand();
    if (subCommand == "LS") {
        QSet<QString> availableCaps = connection->network()->availableCapabilities().toSet();
        foreach (const QString& cap, msg->capabilities())
            handleCapability(&availableCaps, cap);
        q->setAvailableCapabilities(availableCaps);

        if (!connected) {
            const QStringList params = msg->parameters();
            if (params.value(params.count() - 1) != QLatin1String("*")) {
                if (!connection->saslMechanism().isEmpty() && availableCaps.contains(QLatin1String("sasl")))
                    connection->sendCommand(IrcCommand::createCapability("REQ", QLatin1String("sasl")));
                else
                    connection->sendData("CAP END");
            }
        }
    } else if (subCommand == "ACK" || subCommand == "NAK") {
        bool auth = false;
        if (subCommand == "ACK") {
            QSet<QString> activeCaps = connection->network()->activeCapabilities().toSet();
            foreach (const QString& cap, msg->capabilities()) {
                handleCapability(&activeCaps, cap);
                if (cap == "sasl" && !connection->saslMechanism().isEmpty() && !connection->password().isEmpty())
                    auth = connection->sendRaw("AUTHENTICATE " + connection->saslMechanism());
            }
            q->setActiveCapabilities(activeCaps);
        }

        if (!connected && !auth)
            connection->sendData("CAP END");
    }
}

IrcProtocol::IrcProtocol(IrcConnection* connection) : QObject(connection), d_ptr(new IrcProtocolPrivate)
{
    Q_D(IrcProtocol);
    d->q_ptr = this;
    d->connection = connection;
    d->builder = new IrcMessageBuilder(connection);
    connect(d->builder, SIGNAL(messageReceived(IrcMessage*)), this, SLOT(receiveMessage(IrcMessage*)));
}

IrcProtocol::~IrcProtocol()
{
    Q_D(IrcProtocol);
    delete d->builder;
}

IrcConnection* IrcProtocol::connection() const
{
    Q_D(const IrcProtocol);
    return d->connection;
}

QAbstractSocket* IrcProtocol::socket() const
{
    Q_D(const IrcProtocol);
    return d->connection->socket();
}

void IrcProtocol::open()
{
    Q_D(IrcProtocol);

    const bool secure = d->connection->isSecure();
    if (secure)
        QMetaObject::invokeMethod(socket(), "startClientEncryption");

    // Send CAP LS first; if the server understands it this will
    // temporarily pause the handshake until CAP END is sent, so we
    // know whether the server supports the CAP extension.
    d->connection->sendData("CAP LS");

    if (d->connection->saslMechanism().isEmpty() && !d->connection->password().isEmpty())
        authenticate(false);

    d->connection->sendCommand(IrcCommand::createNick(d->connection->nickName()));
    d->connection->sendRaw(QString("USER %1 hostname servername :%2").arg(d->connection->userName(), d->connection->realName()));
}

#include <qblowfish.h>
void IrcProtocol::authenticate(bool secure, const QString& arg)
{
    Q_D(IrcProtocol);
    const QString password = d->connection->password();
    if (!password.isEmpty()) {
        if (secure) {
            const QString sasl = d->connection->saslMechanism();
            if (sasl == "PLAIN" && arg == "+") {
                const QByteArray userName = d->connection->userName().toUtf8();
                const QByteArray data = userName + '\0' + userName + '\0' + password.toUtf8();
                d->connection->sendData("AUTHENTICATE " + data.toBase64());
            } else if (sasl == "DH-BLOWFISH") {
                qDebug() << arg;
                QBlowfish fish(arg.toUtf8());
            }
        } else {
            d->connection->sendRaw(QString("PASS %1").arg(password));
        }
    }
}

void IrcProtocol::read()
{
    Q_D(IrcProtocol);
    d->buffer += socket()->readAll();
    // try reading RFC compliant message lines first
    d->readLines("\r\n");
    // fall back to RFC incompliant lines...
    d->readLines("\n");
}

bool IrcProtocol::write(const QByteArray& data)
{
    return socket()->write(data + QByteArray("\r\n")) != -1;
}

void IrcProtocol::setActive(bool active)
{
    Q_D(IrcProtocol);
    IrcConnectionPrivate* priv = IrcConnectionPrivate::get(d->connection);
    priv->setActive(active);
}

void IrcProtocol::setConnected(bool connected)
{
    Q_D(IrcProtocol);
    IrcConnectionPrivate* priv = IrcConnectionPrivate::get(d->connection);
    priv->setConnected(connected);
}

void IrcProtocol::setNick(const QString& nick)
{
    Q_D(IrcProtocol);
    IrcConnectionPrivate* priv = IrcConnectionPrivate::get(d->connection);
    priv->setNick(nick);
}

void IrcProtocol::setInfo(const QHash<QString, QString>& info)
{
    Q_D(IrcProtocol);
    IrcNetworkPrivate* priv = IrcNetworkPrivate::get(d->connection->network());
    priv->setInfo(info);
}

void IrcProtocol::setAvailableCapabilities(const QSet<QString>& capabilities)
{
    Q_D(IrcProtocol);
    IrcNetworkPrivate* priv = IrcNetworkPrivate::get(d->connection->network());
    priv->setAvailableCapabilities(capabilities);
}

void IrcProtocol::setActiveCapabilities(const QSet<QString>& capabilities)
{
    Q_D(IrcProtocol);
    IrcNetworkPrivate* priv = IrcNetworkPrivate::get(d->connection->network());
    priv->setActiveCapabilities(capabilities);
}

void IrcProtocol::receiveMessage(IrcMessage* message)
{
    Q_D(IrcProtocol);
    IrcConnectionPrivate* priv = IrcConnectionPrivate::get(d->connection);
    priv->receiveMessage(message);
}

#include "moc_ircprotocol.cpp"

IRC_END_NAMESPACE
