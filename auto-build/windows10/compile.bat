mkdir ThirdParty
mkdir Libraries

cd ThirdParty
curl -o strawberry.zip -LO https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-64bit-portable.zip
unzip -q strawberry.zip -d Strawberry
del strawberry.zip

curl -o install.msi https://www.python.org/ftp/python/2.7.8/python-2.7.8.amd64.msi
msiexec /i install.msi /quiet /qn /norestart TARGETDIR="%cd%\Python27"
del install.msi

curl -o cmake.zip -LO https://github.com/Kitware/CMake/releases/download/v3.19.4/cmake-3.19.4-win64-x64.zip
unzip -q cmake.zip -d cmake
del cmake.zip

curl -o nasm.zip https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-win64.zip
unzip -q nasm.zip -d NASM
del nasm.zip

curl -o yasm.zip http://www.tortall.net/projects/yasm/releases/vsyasm-1.3.0-win64.zip
unzip -q yasm.zip -d yasm
del yasm.zip
curl -o ninja.zip -LO https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-win.zip
unzip -q ninja.zip -d Ninja
del ninja.zip
curl -o jom.zip http://www.mirrorservice.org/sites/download.qt-project.org/official_releases/jom/jom_1_1_3.zip
unzip -q jom.zip -d jom
del jom.zip

git clone https://github.com/desktop-app/patches.git
cd patches
git checkout 10aeaf6
cd ..
git clone https://chromium.googlesource.com/external/gyp
cd gyp
git checkout 9f2a7bb1
git apply ../patches/gyp.diff
cd ..\..


SET PATH=%cd%\ThirdParty\Strawberry\perl\bin;%cd%\ThirdParty\NASM\nasm-2.15.05;%cd%\ThirdParty\Python27;%cd%\ThirdParty\jom;%cd%\ThirdParty\cmake\bin;%cd%\ThirdParty\yasm;%cd%\ThirdParty\gyp;%cd%\ThirdParty\Ninja;%PATH%
echo %PATH%

cd Libraries
SET LibrariesPath=%cd%

SET GYP_MSVS_OVERRIDE_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise
SET GYP_MSVS_VERSION=2019

git clone https://github.com/desktop-app/patches.git
cd patches
git checkout 10aeaf6
cd ..
git clone --branch 0.10.0 https://github.com/ericniebler/range-v3 range-v3


copy ..\zlib.zip .
unzip -q zlib.zip -d zlib
dir zlib

copy ..\lzma.zip .
unzip -q lzma.zip -d lzma
dir lzma


git clone https://github.com/openssl/openssl.git openssl_1_1_1
cd openssl_1_1_1
git checkout OpenSSL_1_1_1i

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

copy ..\breakpad.zip .
unzip -q breakpad.zip -d breakpad
dir breakpad

git clone git://code.qt.io/qt/qt5.git qt_5_12_8
cd qt_5_12_8
perl init-repository --module-subset=qtbase,qtimageformats
git checkout v5.12.8
git submodule update qtbase
git submodule update qtimageformats


call configure -prefix "%LibrariesPath%\Qt-5.12.8" -debug-and-release -force-debug-info -opensource -confirm-license -static -static-runtime -I "%LibrariesPath%\openssl_1_1_1\include" -no-opengl -openssl-linked OPENSSL_LIBS_DEBUG="%LibrariesPath%\openssl_1_1_1\out32.dbg\libssl.lib %LibrariesPath%\openssl_1_1_1\out32.dbg\libcrypto.lib Ws2_32.lib Gdi32.lib Advapi32.lib Crypt32.lib User32.lib" OPENSSL_LIBS_RELEASE="%LibrariesPath%\openssl_1_1_1\out32\libssl.lib %LibrariesPath%\openssl_1_1_1\out32\libcrypto.lib Ws2_32.lib Gdi32.lib Advapi32.lib Crypt32.lib User32.lib" -mp -nomake examples -nomake tests -platform win32-msvc

call jom -j4

call jom -j4 install

cd ..

git clone --recursive https://github.com/newton-blockchain/ton.git
cd ton
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

cd %LibrariesPath%\..
git clone --recursive https://github.com/newton-blockchain/wallet-desktop.git

cd wallet-desktop\Wallet
python --version
call configure.bat -D DESKTOP_APP_USE_PACKAGED=OFF
cd ..\out
msbuild Wallet.sln /property:Configuration=Release /p:platform=win32

dir Release
