# Build instructions for Visual Studio 2019

- [Prepare folder](#prepare-folder)
- [Install third party software](#install-third-party-software)
- [Clone source code and prepare libraries](#clone-source-code-and-prepare-libraries)
- [Build the project](#build-the-project)

## Prepare folder

Choose an empty folder for the future build, for example **D:\\Projects**. It will be named ***BuildPath*** in the rest of this document. Create two folders there, ***BuildPath*\\ThirdParty** and ***BuildPath*\\Libraries**.

All commands (if not stated otherwise) will be launched from **x86 Native Tools Command Prompt for VS 2019.bat** (should be in **Start Menu > Visual Studio 2019** menu folder). Pay attention not to use any other Command Prompt.

## Install third party software

* Download **ActivePerl** installer from [https://www.activestate.com/activeperl/downloads](https://www.activestate.com/activeperl/downloads) and install to ***BuildPath*\\ThirdParty\\Perl**
* Download **NASM** installer from [http://www.nasm.us](http://www.nasm.us) and install to ***BuildPath*\\ThirdParty\\NASM**
* Download **Yasm** executable from [http://yasm.tortall.net/Download.html](http://yasm.tortall.net/Download.html), rename to *yasm.exe* and put to ***BuildPath*\\ThirdParty\\yasm**
* Download **jom** archive from [http://download.qt.io/official_releases/jom/jom.zip](http://download.qt.io/official_releases/jom/jom.zip) and unpack to ***BuildPath*\\ThirdParty\\jom**
* Download **Python 2.7** installer from [https://www.python.org/downloads/](https://www.python.org/downloads/) and install to ***BuildPath*\\ThirdParty\\Python27**
* Download **CMake** installer from [https://cmake.org/download/](https://cmake.org/download/) and install to ***BuildPath*\\ThirdParty\\cmake**
* Download **Ninja** executable from [https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-win.zip](https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-win.zip) and unpack to ***BuildPath*\\ThirdParty\\Ninja**

Open **x86 Native Tools Command Prompt for VS 2019.bat**, go to ***BuildPath*** and run

    cd ThirdParty
    git clone https://github.com/desktop-app/patches.git
    git clone https://chromium.googlesource.com/external/gyp
    cd gyp
    git checkout 9f2a7bb1
    git apply ../patches/gyp.diff
    cd ..

Add **GYP** and **Ninja** to your PATH:

* Open **Control Panel** -> **System** -> **Advanced system settings**
* Press **Environment Variables...**
* Select **Path**
* Press **Edit**
* Add ***BuildPath*\\ThirdParty\\gyp** value
* Add ***BuildPath*\\ThirdParty\\Ninja** value

## Clone source code and prepare libraries

Open **x86 Native Tools Command Prompt for VS 2019.bat**, go to ***BuildPath*** and run

    SET PATH=%cd%\ThirdParty\Perl\bin;%cd%\ThirdParty\Python27;%cd%\ThirdParty\NASM;%cd%\ThirdParty\jom;%cd%\ThirdParty\cmake\bin;%cd%\ThirdParty\yasm;%PATH%

    git clone --recursive https://github.com/ton-blockchain/wallet.git

    mkdir Libraries
    cd Libraries

    git clone https://github.com/desktop-app/patches.git

    git clone --branch 0.5.0 https://github.com/ericniebler/range-v3 range-v3

    git clone https://github.com/openssl/openssl.git
    cd openssl
    git checkout OpenSSL_1_0_1-stable
    perl Configure no-shared --prefix="C:\Program Files (x86)\OpenSSL" --openssldir="C:\Program Files (x86)\Common Files\SSL" VC-WIN32
    ms\do_ms
    nmake -f ms\nt.mak
    perl Configure no-shared --prefix="C:\Program Files (x86)\OpenSSL" --openssldir="C:\Program Files (x86)\Common Files\SSL" debug-VC-WIN32
    ms\do_ms
    nmake -f ms\nt.mak
    cd ..

    git clone https://github.com/desktop-app/zlib.git
    cd zlib\contrib\vstudio\vc14
    msbuild zlibstat.vcxproj /property:Configuration=Debug
    msbuild zlibstat.vcxproj /property:Configuration=ReleaseWithoutAsm
    cd ..\..\..\..

    git clone git://code.qt.io/qt/qt5.git qt5_6_2
    cd qt5_6_2
    perl init-repository --module-subset=qtbase,qtimageformats
    git checkout v5.6.2
    cd qtimageformats
    git checkout v5.6.2
    cd ..\qtbase
    git checkout v5.6.2
    git apply ../../patches/qtbase_5_6_2.diff
    cd ..

    configure -debug-and-release -force-debug-info -opensource -confirm-license -static -I "%cd%\..\openssl\inc32" -no-opengl -openssl-linked OPENSSL_LIBS_DEBUG="%cd%\..\openssl\out32.dbg\ssleay32.lib %cd%\..\openssl\out32.dbg\libeay32.lib" OPENSSL_LIBS_RELEASE="%cd%\..\openssl\out32\ssleay32.lib %cd%\..\openssl\out32\libeay32.lib" -mp -nomake examples -nomake tests -platform win32-msvc2015

    jom -j4
    jom -j4 install
    cd ..

    SET LibrariesPath=%cd%

    git clone https://github.com/ton-blockchain/ton.git
    cd ton
    git checkout ecb3e06a06
    git submodule init
    git submodule update third-party/crc32c
    mkdir build-debug
    cd build-debug
    cmake -A Win32 -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ARCH= -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=%LibrariesPath%\openssl\inc32 -DOPENSSL_CRYPTO_LIBRARY=%LibrariesPath%\openssl\out32.dbg\libeay32.lib -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=%LibrariesPath%\zlib -DZLIB_LIBRARIES=%LibrariesPath%\zlib\contrib\vstudio\vc14\x86\ZlibStatDebug\zlibstat.lib -DCMAKE_CXX_FLAGS_DEBUG="/DZLIB_WINAPI /DNDEBUG /MTd /Zi /Od /Ob0" -DCMAKE_C_FLAGS_DEBUG="/DNDEBUG /MTd /Zi /Od /Ob0" -DCMAKE_EXE_LINKER_FLAGS="/SAFESEH:NO" ..
    cmake --build . --target tonlib --config Debug
    cd ..
    mkdir build
    cd build
    cmake -A Win32 -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ARCH= -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=%LibrariesPath%\openssl\inc32 -DOPENSSL_CRYPTO_LIBRARY=%LibrariesPath%\openssl\out32\libeay32.lib -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=%LibrariesPath%\zlib -DZLIB_LIBRARIES=%LibrariesPath%\zlib\contrib\vstudio\vc14\x86\ZlibStatReleaseWithoutAsm\zlibstat.lib -DCMAKE_CXX_FLAGS_RELEASE="/DZLIB_WINAPI /MT /Ob2" -DCMAKE_C_FLAGS_RELEASE="/MT /Ob2" -DCMAKE_EXE_LINKER_FLAGS="/SAFESEH:NO" ..
    cmake --build . --target tonlib --config Release
    cd ../..

    cd ../wallet/Wallet
    gyp\refresh.bat

## Build the project

* Open ***BuildPath*\\wallet\\Wallet\\Wallet.sln** in Visual Studio 2019
* Select Wallet project and press Build > Build Wallet (Debug and Release configurations)
* The result Wallet.exe will be located in ***BuildPath*\\wallet\\out\\Debug** (and **Release**)
