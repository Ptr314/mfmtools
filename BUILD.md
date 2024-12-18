# Настройка окружения и компиляция приложения

Программа написана как проект cmake с использованием библиотеки Qt6. Далее описан процесс настройки окружения и компиляции под Qt Creator и mingw.

Версии под другие ОС и MSVC пока в планах.

## Windows

### MINGW

#### 1. Установить программы
* https://download.qt.io/, скачать online-инсталлятор (возможно, из России понадобится зарубежный VPN) и установить следующие компоненты:
    * Qt [X.X.X]
        * MinGW YY.Y.Y
        * Sources (Если не нужна компиляция статической версии для релиза, можно пропустить)
    * Qt Developer and Designer tools
        * Qt Creator
        * Mingw YY.Y.Y (Версия, соответствующая компилятору библиотеки в предыдущем пункте)
        * cmake
        * ninja

#### 2. Настроить debug-версию в Qt Creator
* Если нужна полная очистка, удалить файлы __CMakeLists.txt.user*__.

#### 2.1. Обновление языковых файлов
~~~
cd папка-с-CMakeLists.txt-проекта
\путь-к-программе\cmake.exe --build build/Desktop_Qt_6_8_1_MinGW_64_bit-Debug --target update_translations
~~~
* Путь после __--build__ должен указывать на build-директорию, установленную в конфигурации проекта.
* Команду надо выполнять на той же платформе, где происходил препроцессинг CMakeLists.txt.
* Далее файлы .ts редактируются с помощью Linguist из Qt Creator.

#### 2.2. Компиляция статической версии Qt

##### Qt6 (актуальная версия)

https://doc.qt.io/qt-6/windows-building.html

* Отредактировать `.build/vars-mingw-latest.cmd` на действительные пути.
* Открыть командную строку и скомпилировать Qt:

```
cd репозиторий-приложения\.build
%SystemRoot%\system32\cmd.exe /E:ON /V:ON /k vars-mingw-latest.cmd
cd C:\Temp
mkdir qt-build
cd qt-build
configure.bat -static -static-runtime -release  -opensource -confirm-license -nomake examples -nomake tests -prefix c:\DEV\Qt\%_QT_VERSION%-static
cmake --build . --parallel
cmake --install .
```

##### Qt5 для Windows XP

Для XP необходима версия Qt 5.6.3 и mingw 4.9.2 (https://download.qt.io/new_archive/qt/5.6/5.6.3/single/)

~~~
cd репозиторий-приложения\.build
%SystemRoot%\system32\cmd.exe /E:ON /V:ON /k vars-mingw-qt5.6.cmd
cd C:\Temp
mkdir qt5.6-build
cd qt5.6-build
configure.bat -release -nomake examples -nomake tests -opensource -confirm-license -no-opengl -target xp -no-directwrite -no-compile-examples -prefix c:\DEV\Qt\%_QT_VERSION%
mingw32-make
mingw32-make install
~~~

Примечания: 
* `-target xp` необходимо для компиляции в формат .exe Windows XP.
* Использовать статическую версию не удается, поэтому в дальнейшем необходимо в папку программы поместить следующие файлы:
    * `Qt5Core.dll`, `Qt5Gui.dll`, `Qt5Widgets.dll` из `Qt/5.6.3/bin/`
    * `platforms/qwindows.dll` из `Qt/5.6.3/plugins`
    * `libgcc_s_dw2-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll` из `Qt/Tools/mingw492_32/bin`

##### Qt5 для Windows 7

Для Windows 7 необходима версия Qt 5.15 и mingw 8.1.0 (https://download.qt.io/archive/qt/5.15/5.15.16/single/)

~~~
cd репозиторий-приложения\.build
%SystemRoot%\system32\cmd.exe /E:ON /V:ON /k vars-mingw-qt5.15.cmd
cd C:\Temp
mkdir qt5.15-build
cd qt5.15-build
configure.bat -static -release -nomake examples -nomake tests -opensource -confirm-license -no-opengl -skip qtlocation -prefix c:\DEV\Qt-static\%_QT_VERSION%
mingw32-make
mingw32-make install
~~~

Примечания: 
* -no-opengl используется для исключения установки OpenGL SDK.
* -skip qtlocation исключает непонятную ошибку компиляции в этом модуле

#### 3. Сборка release-версии
* Проверить, что в Qt Creator/Kits/Compilers есть компилятор, который использовался для сборки Qt.
* Добавить собранную версию Qt в Qt Creator/Kits/Versions и Qt Creator/Kits/Kits. Версия компилятора должна соответствовать версии, с которой происходила сборка Qt.
* Обновить версию приложения в CMakeLists.txt, пересканировать проект (Rescan project), чтобы версия прописалась в заголовочные файлы.
* Закоммитить изменения.
* Откомпилировать приложение нужной версией Qt.
* Собрать архив:
    * Файл exe;
    * Директорию build/XXX/languages с файлами .qm.
    * Содержимое директории deploy (значки языков, файлы конфигурации, лицензию и пр.).
    * Файлы runtime-библиотеки для нестатических сборок.
* Загрузить как релиз на GitHub, добавив последнему коммиту тег с номером версии.


## macOS

https://doc.qt.io/qt-6/macos.html

Далее описывается установка окружения из offline-инсталляторов, так как сетевая установка под виртуальными машинами работала некорректно.

1. Установить xcode. Дистрибутив взять здесь: https://xcodereleases.com, нужен аккаунт на Apple Developer.
    * Скопировать файл .xip в папку Applications и там распаковать. Файл .xip удалить
    * Выполнить команду `sudo xcode-select --switch /Applications/Xcode.app`
2. Установить HomeBrew: https://brew.sh/
3. Установить cmake, ninja, принять лицензию xcode:
```
brew install cmake
brew install ninja
sudo xcodebuild -license
```
4. С https://download.qt.io/official_releases/qtcreator/latest/: скачать Qt Creator Offline Installer и с https://download.qt.io/official_releases/qt/ Qt Sources (qt-everywhere-src-X.X.X.tar.xz)
    * Установить Qt Creator обычным образом.
    * qt-everywhere-src-X.X.X.tar.xz поместить в ~/Downloads

5. Собрать Qt

https://doc.qt.io/qt-6/macos-building.html

```
cd /tmp
tar xf ~/Downloads/qt-everywhere-src-6.8.1.tar.xz
mkdir -p ~/dev/qt-build
cd ~/dev/qt-build
/tmp/qt-everywhere-src-6.8.1/configure -static -static-runtime -release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -make libs -prefix /usr/local/Qt-6.8.1-static
cmake --build . --parallel
sudo cmake --install .
```

После перезагрузки системы /tmp очищается, поэтому для повторного запуска надо распаковывать исходники заново.

6. Добавить Kit в Qt Creator из папки /usr/local/Qt-X.X.X-static (включить отображение скрытых папок при необходимости). 