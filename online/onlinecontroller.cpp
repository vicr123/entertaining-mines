/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "onlinecontroller.h"

#include <QJsonObject>
#include <online/onlinewebsocket.h>

struct OnlineControllerPrivate {
    OnlineController* instance = nullptr;
    OnlineWebSocket* ws = nullptr;

    QString discordJoinSecret;
};

OnlineControllerPrivate* OnlineController::d = new OnlineControllerPrivate();

OnlineController::OnlineController(QObject *parent) : QObject(parent)
{

}

OnlineController*OnlineController::instance()
{
    if (!d->instance) d->instance = new OnlineController();
    return d->instance;
}

bool OnlineController::isConnected()
{
    return d->ws != nullptr;
}

#include <QDebug>
void OnlineController::attachToWebSocket(OnlineWebSocket*ws)
{
    d->ws = ws;

    connect(ws, &OnlineWebSocket::disconnected, this, [=] {
        emit disconnected(ws->closeCode());

        //Clear out the WebSocket
        d->ws->deleteLater();
        d->ws = nullptr;
    });
    connect(ws, &OnlineWebSocket::jsonMessageReceived, this, &OnlineController::jsonMessage);
    connect(ws, &OnlineWebSocket::jsonMessageReceived, this, [=](QJsonDocument doc) {
        qDebug() << doc.toJson();
    });
}

void OnlineController::sendJson(QJsonDocument doc)
{
    d->ws->sendJson(doc);
}

void OnlineController::sendJsonO(QJsonObject object)
{
    if (d->ws == nullptr) {
        qWarning() << "Tried to send a message to an unopen socket";
        return;
    }
    d->ws->sendJsonO(object);
}

void OnlineController::setDiscordJoinSecret(QString joinSecret)
{
    d->discordJoinSecret = joinSecret;
}

QString OnlineController::discordJoinSecret()
{
    QString discordJoinSecret = d->discordJoinSecret;
    d->discordJoinSecret = "";
    return discordJoinSecret;
}

void OnlineController::close()
{
    d->ws->close();
}

