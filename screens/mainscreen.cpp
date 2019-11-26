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
#include "mainscreen.h"
#include "ui_mainscreen.h"

#include <QApplication>
#include <QKeyEvent>
#include <gamepadbuttons.h>
#include <textinputoverlay.h>
#include <loadoverlay.h>
#include <musicengine.h>
#include <QSvgRenderer>
#include <QPainter>
#include <the-libs_global.h>
#include "information/creditsscreen.h"
#include "information/helpscreen.h"

MainScreen::MainScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainScreen)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::None);
    ui->bottomSpacer->changeSize(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding);

    ui->focusBarrierTop->setBounceWidget(ui->startEasy);
    ui->focusBarrierBottom->setBounceWidget(ui->exitButton);
    ui->focusBarrierInfoTop->setBounceWidget(ui->helpButton);
    ui->focusBarrierInfoBottom->setBounceWidget(ui->mainMenuButton);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Exit"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [=] {
        if (ui->stackedWidget->currentWidget() == ui->infoScreen) {
            ui->mainMenuButton->click();
        } else {
            ui->exitButton->click();
        }
    });

    ui->exitButton->setProperty("type", "destructive");

    QPalette pal = ui->stackedWidget->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->stackedWidget->setPalette(pal);
}

MainScreen::~MainScreen()
{
    delete ui;
}

void MainScreen::resizeEvent(QResizeEvent*event)
{
    int width;
    if (this->width() < SC_DPI(600)) {
        width = 0;
    } else {
        width = static_cast<int>(this->width() * 0.4);
    }
    ui->leftSpacing->changeSize(width, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->layout()->invalidate();
}

void MainScreen::paintEvent(QPaintEvent*event)
{
    QPainter painter(this);
    QSvgRenderer renderer(QStringLiteral(":/icons/background.svg"));

    QSize size = renderer.defaultSize();
    size.scale(this->size(), Qt::KeepAspectRatioByExpanding);

    QRect geometry;
    geometry.setSize(size);
    geometry.moveCenter(QPoint(this->width() / 2, this->height() / 2));
    renderer.render(&painter, geometry);

    QLinearGradient grad(QPoint(0, this->height()), QPoint(0, this->height() - SC_DPI(50)));
    grad.setColorAt(0, QColor(0, 0, 0, 127));
    grad.setColorAt(1, QColor(0, 0, 0, 0));

    painter.setBrush(grad);
    painter.setPen(Qt::transparent);
    painter.drawRect(0, 0, this->width(), this->height());
}

void MainScreen::on_startEasy_clicked()
{
    emit startGame(9, 9, 10);
}

void MainScreen::on_startIntermediate_clicked()
{
    emit startGame(16, 16, 40);
}

void MainScreen::on_startDifficult_clicked()
{
    emit startGame(30, 16, 99);
}

void MainScreen::on_exitButton_clicked()
{
    QApplication::exit();
}

void MainScreen::on_startCustom_clicked()
{
    bool canceled;

    int width = 15;
    int height = 15;
    int mines = 60;

    askWidth:
    width = TextInputOverlay::getInt(this, tr("How wide is the board?"), &canceled, width, 5, 99, QLineEdit::Normal);
    if (canceled) return;

    askHeight:
    height = TextInputOverlay::getInt(this, tr("How high is the board?"), &canceled, height, 5, 99, QLineEdit::Normal);
    if (canceled) goto askWidth;

    mines = TextInputOverlay::getInt(this, tr("How many mines are on this board?"), &canceled, mines, 1, static_cast<int>(width * height * 0.9) - 1, QLineEdit::Normal);
    if (canceled) goto askHeight;

    emit startGame(width, height, mines);
}

void MainScreen::on_loadButton_clicked()
{
//    QFile* f = new QFile("/home/victor/.local/share/theSuite/Entertaining Mines/saves/coolsave");
//    f->open(QFile::ReadWrite);
//    QDataStream str(f);
//    int magic;
//    QString string;
//    str >> magic >> magic;
//    str >> string;
//    QVariantMap meta;
//    str >> meta;

//    emit loadGame(&str);

    MusicEngine::playSoundEffect(MusicEngine::Selection);
    LoadOverlay* load = new LoadOverlay(this);
    connect(load, &LoadOverlay::loadData, this, [=](QDataStream* stream) {
        loadGame(stream);
    });
    load->load();
}


void MainScreen::on_settingsButton_clicked()
{
    emit openSettings();
}

void MainScreen::on_infoButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->infoScreen);
}

void MainScreen::on_mainMenuButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainPage);
}

void MainScreen::on_creditsButton_clicked()
{
    CreditsScreen* cred = new CreditsScreen(this);
    connect(cred, &CreditsScreen::done, cred, &CreditsScreen::deleteLater);
}

void MainScreen::on_helpButton_clicked()
{
    HelpScreen* help = new HelpScreen(this);
    connect(help, &HelpScreen::done, help, &HelpScreen::deleteLater);
}

void MainScreen::on_playOnlineButton_clicked()
{
    emit playOnline();
}
