## Build instructions for GYP/CMake under Ubuntu 14.04

### Prepare folder

Choose an empty folder for the future build, for example **/home/user/Projects**. It will be named ***BuildPath*** in the rest of this document.

### Install software and required packages

You will need GCC 8.1 installed. To install them and all the required dependencies run

    sudo apt-get install software-properties-common -y && \
    sudo apt-get install git libexif-dev liblzma-dev libz-dev libssl-dev libappindicator-dev libicu-dev libdee-dev libdrm-dev dh-autoreconf autoconf automake build-essential libass-dev libfreetype6-dev libgpac-dev libsdl1.2-dev libtheora-dev libtool libva-dev libvdpau-dev libvorbis-dev libxcb1-dev libxcb-image0-dev libxcb-shm0-dev libxcb-xfixes0-dev libxcb-keysyms1-dev libxcb-icccm4-dev libxcb-render-util0-dev libxcb-util0-dev libxrender-dev libasound-dev libpulse-dev libxcb-sync0-dev libxcb-randr0-dev libx11-xcb-dev libffi-dev libncurses5-dev pkg-config texi2html zlib1g-dev yasm xutils-dev bison python-xcbgen chrpath -y && \

    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    sudo apt-get update && \
    sudo apt-get install gcc-8 g++-8 -y && \
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 && \
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60 && \
    sudo update-alternatives --config gcc && \
    sudo add-apt-repository --remove ppa:ubuntu-toolchain-r/test -y && \

You can set the multithreaded make parameter by running

    MAKE_THREADS_CNT=-j8

### Clone source code and prepare libraries and cmake

Go to ***BuildPath*** and run

    git clone --recursive https://github.com/ton-blockchain/wallet.git

    mkdir Libraries
    cd Libraries

    git clone https://github.com/desktop-app/patches.git

    git clone --branch v3.15.3 https://github.com/Kitware/CMake cmake
    cd cmake
    ./bootstrap
    make $MAKE_THREADS_CNT
    sudo make install
    cd ..

    git clone --branch 0.5.0 https://github.com/ericniebler/range-v3

    git clone https://github.com/madler/zlib.git
    cd zlib
    ./configure
    make $MAKE_THREADS_CNT
    sudo make install
    cd ..

    git clone https://github.com/openssl/openssl
    cd openssl
    git checkout OpenSSL_1_0_1-stable
    ./config
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

    git clone git://code.qt.io/qt/qt5.git qt5_6_2
    cd qt5_6_2
    perl init-repository --module-subset=qtbase,qtimageformats
    git checkout v5.6.2
    cd qtimageformats && git checkout v5.6.2 && cd ..
    cd qtbase && git checkout v5.6.2 && cd ..
    cd qtbase && git apply ..\..\patches\qtbase_5_6_2.diff && cd ..
    cd qtbase/src/plugins/platforminputcontexts
    git clone https://github.com/desktop-app/fcitx.git
    git clone https://github.com/desktop-app/hime.git
    git clone https://github.com/desktop-app/nimf.git
    cd ../../../..

    ./configure -prefix "/usr/local/desktop-app/Qt-5.6.2" -release -force-debug-info -opensource -confirm-license -qt-zlib -qt-libpng -qt-libjpeg -qt-freetype -qt-harfbuzz -qt-pcre -qt-xcb -qt-xkbcommon-x11 -no-opengl -no-gtkstyle -static -openssl-linked -nomake examples -nomake tests

    make $MAKE_THREADS_CNT
    sudo make install
    cd ..

    git clone https://chromium.googlesource.com/external/gyp
    cd gyp
    git checkout 9f2a7bb1
    git apply ../patches/gyp.diff
    cd ..

    git clone https://github.com/ton-blockchain/ton.git
    cd ton
    git checkout ecb3e06a06
    git submodule init
    git submodule update third-party/crc32c
    mkdir build-debug
    cd build-debug
    cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=/usr/local/include -DOPENSSL_CRYPTO_LIBRARY=/usr/local/ssl/lib/libcrypto.a -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=/usr/local/include -DZLIB_LIBRARIES=/usr/local/lib/libz.a -DTON_ARCH=`uname -m | sed --expression='s/_/-/g'` ..
    make $MAKE_THREADS_CNT tonlib
    cd ..
    mkdir build
    cd build
    cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=/usr/local/include -DOPENSSL_CRYPTO_LIBRARY=/usr/local/ssl/lib/libcrypto.a -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=/usr/local/include -DZLIB_LIBRARIES=/usr/local/lib/libz.a -DTON_ARCH=`uname -m | sed --expression='s/_/-/g'` -DCMAKE_BUILD_TYPE=Release ..
    make $MAKE_THREADS_CNT tonlib
    cd ../..

### Building the project

Go to ***BuildPath*/wallet/Wallet** and run

    gyp/refresh.sh

To make Debug version go to ***BuildPath*/wallet/out/Debug** and run

    make $MAKE_THREADS_CNT

To make Release version go to ***BuildPath*/wallet/out/Release** and run

    make $MAKE_THREADS_CNT

You can debug your builds from Qt Creator, just open **CMakeLists.txt** from ***BuildPath*/wallet/out/Debug** and launch with debug.
