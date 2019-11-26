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
#ifndef ONLINECONTROLLER_H
#define ONLINECONTROLLER_H

#include <QObject>
#include <QJsonDocument>

struct OnlineControllerPrivate;
class OnlineWebSocket;
class OnlineController : public QObject
{
        Q_OBJECT
    public:
        explicit OnlineController(QObject *parent = nullptr);

        static OnlineController* instance();

        bool isConnected();

        void attachToWebSocket(OnlineWebSocket* ws);
        void sendJson(QJsonDocument doc);
        void sendJsonO(QJsonObject object);

        void setDiscordJoinSecret(QString joinSecret);
        QString discordJoinSecret();

        int ping();

    public slots:
        void close();

    signals:
        void disconnected(int closeCode);
        void jsonMessage(QJsonDocument doc);
        void pingChanged();

    private:
        static OnlineControllerPrivate* d;
};

#endif // ONLINECONTROLLER_H
