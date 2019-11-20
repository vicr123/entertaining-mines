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
#include "playercarousel.h"
#include "ui_playercarousel.h"

#include "onlinecontroller.h"
#include "playercarouselitem.h"
#include <online/onlineapi.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointer>
#include <musicengine.h>
#include <tvariantanimation.h>

struct PlayerCarouselPrivate {
    QMap<int, PlayerCarouselItem*> items;
    int currentTurn = -1;

    int thisSessionId = -1;

    QString gamemode;
    bool collapsed = false;
};

PlayerCarousel::PlayerCarousel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerCarousel)
{
    ui->setupUi(this);
    d = new PlayerCarouselPrivate();

    ui->leftSpacer->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->rightSpacer->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);

    ui->carouselLayout->installEventFilter(this);

    connect(OnlineController::instance(), &OnlineController::jsonMessage, this, [=](QJsonDocument doc) {
        QJsonObject obj = doc.object();
        QString type = obj.value("type").toString();
        if (type == "sessionIdChanged") {
            d->thisSessionId = obj.value("session").toInt();
        } else if (type == "roomUpdate") {
            for (PlayerCarouselItem* item : d->items.values()) {
                ui->carouselLayout->removeWidget(item);
                item->deleteLater();
            }
            d->items.clear();

            QJsonArray users = obj.value("users").toArray();
            for (QJsonValue val : users) {
                QJsonObject user = val.toObject();
                int session = user.value("session").toInt();

                QPointer<PlayerCarouselItem> item(new PlayerCarouselItem());
                item->setPlayerName(user.value("username").toString());
                item->setPlayerColor(QColor(user.value("colour").toVariant().toUInt()));
                ui->carouselLayout->addWidget(item);
                d->items.insert(session, item);

                OnlineApi::instance()->profilePicture(user.value("picture").toString(), SC_DPI(32))->then([=](QImage image) {
                    if (!item.isNull()) item->setProfilePicture(image);
                });
            }

            this->setCurrentPlayer(d->currentTurn);
        } else if (type == "hostUpdate") {
//            if (obj.value("isHost").toBool()) {
//                ui->settingsWidget->setEnabled(true);
//            } else {
//                ui->settingsWidget->setEnabled(false);
//            }
        } else if (type == "gamemodeChange") {
            d->gamemode = obj.value("gamemode").toString();
        } else if (type == "currentPlayerChange") {
            this->setCurrentPlayer(obj.value("session").toInt());

            if (d->currentTurn == d->thisSessionId) MusicEngine::playSoundEffect("yourturn");
        } else if (type == "endGame") {
            this->setCurrentPlayer(-1);
        }
    });
}

PlayerCarousel::~PlayerCarousel()
{
    delete d;
    delete ui;
}

int PlayerCarousel::preferredHeight()
{
    return ui->carouselWidget->sizeHint().height();
}

void PlayerCarousel::collapse()
{
    d->collapsed = true;
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->height());
    anim->setEndValue(0);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
    anim->start();
}

void PlayerCarousel::expand()
{
    d->collapsed = false;
    this->setFixedHeight(this->preferredHeight());
}

void PlayerCarousel::setCurrentPlayer(int session)
{
    if (d->items.contains(d->currentTurn)) {
        d->items.value(d->currentTurn)->setIsCurrentTurn(false);
    }

    d->currentTurn = session;

    if (d->items.contains(d->currentTurn)) {
        d->items.value(d->currentTurn)->setIsCurrentTurn(true);
    }
}

bool PlayerCarousel::eventFilter(QObject*watched, QEvent*event)
{
    if (watched == ui->carouselLayout && event->type() == QEvent::LayoutRequest) {
        if (!d->collapsed) this->setFixedHeight(this->preferredHeight());
    }
    return false;
}
