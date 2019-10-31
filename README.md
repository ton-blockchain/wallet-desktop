# [Test Gram Wallet][gram_wallet]

This is the complete source code and build instructions for the TON Blockchain testnet desktop wallet.

The source code is published under GPLv3 with OpenSSL exception, the license is available [here][license].

## Supported systems

* Windows 7 and later
* macOS 10.12 and later
* Ubuntu 14.04 and later (64 bit)

## Third-party

* Qt 5.12.5, slightly patched ([LGPL](http://doc.qt.io/qt-5/lgpl.html))
* OpenSSL 1.1.1 ([OpenSSL License](https://www.openssl.org/source/license.html))
* zlib 1.2.8 ([zlib License](http://www.zlib.net/zlib_license.html))
* LZMA SDK 9.20 ([public domain](http://www.7-zip.org/sdk.html))
* liblzma ([public domain](http://tukaani.org/xz/))
* Google Breakpad ([License](https://chromium.googlesource.com/breakpad/breakpad/+/master/LICENSE))
* Google Crashpad ([Apache License 2.0](https://chromium.googlesource.com/crashpad/crashpad/+/master/LICENSE))
* GYP ([BSD License](https://github.com/bnoordhuis/gyp/blob/master/LICENSE))
* Ninja ([Apache License 2.0](https://github.com/ninja-build/ninja/blob/master/COPYING))
* Guideline Support Library ([MIT License](https://github.com/Microsoft/GSL/blob/master/LICENSE))
* Mapbox Variant ([BSD License](https://github.com/mapbox/variant/blob/master/LICENSE))
* Range-v3 ([Boost License](https://github.com/ericniebler/range-v3/blob/master/LICENSE.txt))
* Open Sans font ([Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html))
* xxHash ([BSD License](https://github.com/Cyan4973/xxHash/blob/dev/LICENSE))

## Build instructions

* [Visual Studio 2019][msvc]
* [Xcode 10][xcode]
* [GYP/CMake on GNU/Linux][cmake]

[//]: # (LINKS)
[gram_wallet]: https://wallet.ton.org
[license]: LICENSE
[msvc]: docs/building-msvc.md
[xcode]: docs/building-xcode.md
[cmake]: docs/building-cmake.md

