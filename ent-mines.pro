QT       += core gui svg gamepad
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
    game/gametile.cpp \
    main.cpp \
    gamewindow.cpp \
    screens/customgamescreen.cpp \
    screens/gamescreen.cpp \
    screens/mainscreen.cpp \
    screens/pausescreen.cpp

HEADERS += \
    game/gametile.h \
    gamewindow.h \
    screens/customgamescreen.h \
    screens/gamescreen.h \
    screens/mainscreen.h \
    screens/pausescreen.h

FORMS += \
    gamewindow.ui \
    screens/customgamescreen.ui \
    screens/gamescreen.ui \
    screens/mainscreen.ui \
    screens/pausescreen.ui
RESOURCES += \
    audio.qrc \
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

    INSTALLS += target desktop icon
}

win32 {
    # Include the-libs build tools
    include(C:/Program Files/thelibs/pri/buildmaster.pri)

    QT += thelib
    INCLUDEPATH += "C:/Program Files/thelibs/include" "C:/Program Files (x86)/libentertaining/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs -L"C:/Program Files (x86)/libentertaining/lib" -lentertaining
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

    INCLUDEPATH += "/usr/local/include/the-libs" "/usr/local/include/libentertaining"
    LIBS += -L/usr/local/lib -lthe-libs -lentertaining

    QMAKE_POST_LINK += $$quote(cp $${PWD}/dmgicon.icns $${PWD}/app-dmg-background.png $${OUT_PWD})
}
