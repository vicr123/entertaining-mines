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
#include "playercarouselitem.h"
#include "ui_playercarouselitem.h"

#include <tvariantanimation.h>
#include <QPainter>

struct PlayerCarouselItemPrivate {
    QString playerName;
    QColor playerCol;

    tVariantAnimation colHeight;
};

PlayerCarouselItem::PlayerCarouselItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerCarouselItem)
{
    ui->setupUi(this);
    d = new PlayerCarouselItemPrivate();

    d->colHeight.setStartValue(SC_DPI(3));
    d->colHeight.setEndValue(this->height());
    d->colHeight.setDuration(250);
    d->colHeight.setEasingCurve(QEasingCurve::Linear);
    connect(&d->colHeight, &tVariantAnimation::valueChanged, this, [=] {
        this->update();
    });
}

PlayerCarouselItem::~PlayerCarouselItem()
{
    delete d;
    delete ui;
}

void PlayerCarouselItem::setPlayerName(QString playerName)
{
    ui->playerNameLabel->setText(playerName);
    d->playerName = playerName;
}

void PlayerCarouselItem::setPlayerColor(QColor col)
{
    d->playerCol = col;
}

void PlayerCarouselItem::setProfilePicture(QImage picture)
{
    ui->profilePictureIcon->setPixmap(QPixmap::fromImage(picture));
}

void PlayerCarouselItem::setIsCurrentTurn(bool isCurrentTurn)
{
    if (isCurrentTurn) {
        d->colHeight.setDirection(tVariantAnimation::Forward);
    } else {
        d->colHeight.setDirection(tVariantAnimation::Backward);
    }
    d->colHeight.start();
}

void PlayerCarouselItem::resizeEvent(QResizeEvent*event)
{
    d->colHeight.setEndValue(this->height());
}

void PlayerCarouselItem::paintEvent(QPaintEvent*event)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(d->playerCol);

    painter.drawRect(0, this->height() - d->colHeight.currentValue().toInt(), this->width(), d->colHeight.currentValue().toInt());
}
