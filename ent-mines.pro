QT       += core gui svg gamepad network websockets
TARGET   = entertaining-mines
SHARE_APP_NAME = entertaining-mines

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    game/congratulation.cpp \
    game/gameover.cpp \
    game/gametile.cpp \
    information/creditsscreen.cpp \
    information/helpscreen.cpp \
    main.cpp \
    gamewindow.cpp \
    online/cannedmessagebox.cpp \
    online/cannedmessagepopover.cpp \
    online/gamemodeselect.cpp \
    online/mainonlinescreen.cpp \
    online/onlinecontroller.cpp \
    online/onlinegamescreen.cpp \
    online/onlinejoinscreen.cpp \
    online/onlinelobbyscreen.cpp \
    online/onlinemenuscreen.cpp \
    online/playercarousel.cpp \
    online/playercarouselitem.cpp \
    screens/gamescreen.cpp \
    screens/mainscreen.cpp \
    screens/pausescreen.cpp \
    screens/settingsscreen.cpp

HEADERS += \
    game/congratulation.h \
    game/gameover.h \
    game/gametile.h \
    gamewindow.h \
    information/creditsscreen.h \
    information/helpscreen.h \
    online/cannedmessagebox.h \
    online/cannedmessagepopover.h \
    online/gamemodeselect.h \
    online/mainonlinescreen.h \
    online/onlinecontroller.h \
    online/onlinegamescreen.h \
    online/onlinejoinscreen.h \
    online/onlinelobbyscreen.h \
    online/onlinemenuscreen.h \
    online/playercarousel.h \
    online/playercarouselitem.h \
    screens/gamescreen.h \
    screens/mainscreen.h \
    screens/pausescreen.h \
    screens/settingsscreen.h

FORMS += \
    game/congratulation.ui \
    game/gameover.ui \
    gamewindow.ui \
    information/creditsscreen.ui \
    information/helpscreen.ui \
    online/cannedmessagebox.ui \
    online/cannedmessagepopover.ui \
    online/gamemodeselect.ui \
    online/mainonlinescreen.ui \
    online/onlinegamescreen.ui \
    online/onlinejoinscreen.ui \
    online/onlinelobbyscreen.ui \
    online/onlinemenuscreen.ui \
    online/playercarousel.ui \
    online/playercarouselitem.ui \
    screens/gamescreen.ui \
    screens/mainscreen.ui \
    screens/pausescreen.ui \
    screens/settingsscreen.ui
RESOURCES += \
    help.qrc \
    resources.qrc

QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/defaults.conf) $$shell_quote($$OUT_PWD);

unix:!macx:!android {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    QT += thelib entertaining
    TARGET = entertaining-mines

    target.path = /usr/bin

    desktop.path = /usr/share/applications
    desktop.files = com.vicr123.entertaining.mines.desktop

    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files = icons/entertaining-mines.svg

    audio.path = /usr/share/entertaining-mines/audio
    audio.files = audio/*

    defaults.files = defaults.conf
    defaults.path = /etc/entertaining-games/entertaining-mines/

    INSTALLS += target desktop icon audio defaults
}

win32 {
    # Include the-libs build tools
    include(C:/Program Files/thelibs/pri/buildmaster.pri)

    INCLUDEPATH += "C:/Program Files/thelibs/include" "C:/Program Files/libentertaining/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs -L"C:/Program Files/libentertaining/lib" -lentertaining
    RC_FILE = icon.rc
}

macx {
    # Include the-libs build tools
    include(/usr/local/share/the-libs/pri/buildmaster.pri)

    QT += macextras
    LIBS += -framework CoreFoundation -framework AppKit

    blueprint {
        TARGET = "Entertaining Mines Blueprint"
#        ICON = icon-bp.icns
    } else {
        TARGET = "Entertaining Mines"
        ICON = icon.icns
    }

    audio.files = audio/
    audio.path = Contents/audio

    QMAKE_BUNDLE_DATA += audio

    INCLUDEPATH += "/usr/local/include/the-libs" "/usr/local/include/libentertaining"
    LIBS += -L/usr/local/lib -lthe-libs -lentertaining

    QMAKE_POST_LINK += $$quote(cp $${PWD}/dmgicon.icns $${PWD}/app-dmg-background.png $${OUT_PWD})
}

android {
    # Include the-libs build tools
    include(/opt/thesuite-android/share/the-libs/pri/gentranslations.pri)

    QT += multimedia
    INCLUDEPATH += "/opt/thesuite-android/include/the-libs" "/opt/thesuite-android/include/libentertaining"
    LIBS += -L/opt/thesuite-android/libs/armeabi-v7a -lthe-libs -lentertaining

    ANDROID_EXTRA_LIBS = \
        /opt/thesuite-android/libs/armeabi-v7a/libentertaining.so \
        /opt/thesuite-android/libs/armeabi-v7a/libthe-libs.so \
        /opt/thesuite-android/openssl/libcrypto_1_1.so \
        /opt/thesuite-android/openssl/libssl_1_1.so

    ANDROID_EXTRA_PLUGINS = \
        /opt/thesuite-android/plugins/

    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    defaults.conf


