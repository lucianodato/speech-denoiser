#!/usr/bin/env bash

#default installation directories
INSTALL_DIR_LINUX="/usr/local/lib/lv2"
INSTALL_DIR_MAC="/Library/Audio/Plug-Ins/LV2"

# Detect the platform (similar to $OSTYPE)
OS="`uname`"
case $OS in
  'Linux') OS='Linux';;
  'Darwin') OS='Mac';;
  *) ;;
esac

#remove previous build
rm -rf build rnnoise || true

#build rrnoise statically
git clone https://github.com/xiph/rnnoise.git
./rnnoise/autogen.sh

if [ $OS = "Mac" ]; then
    CFLAGS="-fvisibility=hidden -fPIC " \ 
    ./configure/rnnoise/ --disable-examples --disable-doc --disable-shared --enable-static        
elif [ $OS = "Linux" ]; then
    CFLAGS="-fvisibility=hidden -fPIC -Wl,--exclude-libs,ALL" \
    ./configure/rnnoise/ --disable-examples --disable-doc --disable-shared --enable-static
fi

#build the plugin in the new directory
if [ $OS = "Linux" ]; then
    echo "You are on a Linux system. Building for Linux"
    meson build --buildtype release --prefix $INSTALL_DIR_LINUX
elif [ $OS = "Mac" ]; then
    echo "You are on a Mac system. Building for MacOS"
    meson build --buildtype release --prefix $INSTALL_DIR_MAC
fi

cd build
ninja -v

#install the plugin in the system
sudo ninja install