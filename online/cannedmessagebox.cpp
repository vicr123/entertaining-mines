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
#include "cannedmessagebox.h"
#include "ui_cannedmessagebox.h"

#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <musicengine.h>
#include <QPointer>
#include "playercarouselitem.h"
#include <the-libs_global.h>
#include <tvariantanimation.h>

struct CannedMessageBoxPrivate {
    QGraphicsOpacityEffect* effect;
    QPointer<PlayerCarouselItem> carouselItem;

    bool hiding = false;
};

CannedMessageBox::CannedMessageBox(QString cannedMessage, QWidget* carouselItem, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CannedMessageBox)
{
    ui->setupUi(this);

    d = new CannedMessageBoxPrivate();
    d->carouselItem = qobject_cast<PlayerCarouselItem*>(carouselItem);
    d->effect = new QGraphicsOpacityEffect();

    if (d->carouselItem->cannedMessageBox()) {
        d->carouselItem->cannedMessageBox()->hide();
    }

    d->carouselItem->setCannedMessageBox(this);

    this->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->setContentsMargins(SC_DPI(9), SC_DPI(9), SC_DPI(9), SC_DPI(14));
    this->setGraphicsEffect(d->effect);

    ui->messageLabel->setText(cannedMessage);

    carouselItem->installEventFilter(this);
    parent->installEventFilter(this);

    this->show();

    QTimer::singleShot(5000, this, &CannedMessageBox::hide);
    MusicEngine::playSoundEffect("message");
}

CannedMessageBox::~CannedMessageBox()
{
    delete d;
    delete ui;
}

void CannedMessageBox::show()
{
    d->effect->setOpacity(0);
    QRect startGeom = this->endGeometry();
    startGeom.translate(0, SC_DPI(50));

    this->setGeometry(startGeom);

    QWidget::show();

    tVariantAnimation* geom = new tVariantAnimation();
    geom->setStartValue(startGeom);
    geom->setEndValue(this->endGeometry());
    geom->setDuration(500);
    geom->setEasingCurve(QEasingCurve::OutCubic);
    connect(geom, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        this->setGeometry(value.toRect());
    });
    connect(geom, &tVariantAnimation::finished, geom, &tVariantAnimation::deleteLater);
    geom->start();

    tVariantAnimation* opac = new tVariantAnimation();
    opac->setStartValue(0.0);
    opac->setEndValue(1.0);
    opac->setDuration(500);
    opac->setEasingCurve(QEasingCurve::OutCubic);
    connect(opac, &tVariantAnimation::valueChanged, opac, [=](QVariant value) {
        d->effect->setOpacity(value.toDouble());
    });
    connect(opac, &tVariantAnimation::finished, opac, &tVariantAnimation::deleteLater);
    opac->start();
}

void CannedMessageBox::hide()
{
    if (d->hiding) return;
    d->hiding = true;

    if (!d->carouselItem.isNull()) {
        d->carouselItem->setCannedMessageBox(nullptr);
    }

    tVariantAnimation* opac = new tVariantAnimation();
    opac->setStartValue(1.0);
    opac->setEndValue(0.0);
    opac->setDuration(500);
    opac->setEasingCurve(QEasingCurve::OutCubic);
    connect(opac, &tVariantAnimation::valueChanged, opac, [=](QVariant value) {
        d->effect->setOpacity(value.toDouble());
    });
    connect(opac, &tVariantAnimation::finished, this, [=] {
        QWidget::hide();
        opac->deleteLater();
        this->deleteLater();
    });
    opac->start();
}

void CannedMessageBox::paintEvent(QPaintEvent*event)
{
    QPainter painter(this);

    int base = this->height() - 1 - SC_DPI(5);

    QPolygon pol;
    pol.append(QPoint(0, 0));
    pol.append(QPoint(this->width() - 1, 0));
    pol.append(QPoint(this->width() - 1, base));
    pol.append(QPoint((this->width() - 1) / 2 + SC_DPI(5), base));
    pol.append(QPoint((this->width() - 1) / 2, this->height() - 1));
    pol.append(QPoint((this->width() - 1) / 2 - SC_DPI(5), base));
    pol.append(QPoint(0, base));

    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.setBrush(this->palette().color(QPalette::Window));
    painter.drawPolygon(pol);
}

bool CannedMessageBox::eventFilter(QObject*watched, QEvent*event)
{
    if (event->type() == QEvent::Move || event->type() == QEvent::Resize) {
        updatePosition();
    }
    return false;
}

QRect CannedMessageBox::endGeometry()
{
    QPoint mappingPoint(d->carouselItem->width() / 2, 0);
    QPoint centerBottom = d->carouselItem->mapTo(this->parentWidget(), mappingPoint);

    QRect geometry;
    geometry.setSize(this->sizeHint());
    geometry.moveCenter(centerBottom - QPoint(0, this->sizeHint().height() / 2));
    return geometry;
}

void CannedMessageBox::updatePosition()
{
    this->setGeometry(this->endGeometry());
}
