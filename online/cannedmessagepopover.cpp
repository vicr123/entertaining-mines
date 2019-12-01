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
#include "cannedmessagepopover.h"
#include "ui_cannedmessagepopover.h"

#include <the-libs_global.h>
#include <QStringListModel>
#include <QShortcut>

CannedMessagePopover::CannedMessagePopover(QStringList cannedMessages, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CannedMessagePopover)
{
    ui->setupUi(this);

    ui->cannedMessagesList->setFixedWidth(SC_DPI(600));
    ui->cannedMessagesList->setModel(new QStringListModel(cannedMessages));
    this->setFocusProxy(ui->cannedMessagesList);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [=] {
        on_cannedMessagesList_activated(ui->cannedMessagesList->currentIndex());
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton->click();
    });

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activated, this, [=] {
        ui->backButton->click();
    });
    connect(backShortcut, &QShortcut::activatedAmbiguously, this, [=] {
        ui->backButton->click();
    });
}

CannedMessagePopover::~CannedMessagePopover()
{
    delete ui;
}

void CannedMessagePopover::on_cannedMessagesList_activated(const QModelIndex &index)
{
    if (this->isDone) return;
    this->isDone = true;

    emit sendCannedMessage(index.row());
}

void CannedMessagePopover::on_backButton_clicked()
{
    emit done();
}
