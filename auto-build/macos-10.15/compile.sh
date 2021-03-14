#!/bin/bash

MAKE_THREADS_CNT=-j8
MACOSX_DEPLOYMENT_TARGET=10.12

ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew install automake cmake fdk-aac git lame libass libtool libvorbis libvpx ninja opus sdl shtool texi2html theora wget x264 xvid yasm pkg-config

sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

rootPath=`pwd`

git clone --recursive https://github.com/newton-blockchain/wallet-desktop.git

cd wallet-desktop/Wallet/ThirdParty/rlottie
git fetch
git checkout master
git pull

cd $rootPath

mkdir ThirdParty
cd ThirdParty

git clone https://github.com/desktop-app/patches.git
cd patches
git checkout 10aeaf6
cd ../
git clone https://chromium.googlesource.com/external/gyp
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH="$PWD/depot_tools:$PATH"
cd gyp
git checkout 9f2a7bb1
git apply ../patches/gyp.diff
./setup.py build
sudo ./setup.py install
cd ../..

mkdir -p Libraries/macos
cd Libraries/macos
wget http://tukaani.org/xz/xz-5.0.5.tar.gz
tar -xvf xz-5.0.5.tar.gz
rm xz-5.0.5.tar.gz

LibrariesPath=`pwd`

git clone https://github.com/desktop-app/patches.git
cd patches
git checkout 10aeaf6
cd ..
git clone --branch 0.10.0 https://github.com/ericniebler/range-v3

cd xz-5.0.5
CFLAGS="-mmacosx-version-min=10.12" LDFLAGS="-mmacosx-version-min=10.12" ./configure --prefix=/usr/local/macos
make $MAKE_THREADS_CNT
sudo make install
cd ..

git clone https://github.com/desktop-app/zlib.git
cd zlib
CFLAGS="-mmacosx-version-min=10.12 -Werror=unguarded-availability-new" LDFLAGS="-mmacosx-version-min=10.12" ./configure --prefix=/usr/local/macos
make $MAKE_THREADS_CNT
sudo make install
cd ..

git clone https://github.com/openssl/openssl openssl_1_1_1
cd openssl_1_1_1
git checkout OpenSSL_1_1_1-stable
./Configure --prefix=/usr/local/macos darwin64-x86_64-cc -static -mmacosx-version-min=10.12
make build_libs $MAKE_THREADS_CNT
cd ..


git clone https://chromium.googlesource.com/crashpad/crashpad.git
cd crashpad
git checkout feb3aa3923
git apply ../patches/crashpad.diff
cd third_party/mini_chromium
git clone https://chromium.googlesource.com/chromium/mini_chromium
cd mini_chromium
git checkout 7c5b0c1ab4
git apply ../../../../patches/mini_chromium.diff
cd ../../gtest
git clone https://chromium.googlesource.com/external/github.com/google/googletest gtest
cd gtest
git checkout d62d6c6556
cd ../../..

git apply $rootPath/wallet-desktop/auto-build/macos-10.15/crashpad.patch

build/gyp_crashpad.py -Dmac_deployment_target=10.10
ninja -C out/Debug
ninja -C out/Release
cd ..

git clone git://code.qt.io/qt/qt5.git qt5_12_8
cd qt5_12_8
perl init-repository --module-subset=qtbase,qtimageformats
git checkout v5.12.8
git submodule update qtbase
git submodule update qtimageformats
cd qtbase
git apply ../../patches/qtbase_5_12_8.diff
cd ..

./configure -prefix "/usr/local/desktop-app/Qt-5.12.8" \
-debug-and-release \
-force-debug-info \
-opensource \
-confirm-license \
-static \
-opengl desktop \
-no-openssl \
-securetransport \
-nomake examples \
-nomake tests \
-platform macx-clang

make $MAKE_THREADS_CNT
sudo make install
cd ..

LibrariesPath=`pwd`

git clone https://github.com/newton-blockchain/ton.git
cd ton
git submodule init
git submodule update third-party/crc32c
mkdir build-debug
cd build-debug
cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ARCH= -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=$LibrariesPath/openssl_1_1_1/include -DOPENSSL_CRYPTO_LIBRARY=$LibrariesPath/openssl_1_1_1/libcrypto.a -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=$LibrariesPath/zlib -DZLIB_LIBRARY=/usr/local/macos/lib/libz.a -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.12 -DCMAKE_CXX_FLAGS="-stdlib=libc++" ..
make $MAKE_THREADS_CNT tonlib
cd ..
mkdir build
cd build
cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ARCH= -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=$LibrariesPath/openssl_1_1_1/include -DOPENSSL_CRYPTO_LIBRARY=$LibrariesPath/openssl_1_1_1/libcrypto.a -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=$LibrariesPath/zlib -DZLIB_LIBRARY=/usr/local/macos/lib/libz.a -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.12 -DCMAKE_CXX_FLAGS="-stdlib=libc++" -DCMAKE_BUILD_TYPE=Release ..
make $MAKE_THREADS_CNT tonlib

cd $rootPath/wallet-desktop/Wallet/
./configure.sh -D DESKTOP_APP_USE_PACKAGED=OFF

git apply $rootPath/wallet-desktop/auto-build/macos-10.15/wallet.patch

cd ../out

xcodebuild -list -project Wallet.xcodeproj

xcodebuild -scheme ALL_BUILD -configuration Release build
