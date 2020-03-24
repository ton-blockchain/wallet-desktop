# Build instructions for Visual Studio 2019

- [Prepare folder](#prepare-folder)
- [Install third party software](#install-third-party-software)
- [Clone source code and prepare libraries](#clone-source-code-and-prepare-libraries)
- [Build the project](#build-the-project)

## Prepare folder

Choose an empty folder for the future build, for example **D:\\Projects**. It will be named ***BuildPath*** in the rest of this document. Create two folders there, ***BuildPath*\\ThirdParty** and ***BuildPath*\\Libraries**.

All commands (if not stated otherwise) will be launched from **x86 Native Tools Command Prompt for VS 2019.bat** (should be in **Start Menu > Visual Studio 2019** menu folder). Pay attention not to use any other Command Prompt.

## Install third party software

* Download **Strawberry Perl** installer from [http://strawberryperl.com/](http://strawberryperl.com/) and install to ***BuildPath*\\ThirdParty\\Strawberry**
* Download **NASM** installer from [http://www.nasm.us](http://www.nasm.us) and install to ***BuildPath*\\ThirdParty\\NASM**
* Download **Yasm** executable from [http://yasm.tortall.net/Download.html](http://yasm.tortall.net/Download.html), rename to *yasm.exe* and put to ***BuildPath*\\ThirdParty\\yasm**
* Download **jom** archive from [http://download.qt.io/official_releases/jom/jom.zip](http://download.qt.io/official_releases/jom/jom.zip) and unpack to ***BuildPath*\\ThirdParty\\jom**
* Download **Python 2.7** installer from [https://www.python.org/downloads/](https://www.python.org/downloads/) and install to ***BuildPath*\\ThirdParty\\Python27**
* Download **CMake** installer from [https://cmake.org/download/](https://cmake.org/download/) and install to ***BuildPath*\\ThirdParty\\cmake**
* Download **Ninja** executable from [https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-win.zip](https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-win.zip) and unpack to ***BuildPath*\\ThirdParty\\Ninja**

Open **x86 Native Tools Command Prompt for VS 2019.bat**, go to ***BuildPath*** and run

    cd ThirdParty
    git clone https://github.com/desktop-app/patches.git
    cd patches
    git checkout 395b620
    cd ../
    git clone https://chromium.googlesource.com/external/gyp
    cd gyp
    git checkout 9f2a7bb1
    git apply ../patches/gyp.diff
    cd ..\..

Add **GYP** and **Ninja** to your PATH:

* Open **Control Panel** -> **System** -> **Advanced system settings**
* Press **Environment Variables...**
* Select **Path**
* Press **Edit**
* Add ***BuildPath*\\ThirdParty\\gyp** value
* Add ***BuildPath*\\ThirdParty\\Ninja** value

## Clone source code and prepare libraries

Open **x86 Native Tools Command Prompt for VS 2019.bat**, go to ***BuildPath*** and run

    SET PATH=%cd%\ThirdParty\Strawberry\perl\bin;%cd%\ThirdParty\Python27;%cd%\ThirdParty\NASM;%cd%\ThirdParty\jom;%cd%\ThirdParty\cmake\bin;%cd%\ThirdParty\yasm;%PATH%

    git clone --recursive https://github.com/ton-blockchain/wallet-desktop.git

    mkdir Libraries
    cd Libraries

    SET LibrariesPath=%cd%

    git clone https://github.com/desktop-app/patches.git
    cd patches
    git checkout 395b620
    cd ..
    git clone --branch 0.10.0 https://github.com/ericniebler/range-v3 range-v3

    git clone https://github.com/openssl/openssl.git openssl_1_1_1
    cd openssl_1_1_1
    git checkout OpenSSL_1_1_1-stable
    perl Configure no-shared debug-VC-WIN32
    nmake
    mkdir out32.dbg
    move libcrypto.lib out32.dbg
    move libssl.lib out32.dbg
    move ossl_static.pdb out32.dbg\ossl_static
    nmake clean
    move out32.dbg\ossl_static out32.dbg\ossl_static.pdb
    perl Configure no-shared VC-WIN32
    nmake
    mkdir out32
    move libcrypto.lib out32
    move libssl.lib out32
    move ossl_static.pdb out32
    cd ..

    git clone https://github.com/desktop-app/zlib.git
    cd zlib\contrib\vstudio\vc14
    msbuild zlibstat.vcxproj /property:Configuration=Debug
    msbuild zlibstat.vcxproj /property:Configuration=ReleaseWithoutAsm
    cd ..\..\..\..

    git clone https://github.com/desktop-app/lzma.git
    cd lzma\C\Util\LzmaLib
    msbuild LzmaLib.sln /property:Configuration=Debug
    msbuild LzmaLib.sln /property:Configuration=Release
    cd ..\..\..\..

    git clone https://github.com/google/breakpad
    cd breakpad
    git checkout a1dbcdcb43
    git apply ../patches/breakpad.diff
    cd src
    git clone https://github.com/google/googletest testing
    cd client\windows
    gyp --no-circular-check breakpad_client.gyp --format=ninja
    cd ..\..
    ninja -C out/Debug common crash_generation_client exception_handler
    ninja -C out/Release common crash_generation_client exception_handler
    cd tools\windows\dump_syms
    gyp dump_syms.gyp
    msbuild dump_syms.vcxproj /property:Configuration=Release
    cd ..\..\..\..\..

    git clone git://code.qt.io/qt/qt5.git qt_5_12_5
    cd qt_5_12_5
    perl init-repository --module-subset=qtbase,qtimageformats
    git checkout v5.12.5
    git submodule update qtbase
    git submodule update qtimageformats
    cd qtbase
    git apply ../../patches/qtbase_5_12_5.diff
    cd ..

    configure -prefix "%LibrariesPath%\Qt-5.12.5" -debug-and-release -force-debug-info -opensource -confirm-license -static -static-runtime -I "%LibrariesPath%\openssl_1_1_1\include" -no-opengl -openssl-linked OPENSSL_LIBS_DEBUG="%LibrariesPath%\openssl_1_1_1\out32.dbg\libssl.lib %LibrariesPath%\openssl_1_1_1\out32.dbg\libcrypto.lib Ws2_32.lib Gdi32.lib Advapi32.lib Crypt32.lib User32.lib" OPENSSL_LIBS_RELEASE="%LibrariesPath%\openssl_1_1_1\out32\libssl.lib %LibrariesPath%\openssl_1_1_1\out32\libcrypto.lib Ws2_32.lib Gdi32.lib Advapi32.lib Crypt32.lib User32.lib" -mp -nomake examples -nomake tests -platform win32-msvc

    jom -j4
    jom -j4 install
    cd ..

    git clone https://github.com/ton-blockchain/ton.git
    cd ton
    git checkout 950e2922
    git submodule init
    git submodule update third-party/crc32c
    mkdir build-debug
    cd build-debug
    cmake -A Win32 -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ARCH= -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=%LibrariesPath%\openssl_1_1_1\include -DOPENSSL_CRYPTO_LIBRARY=%LibrariesPath%\openssl_1_1_1\out32.dbg\libcrypto.lib -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=%LibrariesPath%\zlib -DZLIB_LIBRARY=%LibrariesPath%\zlib\contrib\vstudio\vc14\x86\ZlibStatDebug\zlibstat.lib -DCMAKE_CXX_FLAGS_DEBUG="/DZLIB_WINAPI /DNDEBUG /MTd /Zi /Od /Ob0" -DCMAKE_C_FLAGS_DEBUG="/DNDEBUG /MTd /Zi /Od /Ob0" -DCMAKE_EXE_LINKER_FLAGS="/SAFESEH:NO Ws2_32.lib Gdi32.lib Advapi32.lib Crypt32.lib User32.lib" ..
    cmake --build . --target tonlib --config Debug
    cd ..
    mkdir build
    cd build
    cmake -A Win32 -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ARCH= -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=%LibrariesPath%\openssl_1_1_1\include -DOPENSSL_CRYPTO_LIBRARY=%LibrariesPath%\openssl_1_1_1\out32\libcrypto.lib -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=%LibrariesPath%\zlib -DZLIB_LIBRARY=%LibrariesPath%\zlib\contrib\vstudio\vc14\x86\ZlibStatReleaseWithoutAsm\zlibstat.lib -DCMAKE_CXX_FLAGS_RELEASE="/DZLIB_WINAPI /MT /Ob2" -DCMAKE_C_FLAGS_RELEASE="/MT /Ob2" -DCMAKE_EXE_LINKER_FLAGS="/SAFESEH:NO Ws2_32.lib Gdi32.lib Advapi32.lib Crypt32.lib User32.lib" ..
    cmake --build . --target tonlib --config Release
    cd ../..

## Build the project

Go to ***BuildPath*\\wallet-desktop\\Wallet** and run

    configure.bat -D DESKTOP_APP_USE_PACKAGED=OFF

* Open ***BuildPath*\\wallet-desktop\\out\\Wallet.sln** in Visual Studio 2019
* Select Wallet project and press Build > Build Wallet (Debug and Release configurations)
* The result Wallet.exe will be located in ***BuildPath*\\wallet-desktop\\out\\Debug** (and **Release**)
