#include "settingsscreen.h"
#include "ui_settingsscreen.h"

#include <the-libs_global.h>
#include <QShortcut>
#include <QKeyEvent>
#include <musicengine.h>
#include <settingwidget.h>
#include <focusbarrier.h>
#include <gamepadconfigurationoverlay.h>

#define SECTION_LABEL(text) ([=] { \
        QLabel* label = new QLabel(this); \
        label->setText(text.toUpper()); \
        label->setMargin(9); \
        QFont fnt = label->font(); \
        fnt.setBold(true); \
        label->setFont(fnt); \
        return label; \
    })()

#define DESCRIPTION_LABEL(text) ([=] { \
        QLabel* label = new QLabel(this); \
        label->setText(text); \
        label->setMargin(9); \
        label->setWordWrap(true); \
        return label; \
    })()

struct SettingsScreenPrivate {
    QList<QWidget*> settings;
};

SettingsScreen::SettingsScreen(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SettingsScreen) {
    ui->setupUi(this);

    d = new SettingsScreenPrivate();

    SettingWidget* bgMusic = new SettingWidget(this, SettingWidget::Range, tr("Background Volume"), "audio/backgroundVol");
    SettingWidget* effectMusic = new SettingWidget(this, SettingWidget::Boolean, tr("Sound Effects"), "audio/effects");

    QPushButton* gamepadSettingsButton = new QPushButton();
    gamepadSettingsButton->setText(tr("Gamepad Settings"));
    connect(gamepadSettingsButton, &QPushButton::clicked, this, [ = ] {
        GamepadConfigurationOverlay* conf = new GamepadConfigurationOverlay(this);
        connect(conf, &GamepadConfigurationOverlay::done, conf, &GamepadConfigurationOverlay::deleteLater);
    });

    d->settings.append(SECTION_LABEL(tr("Audio")));
    d->settings.append(bgMusic);
    d->settings.append(effectMusic);
    d->settings.append(SECTION_LABEL(tr("Behaviour")));
    d->settings.append(new SettingWidget(this, SettingWidget::Boolean, tr("Marks"), "behaviour/marks"));
    d->settings.append(DESCRIPTION_LABEL(tr("When this is enabled, right clicking a flag will change it into the marked state first.")));
    d->settings.append(SECTION_LABEL(tr("Hardware")));
    d->settings.append(gamepadSettingsButton);

    FocusBarrier* bar1 = new FocusBarrier(this);
    bar1->setBounceWidget(d->settings.at(1));
    FocusBarrier* bar2 = new FocusBarrier(this);
    bar2->setBounceWidget(d->settings.at(d->settings.count() - 1));

    QWidget* previousWidget = bar1;
    ui->settingsLayout->addWidget(bar1);
    for (QWidget* w : d->settings) {
        SettingWidget* s = qobject_cast<SettingWidget*>(w);
        if (s) connect(s, QOverload<>::of(&SettingWidget::hasFocus), this, [ = ] {
            ui->scrollArea->ensureWidgetVisible(s);
        });

        ui->settingsLayout->addWidget(w);

        if (!qobject_cast<QLabel*>(w)) {
            QWidget::setTabOrder(previousWidget, w);
            previousWidget = w;
        }
    }
    ui->settingsLayout->addWidget(bar2);
    QWidget::setTabOrder(previousWidget, bar2);

    ui->settingsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

    this->setFocusProxy(d->settings.at(1));

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Back"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, [ = ] {
        QKeyEvent event(QKeyEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
        QApplication::sendEvent(QApplication::focusWidget(), &event);

        QKeyEvent event2(QKeyEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
        QApplication::sendEvent(QApplication::focusWidget(), &event2);
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [ = ] {
        ui->backButton->click();
    });

    QShortcut* escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [ = ] {
        ui->backButton->click();
    });
}

SettingsScreen::~SettingsScreen() {
    delete ui;
}

void SettingsScreen::updateSettings() {
    for (QWidget* w : d->settings) {
        SettingWidget* s = qobject_cast<SettingWidget*>(w);
        if (s) {
            s->updateSetting();
        }
    }
}

void SettingsScreen::on_backButton_clicked() {
    emit goBack();
}

void SettingsScreen::resizeEvent(QResizeEvent* event) {
    int width = 0;
    if (this->width() > SC_DPI(600)) {
        width = (this->width() - SC_DPI(600)) / 2;
    }

    ui->spacer1->changeSize(width, 0, QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->spacer2->changeSize(width, 0, QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->settingsArea->layout()->invalidate();
}
