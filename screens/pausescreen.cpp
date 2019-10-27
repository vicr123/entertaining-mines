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
#include "pausescreen.h"
#include "ui_pausescreen.h"

#include <QShortcut>
#include <gamepadbuttons.h>
#include <QKeyEvent>
#include <pauseoverlay.h>
#include <musicengine.h>
#include <saveoverlay.h>

struct PauseScreenPrivate {
    PauseOverlay* overlay;
};

PauseScreen::PauseScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PauseScreen)
{
    ui->setupUi(this);
    d = new PauseScreenPrivate();

    d->overlay = new PauseOverlay(parent);
    d->overlay->pushOverlayWidget(this);

    this->setFocusProxy(ui->resumeButton);

    ui->focusBarrierTop->setBounceWidget(ui->resumeButton);
    ui->focusBarrierBottom->setBounceWidget(ui->mainMenuButton);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Resume"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [=] {
        QKeyEvent event(QKeyEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
        QApplication::sendEvent(QApplication::focusWidget(), &event);

        QKeyEvent event2(QKeyEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
        QApplication::sendEvent(QApplication::focusWidget(), &event2);
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        ui->resumeButton->click();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonStart, [=] {
        ui->resumeButton->click();
    });

    ui->mainMenuButton->setProperty("type", "destructive");

    QShortcut* pauseShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(pauseShortcut, &QShortcut::activatedAmbiguously, this, [=] {
        ui->resumeButton->click();
    });
}

PauseScreen::~PauseScreen()
{
    delete d;
    delete ui;
}

void PauseScreen::show()
{

}

void PauseScreen::on_resumeButton_clicked()
{
    MusicEngine::playSoundEffect(MusicEngine::Backstep);
    MusicEngine::playBackgroundMusic();
    d->overlay->popOverlayWidget([=] {
        emit resume();
    });
}

void PauseScreen::resizeEvent(QResizeEvent*event)
{
    ui->leftSpacing->changeSize(this->width() * 0.1, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->layout()->invalidate();
}

void PauseScreen::on_mainMenuButton_clicked()
{
    MusicEngine::playSoundEffect(MusicEngine::Selection);
    d->overlay->popOverlayWidget([=] {
        emit mainMenu();
    });
}

void PauseScreen::on_saveButton_clicked()
{
    SaveOverlay* save = new SaveOverlay(this, d->overlay);
    connect(save, &SaveOverlay::provideMetadata, this, &PauseScreen::provideMetadata);
    connect(save, &SaveOverlay::provideSaveData, this, [=](QDataStream* stream) {
        emit provideSaveData(stream);
    });
    save->save();
}
