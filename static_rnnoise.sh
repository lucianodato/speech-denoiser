#!/usr/bin/env bash

# No need for these with submodule
# rm -rf rnnoise || true
# git clone https://github.com/xiph/rnnoise.git
cd rnnoise/
./autogen.sh
mv ../ltmain.sh ./ && ./autogen.sh # Dang, what with this thing

if [ "$(uname)" == "Darwin" ]; then
  CFLAGS="-fvisibility=hidden -fPIC " \ 
	./configure \
  --disable-examples --disable-doc \
	--disable-shared --enable-static        
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  CFLAGS="-fvisibility=hidden -fPIC -Wl,--exclude-libs,ALL" \
	./configure \
  --disable-examples --disable-doc \
  --disable-shared --enable-static
# elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
#     CFLAGS="-fvisibility=hidden -fPIC -Wl,--exclude-libs,ALL" \
# 	./configure \
#   --disable-examples --disable-doc \
#   --disable-shared --enable-static
# elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
#     CFLAGS="-fvisibility=hidden -fPIC -Wl,--exclude-libs,ALL" \
# 	./configure \
#   --disable-examples --disable-doc \
#   --disable-shared --enable-static
fi

# Got to remove the -j, as Arch's makepkg uses the user configured one, and that
# should not be overidden.
make
