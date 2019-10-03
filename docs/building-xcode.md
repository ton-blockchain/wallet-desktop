## Build instructions for Xcode 10.1

### Prepare folder

Choose a folder for the future build, for example **/Users/user/Projects**. It will be named ***BuildPath*** in the rest of this document. All commands will be launched from Terminal.

### Clone source code and prepare libraries

Go to ***BuildPath*** and run

    MAKE_THREADS_CNT=-j4
    MACOSX_DEPLOYMENT_TARGET=10.8

    ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
    brew install automake cmake fdk-aac git lame libass libtool libvorbis libvpx ninja opus sdl shtool texi2html theora wget x264 xvid yasm pkg-config

    sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

    git clone --recursive https://github.com/ton-blockchain/wallet.git

    cd Libraries

    git clone --branch 0.5.0 https://github.com/ericniebler/range-v3

    cd xz-5.0.5
    CFLAGS="-mmacosx-version-min=10.8" LDFLAGS="-mmacosx-version-min=10.8" ./configure
    make
    sudo make install
    cd ..

    git clone https://github.com/madler/zlib.git
    cd zlib
    CFLAGS="-mmacosx-version-min=10.8" LDFLAGS="-mmacosx-version-min=10.8" ./configure
    make $MAKE_THREADS_CNT
    sudo make install
    cd ..

    git clone https://github.com/openssl/openssl
    cd openssl
    git checkout OpenSSL_1_0_1-stable
    ./Configure darwin64-x86_64-cc -static -mmacosx-version-min=10.8
    make build_libs $MAKE_THREADS_CNT
    cd ..

    git clone https://chromium.googlesource.com/external/gyp
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    export PATH="$PWD/depot_tools:$PATH"
    cd gyp
    git checkout 9f2a7bb1
    git apply ../patches/gyp.diff
    ./setup.py build
    sudo ./setup.py install
    cd ..

    git clone git://code.qt.io/qt/qt5.git qt5_6_2
    cd qt5_6_2
    perl init-repository --module-subset=qtbase,qtimageformats
    git checkout v5.6.2
    cd qtimageformats && git checkout v5.6.2 && cd ..
    cd qtbase && git checkout v5.6.2 && git apply ../../patches/qtbase_5_6_2.diff && cd ..

    ./configure -prefix "/usr/local/desktop-app/Qt-5.6.2" -debug-and-release -force-debug-info -opensource -confirm-license -static -opengl desktop -no-openssl -securetransport -nomake examples -nomake tests -platform macx-clang
    make $MAKE_THREADS_CNT
    sudo make install
    cd ..

    LibrariesPath=`pwd`

    git clone https://github.com/ton-blockchain/ton.git
    cd ton
    git checkout ecb3e06a06
    git submodule init
    git submodule update third-party/crc32c
    mkdir build-debug
    cd build-debug
    cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ARCH= -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=$LibrariesPath/openssl/include -DOPENSSL_CRYPTO_LIBRARY=$LibrariesPath/openssl/libcrypto.a -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=$LibrariesPath/zlib -DZLIB_LIBRARIES=/usr/local/lib/libz.a -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.8 -DCMAKE_CXX_FLAGS="-stdlib=libc++" ..
    make $MAKE_THREADS_CNT tonlib
    cd ..
    mkdir build
    cd build
    cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ARCH= -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=$LibrariesPath/openssl/include -DOPENSSL_CRYPTO_LIBRARY=$LibrariesPath/openssl/libcrypto.a -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=$LibrariesPath/zlib -DZLIB_LIBRARIES=/usr/local/lib/libz.a -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.8 -DCMAKE_CXX_FLAGS="-stdlib=libc++" -DCMAKE_BUILD_TYPE=Release ..
    make $MAKE_THREADS_CNT tonlib
    cd ../..

### Building the project

Go to ***BuildPath*/wallet/Wallet** and run

    gyp/refresh.sh

Then launch Xcode, open ***BuildPath*/wallet/Wallet/Wallet.xcodeproj** and build for Debug / Release.
