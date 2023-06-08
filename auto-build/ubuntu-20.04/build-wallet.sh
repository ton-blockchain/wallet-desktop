#!/bin/bash

MAKE_THREADS_CNT=-j16

WALLET_REPO_ACCOUNT="ton-blockchain"
WALLET_BRANCH="master"

TONLIB_REPO_ACCOUNT="newton-blockchain"
TONLIB_BRANCH="wallets"

PATCH_REPO_ACCOUNT="desktop-app"
PATCH_COMMIT="10aeaf6"

function red() { echo -e -n "\n\033[1;31m$1\033[0m\n\n"; }
function yel() { echo -e -n "\n\033[1;33m$1\033[0m\n\n"; }
function grn() { echo -e -n "\033[1;32m$1\033[0m\n"; }
function blu() { echo -e -n "\033[1;34m$1\033[0m\n"; }

if [ ! -f "system-deps-installed" ]; then
  yel "Installing neccessary system dependencies"
  apt update && apt install git && \
  apt install python2.7 python2.7-minimal libpython2.7-minimal libpython2.7-stdlib -y && \
  update-alternatives --install /usr/bin/python python /usr/bin/python2.7 10 && \
  apt-get install software-properties-common -y && \
  apt-get install git libexif-dev liblzma-dev libz-dev libssl-dev \
    libgtk2.0-dev libice-dev libsm-dev libicu-dev libdrm-dev dh-autoreconf \
    autoconf automake build-essential libxml2-dev libass-dev libfreetype6-dev \
    libgpac-dev libsdl1.2-dev libtheora-dev libtool libva-dev libvdpau-dev \
    libvorbis-dev libenchant-dev libxcb1-dev libxcb-image0-dev libxcb-shm0-dev \
    libxcb-xfixes0-dev libxcb-keysyms1-dev libxcb-icccm4-dev libatspi2.0-dev \
    libxcb-render-util0-dev libxcb-util0-dev libxcb-xkb-dev libxrender-dev \
    libasound-dev libpulse-dev libxcb-sync0-dev libxcb-randr0-dev libegl1-mesa-dev \
    libx11-xcb-dev libffi-dev libncurses5-dev pkg-config texi2html bison yasm \
    zlib1g-dev xutils-dev chrpath gperf -y && \
  add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
  apt-get update && \
  apt-get install gcc-8 g++-8 -y && \
  update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 && \
  update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60 && \
  update-alternatives --config gcc && \
  add-apt-repository --remove ppa:ubuntu-toolchain-r/test -y && \
  touch system-deps-installed
  if [ -f "system-deps-installed" ]; then
    grn "System dependencies installed successfully"
  else
    red "Installation failed"
    exit 2
  fi
fi

if [ ! -d 'wallet-desktop' ]; then
  yel "Cloning $WALLET_REPO_ACCOUNT/wallet-desktop:$WALLET_BRANCH"
  git clone -b "$WALLET_BRANCH" --recursive "https://github.com/$WALLET_REPO_ACCOUNT/wallet-desktop.git"
  cd wallet-desktop || exit
    git submodule update --remote Wallet/lib_wallet
  cd .. # wallet-desktop
  grn "wallet-desktop completed"
else
  blu "wallet-desktop already present"
fi

mkdir -p Libraries
cd Libraries || exit

if [ ! -d 'cmake' ]; then
  yel "Cloning and building cmake"
  git clone https://github.com/Kitware/CMake cmake
  cd cmake || exit
    git checkout v3.16.0
    ./bootstrap
    make $MAKE_THREADS_CNT
    make install
  cd .. # cmake
  grn "cmake completed"
else
  blu "cmake already present"
fi

if [ ! -d "patches" ]; then
  yel "Cloning patches"
  git clone "https://github.com/$PATCH_REPO_ACCOUNT/patches.git"
  cd patches || exit
    git checkout "$PATCH_COMMIT"
  cd .. # patches
  grn "patches completed"
else
  blu "patches already present"
fi

if [ ! -d 'range-v3' ]; then
  yel "Cloning range-v3"
  git clone --branch 0.10.0 https://github.com/ericniebler/range-v3
  grn "range-v3 completed"
else
  blu "range-v3 already present"
fi

if [ ! -d 'openssl_1_1_1' ]; then
  yel "Cloning and building openssl 1.1.1"
  git clone https://github.com/openssl/openssl openssl_1_1_1
  cd openssl_1_1_1 || exit
    git checkout OpenSSL_1_1_1-stable
    ./config --prefix=/usr/local/desktop-app/openssl-1.1.1
    make $MAKE_THREADS_CNT
    make install
  cd .. # openssl_1_1_1
  grn "openssl completed"
else
  blu "openssl already present"
fi

if [ ! -d 'libxkbcommon' ]; then
  yel "Cloning and building libxkbcommon"
  git clone https://github.com/xkbcommon/libxkbcommon.git
  cd libxkbcommon || exit
    git checkout xkbcommon-0.8.4
    ./autogen.sh --disable-x11
    make $MAKE_THREADS_CNT
    make install
  cd .. # libxkbcommon
  grn "libxkbcommon completed"
else
  blu "libxkbcommon already present"
fi

if [ ! -d 'googletest' ]; then
  yel "Cloning and building googletest"
  git clone https://github.com/google/googletest
  cd googletest || exit
    mkdir build
    cd build || exit
      cmake ..
      make
      cp lib/*.a /usr/lib
    cd .. # build
  cd .. # googletest
  grn "googletest completed"
else
  blu "googletest already present"
fi

if [ ! -d 'qt_5_12_8' ]; then
  yel "Cloning and building qt 5.12.8"
  git clone git://code.qt.io/qt/qt5.git qt_5_12_8
  cd qt_5_12_8 || exit
    perl init-repository --module-subset=qtbase,qtwayland,qtimageformats,qtsvg
    git checkout v5.12.8
    git submodule update qtbase
    git submodule update qtwayland
    git submodule update qtimageformats
    git submodule update qtsvg
    cd qtbase || exit
      git apply ../../patches/qtbase_5_12_8.diff
      cd src/plugins/platforminputcontexts || exit
        git clone https://github.com/desktop-app/fcitx.git
        git clone https://github.com/desktop-app/hime.git
        git clone https://github.com/desktop-app/nimf.git
      cd ../../.. # src/plugins/platforminputcontexts
    cd .. # qtbase

    OPENSSL_DIR=/usr/local/desktop-app/openssl-1.1.1
    ./configure -prefix "/usr/local/desktop-app/Qt-5.12.8" \
      -release -force-debug-info -opensource -confirm-license \
      -qt-zlib -qt-libpng -qt-libjpeg -qt-harfbuzz -qt-pcre -qt-xcb \
      -system-freetype -fontconfig -no-gtk -static -dbus-runtime \
      -openssl-linked -I "$OPENSSL_DIR/include" -I "/usr/include/drm" \
      OPENSSL_LIBS="$OPENSSL_DIR/lib/libssl.a $OPENSSL_DIR/lib/libcrypto.a -ldl -lpthread" \
      -nomake examples -nomake tests

    make $MAKE_THREADS_CNT
    make install
  cd .. # qt_5_12_8
  grn "qt completed"
else
  blu "qt already present"
fi

if [ ! -d 'gyp' ]; then
  yel "Cloning gyp"
  git clone https://chromium.googlesource.com/external/gyp
  cd gyp || exit
    git checkout 9f2a7bb1
    git apply ../patches/gyp.diff
  cd .. # gyp
  grn "gyp completed"
else
  blu "gyp already present"
fi

if [ ! -d 'breakpad' ]; then
  yel "Cloning and building breakpad"
  git clone https://chromium.googlesource.com/breakpad/breakpad
  cd breakpad || exit
    git checkout bc8fb886
    git clone https://chromium.googlesource.com/linux-syscall-support src/third_party/lss
    cd src/third_party/lss || exit
      git checkout a91633d1
    cd ../../.. # src/third_party_lss
    ./configure

    cp -R ../googletest/googletest src/testing/
    cp -R ../googletest/googlemock src/testing/

    cp src/tools/mac/symupload/minidump_upload.m src/tools/linux/symupload/

    # fix extern - static function compilation error
    sed -i 's/static int tgkill/int tgkill/g' src/client/linux/handler/exception_handler.cc

    make $MAKE_THREADS_CNT
    make install
    cd src/tools || exit
      ../../../gyp/gyp  --depth=. --generator-output=.. -Goutput_dir=../out tools.gyp --format=cmake
    cd ../.. # src/tools
    cd out/Default || exit
      cmake .
      make $MAKE_THREADS_CNT dump_syms
    cd ../.. # out/Default
  cd .. # breakpad
  grn "breakpad completed"
else
  blu "breakpad already present"
fi

if [ ! -d 'ton' ]; then
  yel "Cloning and building $TONLIB_REPO_ACCOUNT/ton:$TONLIB_BRANCH"
  git clone --single-branch --branch "$TONLIB_BRANCH" "https://github.com/$TONLIB_REPO_ACCOUNT/ton.git"
  cd ton || exit
    git submodule init
    git submodule update third-party/crc32c
    mkdir build-debug
    mkdir build
    cd build-debug || exit
      cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ONLY_TONLIB=ON \
        -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=/usr/local/desktop-app/openssl-1.1.1/include \
        -DOPENSSL_CRYPTO_LIBRARY=/usr/local/desktop-app/openssl-1.1.1/lib/libcrypto.a \
        -DTON_ARCH="$(uname -m | sed --expression='s/_/-/g')" ..
    cd .. # build-debug
    cd build || exit
      cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ONLY_TONLIB=ON \
        -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=/usr/local/desktop-app/openssl-1.1.1/include \
        -DOPENSSL_CRYPTO_LIBRARY=/usr/local/desktop-app/openssl-1.1.1/lib/libcrypto.a \
        -DTON_ARCH="$(uname -m | sed --expression='s/_/-/g')" -DCMAKE_BUILD_TYPE=Release ..
    cd .. # build
  cd .. # ton
  grn "ton (tonlib) completed"
else
  blu "ton (tonlib) already present"
fi

cd ton/build || exit
  yel "Building tonlib"
  make $MAKE_THREADS_CNT tonlib
  grn "tonlib build completed"
cd ../.. # ton/build

cd .. # Libraries

if [ ! -f "wallet-cmake-patched" ]; then
  yel "patching qt CMakeLists"
  cd wallet-desktop || exit
    # temporary even more dirty workaround
    sed -i '238d' cmake/external/qt/CMakeLists.txt
    sed -i '238d' cmake/external/qt/CMakeLists.txt
    sed -i '238d' cmake/external/qt/CMakeLists.txt
    # sed -i '242d' cmake/external/qt/CMakeLists.txt
    sed -i 's/xkbcommon-x11/dbus-1/g' cmake/external/qt/CMakeLists.txt
  cd ..
  touch "wallet-cmake-patched"
  grn "qt CMakeLists patched"
fi

cd wallet-desktop || exit

  cd Wallet || exit
    yel "Configuring Wallet"
    ./configure.sh -D DESKTOP_APP_USE_PACKAGED=OFF
    grn "Confugration done"
  cd ..

  cd out/Release || exit
    yel "Building Wallet (Release)"
    make $MAKE_THREADS_CNT
    grn "Build done"
  cd ../.. # out/Release

cd .. # wallet-desktop

blu "Stripping symbols"
cp wallet-desktop/out/Release/bin/Wallet ./
strip ./Wallet
grn "All done!"