#!/usr/bin/env bash

#default installation directories
INSTALL_DIR_LINUX="/usr/local/lib/lv2"
INSTALL_DIR_MAC="/Library/Audio/Plug-Ins/LV2"

# Detect the platform (similar to $OSTYPE)
OS="`uname`"
case $OS in
  'Linux') OS='Linux' && echo "You are on a Linux system. Building for Linux";;
  'Darwin') OS='Mac' && echo "You are on a Mac system. Building for MacOS";;
  *) ;;
esac

#remove previous builds
rm -rf build || true

#build rrnoise statically
wget https://github.com/xiph/rnnoise/archive/master.zip
unzip -o master.zip && rm master.zip
cd rnnoise-master && ./autogen.sh
mv ../ltmain.sh ./ && ./autogen.sh #This is weird but otherwise it won't work

if [ $OS = "Mac" ]; then
    CFLAGS="-fvisibility=hidden -fPIC " \ 
    ./configure --disable-examples --disable-doc --disable-shared --enable-static        
elif [ $OS = "Linux" ]; then
    CFLAGS="-fvisibility=hidden -fPIC -Wl,--exclude-libs,ALL" \
    ./configure --disable-examples --disable-doc --disable-shared --enable-static
fi

make -j2
cd ..

#build the plugin in the new directory
if [ $OS = "Linux" ]; then
    meson build --buildtype release --prefix $INSTALL_DIR_LINUX
elif [ $OS = "Mac" ]; then
    meson build --buildtype release --prefix $INSTALL_DIR_MAC
fi

cd build
ninja -v

#install the plugin in the system
sudo ninja install

#Remove static rnnoise build
cd ..
rm -rfv rnnoise-master
