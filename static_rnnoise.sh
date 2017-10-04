#!/bin/sh
if [ -d "rnnoise" ]; then
  rm -rf ./rnnoise
fi
git clone https://github.com/xiph/rnnoise.git
cd rnnoise/
./autogen.sh
CFLAGS="-fvisibility=hidden -fPIC -Wl,--exclude-libs,ALL" \
	./configure \
  # --disable-examples --disable-doc \
	--enable-single --enable-sse --enable-avx --disable-mpi \
	--disable-shared --enable-static
make -j2
