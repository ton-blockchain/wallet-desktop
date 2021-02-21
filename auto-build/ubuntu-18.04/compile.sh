apt install python2.7 python2.7-minimal libpython2.7-minimal libpython2.7-stdlib -y
update-alternatives --install /usr/bin/python python /usr/bin/python2.7 10

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
add-apt-repository --remove ppa:ubuntu-toolchain-r/test -y

MAKE_THREADS_CNT=-j8

git clone --recursive https://github.com/newton-blockchain/wallet-desktop.git

mkdir Libraries
cd Libraries

git clone https://github.com/Kitware/CMake cmake
cd cmake
git checkout v3.16.0
./bootstrap
make $MAKE_THREADS_CNT
make install
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
make install
cd ..

git clone https://github.com/xkbcommon/libxkbcommon.git
cd libxkbcommon
git checkout xkbcommon-0.8.4
./autogen.sh --disable-x11
make $MAKE_THREADS_CNT
make install

cd ..

git clone https://github.com/google/googletest
cd googletest
mkdir build
cd build
cmake ..
make
cp lib/*.a /usr/lib

cd ../..

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
make install
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

cp -R ../googletest/googletest src/testing/
cp -R ../googletest/googlemock src/testing/

cp src/tools/mac/symupload/minidump_upload.m src/tools/linux/symupload/

make $MAKE_THREADS_CNT
make install
cd src/tools
../../../gyp/gyp  --depth=. --generator-output=.. -Goutput_dir=../out tools.gyp --format=cmake
cd ../../out/Default
cmake .
make $MAKE_THREADS_CNT dump_syms
cd ../../..

git clone https://github.com/newton-blockchain/ton.git
cd ton
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

cd ../../..

cd wallet-desktop

#temporary dirty workaround
sed -i '238d' cmake/external/qt/CMakeLists.txt
sed -i '238d' cmake/external/qt/CMakeLists.txt
sed -i '238d' cmake/external/qt/CMakeLists.txt
sed -i '242d' cmake/external/qt/CMakeLists.txt

cd Wallet

./configure.sh -D DESKTOP_APP_USE_PACKAGED=OFF

cd ../out/Release

make $MAKE_THREADS_CNT

cd bin
tar -czvf Wallet.tar.gz Wallet
