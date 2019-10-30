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
#include "helpscreen.h"
#include "ui_helpscreen.h"

#include <pauseoverlay.h>
#include <QShortcut>

HelpScreen::HelpScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HelpScreen)
{
    ui->setupUi(this);

    PauseOverlay::overlayForWindow(this->parentWidget())->pushOverlayWidget(this);

    QPalette pal = ui->scrollArea->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->scrollArea->setPalette(pal);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->backButton->click();
    });

    QShortcut* backShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(backShortcut, &QShortcut::activatedAmbiguously, this, [=] {
        ui->backButton->click();
    });

    ui->helpBrowser->setSearchPaths({"qrc:/resources/help/"});
    ui->helpBrowser->setSource(QUrl("qrc:/resources/help/en.html"));
    this->setFocusProxy(ui->helpBrowser);
    ui->helpBrowser->setFocus();
}

HelpScreen::~HelpScreen()
{
    delete ui;
}

void HelpScreen::on_backButton_clicked()
{
    PauseOverlay::overlayForWindow(this->parentWidget())->popOverlayWidget([=] {
        emit done();
    });
}

void HelpScreen::resizeEvent(QResizeEvent*event)
{
    int width = 0;
    if (this->width() > 600) {
        width = (this->width() - 600) / 8;
    }

    ui->spacer1->changeSize(width, 0, QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->spacer2->changeSize(width, 0, QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->helpArea->layout()->invalidate();
}
