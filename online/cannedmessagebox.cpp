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
#include <the-libs_global.h>

struct CannedMessageBoxPrivate {
    QGraphicsOpacityEffect* effect;
    QWidget* carouselItem;
};

CannedMessageBox::CannedMessageBox(QString cannedMessage, QWidget*carouselItem, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CannedMessageBox)
{
    ui->setupUi(this);

    d = new CannedMessageBoxPrivate();
    d->carouselItem = carouselItem;

    this->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->setContentsMargins(SC_DPI(9), SC_DPI(9), SC_DPI(9), SC_DPI(14));

    ui->messageLabel->setText(cannedMessage);

    carouselItem->installEventFilter(this);
    parent->installEventFilter(this);

    updatePosition();
    this->show();

    QTimer::singleShot(5000, this, &CannedMessageBox::hide);
}

CannedMessageBox::~CannedMessageBox()
{
    delete ui;
}

void CannedMessageBox::hide()
{
    QWidget::hide();
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

void CannedMessageBox::updatePosition()
{
    QPoint mappingPoint(d->carouselItem->width() / 2, 0);
    QPoint centerBottom = d->carouselItem->mapTo(this->parentWidget(), mappingPoint);

    QRect geometry;
    geometry.setSize(this->sizeHint());
    geometry.moveCenter(centerBottom - QPoint(0, this->sizeHint().height() / 2));
    this->setGeometry(geometry);
}
