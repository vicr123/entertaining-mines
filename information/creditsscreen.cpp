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
#include "creditsscreen.h"
#include "ui_creditsscreen.h"

#include <QShortcut>
#include <QLayout>
#include <pauseoverlay.h>

struct CreditsScreenPrivate {
    PauseOverlay* overlay;
};

CreditsScreen::CreditsScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreditsScreen)
{
    ui->setupUi(this);

    d = new CreditsScreenPrivate();
    d->overlay = new PauseOverlay(parent);
    d->overlay->pushOverlayWidget(this);

    QPalette pal = ui->scrollArea->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->scrollArea->setPalette(pal);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton->click();
    });

    ui->bottomSpacer->changeSize(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding);

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activatedAmbiguously, this, [=] {
        ui->backButton->click();
    });
}

CreditsScreen::~CreditsScreen()
{
    delete ui;
    delete d;
}

void CreditsScreen::on_backButton_clicked()
{
    d->overlay->popOverlayWidget([=] {
        emit done();
    });
}

void CreditsScreen::resizeEvent(QResizeEvent*event)
{
    int width = 0;
    if (this->width() > 600) {
        width = (this->width() - 600) / 2;
    }

    ui->spacer1->changeSize(width, 0, QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->spacer2->changeSize(width, 0, QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->creditsArea->layout()->invalidate();
}
