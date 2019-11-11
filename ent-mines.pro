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
    online/mainonlinescreen.cpp \
    online/onlinemenuscreen.cpp \
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
    online/mainonlinescreen.h \
    online/onlinemenuscreen.h \
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
    online/mainonlinescreen.ui \
    online/onlinemenuscreen.ui \
    screens/gamescreen.ui \
    screens/mainscreen.ui \
    screens/pausescreen.ui \
    screens/settingsscreen.ui
RESOURCES += \
    help.qrc \
    resources.qrc


unix:!macx {
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

    INSTALLS += target desktop icon audio
}

win32 {
    # Include the-libs build tools
    include(C:/Program Files/thelibs/pri/buildmaster.pri)

    INCLUDEPATH += "C:/Program Files/thelibs/include" "C:/Program Files/libentertaining/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs -L"C:/Program Files/libentertaining/lib" -lentertaining
#    RC_FILE = icon.rc
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
