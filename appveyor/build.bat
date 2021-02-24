if "%APPVEYOR_REPO_TAG_NAME%"=="continuous" (

    exit 1

)

git submodule init
git submodule update

set QTDIR=C:\Qt\5.15\msvc2019_64
set PATH=%PATH%;%QTDIR%\bin
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

git clone https://github.com/discordapp/discord-rpc.git
cd discord-rpc
mkdir build
cd build
cmake -G"NMake Makefiles" .. -DBUILD_EXAMPLES=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --build . --config Release --target install
cd ..
cd ..

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
copy "C:\OpenSSL-v111-Win64\bin\libssl-1_1-x64.dll" deploy
copy "C:\OpenSSL-v111-Win64\bin\libcrypto-1_1-x64.dll" deploy
copy defaults.conf deploy
cd deploy
windeployqt entertaining-mines.exe -network -quickwidgets -gamepad -svg -multimedia -printsupport -concurrent
