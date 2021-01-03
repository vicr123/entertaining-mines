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
#include "gamewindow.h"

#include <tapplication.h>
#include <QDir>
#include <entertaining.h>
#include <discordintegration.h>
#include <musicengine.h>
#include <tsettings.h>
#include <notificationengine.h>

int main(int argc, char* argv[]) {

#ifdef Q_OS_ANDROID
    qputenv("QT_SCALE_FACTOR", "1.5");
#endif

    tApplication a(argc, argv);
    a.registerCrashTrap();

    if (QDir("/usr/share/entertaining-mines").exists()) {
        a.setShareDir("/usr/share/entertaining-mines");
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/entertaining-mines/")).exists()) {
        a.setShareDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/entertaining-mines/"));
    }
    a.installTranslators();

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationIcon(QIcon::fromTheme("entertaining-mines", QIcon(":/icons/entertaining-mines.svg")));
    a.setApplicationVersion("0.1");
    a.setGenericName(QApplication::translate("main", "Minesweeper"));
//    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2019");
    a.setDesktopFileName("com.vicr123.entertaining.mines");
#ifdef T_BLUEPRINT_BUILD
    a.setApplicationName("Entertaining Mines Blueprint");
#else
    a.setApplicationName("Entertaining Mines");
#endif

    tSettings::registerDefaults(a.applicationDirPath() + "/defaults.conf");
    tSettings::registerDefaults("/etc/entertaining-games/entertaining-chess/defaults.conf");

    Entertaining::initialize();
    DiscordIntegration::makeInstance("638385511530102794", "");

    tSettings settings;
    QObject::connect(&settings, &tSettings::settingChanged, [ = ](QString key, QVariant value) {
        if (key == "audio/backgroundVol") {
            MusicEngine::setUserBackgroundVolume(value.toInt() / 100.0);
        } else if (key == "audio/effects") {
            MusicEngine::setMuteEffects(!value.toBool());
        }
    });
    MusicEngine::setUserBackgroundVolume(settings.value("audio/backgroundVol").toInt() / 100.0);
    MusicEngine::setMuteEffects(!settings.value("audio/effects").toBool());

    GameWindow w;
    w.show();

    NotificationEngine::setApplicationNotificationWindow(&w);

    return a.exec();
}
