if "%APPVEYOR_REPO_TAG_NAME%"=="continuous" (

    exit 1

)

git submodule init
git submodule update

set QTDIR=C:\Qt\5.12\msvc2017_64
set PATH=%PATH%;%QTDIR%\bin
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

git clone https://github.com/vicr123/the-libs.git
cd the-libs
git checkout blueprint
qmake the-libs.pro "CONFIG+=release"
nmake release
nmake install
cd ..

git clone https://github.com/vicr123/libentertaining.git
cd libentertaining
qmake libentertaining.pro "CONFIG+=release"
nmake release
nmake install
cd ..

git clone https://github.com/vicr123/contemporary-theme.git
cd contemporary-theme
qmake Contemporary.pro "CONFIG+=release"
nmake release
cd ..

qmake ent-mines.pro "CONFIG+=release"
nmake release
mkdir deploy
mkdir deploy\styles
mkdir deploy\translations
mkdir deploy\audio
copy "contemporary-theme\release\Contemporary.dll" deploy\styles
copy release\entertaining-mines.exe deploy
copy translations\*.qm deploy\translations
copy audio\* deploy\audio
copy "C:\Program Files\thelibs\lib\the-libs.dll" deploy
copy "C:\Program Files\libentertaining\lib\entertaining.dll" deploy
cd deploy
windeployqt entertaining-mines.exe -network -quickwidgets -gamepad -svg -multimedia
