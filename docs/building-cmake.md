## Build instructions for GYP/CMake under Ubuntu 14.04

### Prepare folder

Choose an empty folder for the future build, for example **/home/user/Projects**. It will be named ***BuildPath*** in the rest of this document.

### Install software and required packages

You will need GCC 8 installed. To install them and all the required dependencies run

    sudo apt-get install software-properties-common -y && \
    sudo apt-get install git libexif-dev liblzma-dev libz-dev libssl-dev \
    libgtk2.0-dev libice-dev libsm-dev libicu-dev libdrm-dev dh-autoreconf \
    autoconf automake build-essential libxml2-dev libass-dev libfreetype6-dev \
    libgpac-dev libsdl1.2-dev libtheora-dev libtool libva-dev libvdpau-dev \
    libvorbis-dev libenchant-dev libxcb1-dev libxcb-image0-dev libxcb-shm0-dev \
    libxcb-xfixes0-dev libxcb-keysyms1-dev libxcb-icccm4-dev libatspi2.0-dev \
    libxcb-render-util0-dev libxcb-util0-dev libxcb-xkb-dev libxrender-dev \
    libasound-dev libpulse-dev libxcb-sync0-dev libxcb-randr0-dev libegl1-mesa-dev \
    libx11-xcb-dev libffi-dev libncurses5-dev pkg-config texi2html bison yasm \
    zlib1g-dev xutils-dev python-xcbgen chrpath gperf -y --force-yes && \
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    sudo apt-get update && \
    sudo apt-get install gcc-8 g++-8 -y && \
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 && \
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60 && \
    sudo update-alternatives --config gcc && \
    sudo add-apt-repository --remove ppa:ubuntu-toolchain-r/test -y

You can set the multithreaded make parameter by running

    MAKE_THREADS_CNT=-j8

### Clone source code and prepare libraries and cmake

Go to ***BuildPath*** and run

    git clone --recursive https://github.com/ton-blockchain/wallet-desktop.git

    mkdir Libraries
    cd Libraries

    git clone https://github.com/Kitware/CMake cmake
    cd cmake
    git checkout v3.16.0
    ./bootstrap
    make $MAKE_THREADS_CNT
    sudo make install
    cd ..

    git clone https://github.com/desktop-app/patches.git
    cd patches
    git checkout 10aeaf6
    cd ../
    git clone --branch 0.10.0 https://github.com/ericniebler/range-v3

    git clone https://github.com/openssl/openssl openssl_1_1_1
    cd openssl_1_1_1
    git checkout OpenSSL_1_1_1-stable
    ./config --prefix=/usr/local/desktop-app/openssl-1.1.1
    make $MAKE_THREADS_CNT
    sudo make install
    cd ..

    git clone https://github.com/xkbcommon/libxkbcommon.git
    cd libxkbcommon
    git checkout xkbcommon-0.8.4
    ./autogen.sh --disable-x11
    make $MAKE_THREADS_CNT
    sudo make install
    cd ..

    git clone git://code.qt.io/qt/qt5.git qt_5_12_8
    cd qt_5_12_8
    perl init-repository --module-subset=qtbase,qtwayland,qtimageformats,qtsvg
    git checkout v5.12.8
    git submodule update qtbase
    git submodule update qtwayland
    git submodule update qtimageformats
    git submodule update qtsvg
    cd qtbase
    git apply ../../patches/qtbase_5_12_8.diff
    cd src/plugins/platforminputcontexts
    git clone https://github.com/desktop-app/fcitx.git
    git clone https://github.com/desktop-app/hime.git
    git clone https://github.com/desktop-app/nimf.git
    cd ../../../..

    OPENSSL_DIR=/usr/local/desktop-app/openssl-1.1.1
    ./configure -prefix "/usr/local/desktop-app/Qt-5.12.8" \
    -release \
    -force-debug-info \
    -opensource \
    -confirm-license \
    -qt-zlib \
    -qt-libpng \
    -qt-libjpeg \
    -qt-harfbuzz \
    -qt-pcre \
    -qt-xcb \
    -system-freetype \
    -fontconfig \
    -no-gtk \
    -static \
    -dbus-runtime \
    -openssl-linked \
    -I "$OPENSSL_DIR/include" OPENSSL_LIBS="$OPENSSL_DIR/lib/libssl.a $OPENSSL_DIR/lib/libcrypto.a -ldl -lpthread" \
    -nomake examples \
    -nomake tests

    make $MAKE_THREADS_CNT
    sudo make install
    cd ..

    git clone https://chromium.googlesource.com/external/gyp
    cd gyp
    git checkout 9f2a7bb1
    git apply ../patches/gyp.diff
    cd ..

    git clone https://chromium.googlesource.com/breakpad/breakpad
    cd breakpad
    git checkout bc8fb886
    git clone https://chromium.googlesource.com/linux-syscall-support src/third_party/lss
    cd src/third_party/lss
    git checkout a91633d1
    cd ../../..
    ./configure
    make $MAKE_THREADS_CNT
    sudo make install
    cd src/tools
    ../../../gyp/gyp  --depth=. --generator-output=.. -Goutput_dir=../out tools.gyp --format=cmake
    cd ../../out/Default
    cmake .
    make $MAKE_THREADS_CNT dump_syms
    cd ../../..

    git clone https://github.com/ton-blockchain/ton.git
    cd ton
    git checkout be9c34c6
    git submodule init
    git submodule update third-party/crc32c
    mkdir build-debug
    cd build-debug
    cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=/usr/local/desktop-app/openssl-1.1.1/include -DOPENSSL_CRYPTO_LIBRARY=/usr/local/desktop-app/openssl-1.1.1/lib/libcrypto.a -DTON_ARCH=`uname -m | sed --expression='s/_/-/g'` ..
    make $MAKE_THREADS_CNT tonlib
    cd ..
    mkdir build
    cd build
    cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=/usr/local/desktop-app/openssl-1.1.1/include -DOPENSSL_CRYPTO_LIBRARY=/usr/local/desktop-app/openssl-1.1.1/lib/libcrypto.a -DTON_ARCH=`uname -m | sed --expression='s/_/-/g'` -DCMAKE_BUILD_TYPE=Release ..
    make $MAKE_THREADS_CNT tonlib
    cd ../..

### Building the project

Go to ***BuildPath*/wallet-desktop/Wallet** and run

    ./configure.sh -D DESKTOP_APP_USE_PACKAGED=OFF

To make Debug version go to ***BuildPath*/wallet-desktop/out/Debug** and run

    make $MAKE_THREADS_CNT

To make Release version go to ***BuildPath*/wallet-desktop/out/Release** and run

    make $MAKE_THREADS_CNT

You can debug your builds from Qt Creator, just open ***BuildPath*/wallet-desktop/CMakeLists.txt**, configure to a separate directory with correct options and launch with debug.
