[![Build Status](https://travis-ci.org/lucianodato/speech-denoiser.svg?branch=master)](https://travis-ci.org/lucianodato/speech-denoiser)

speech-denoiser
------
A speech denoise lv2 plugin based on Xiph's RNNoise library

Requirements
-----
You will require git gnu-make gnu-autoconf gnu-m4 gnu-libtool to be installed on your system in order to compile the static instance of RNNoise library

To compile and install this plug-in you will need the LV2 SDK, Meson build system (use pip3 to install it), ninja compiler and git

Installation 
-----
Use whatever --prefix folder your OS needs, for example in MacOS:
```bash
  chmod +x static_rnnoise.sh
  ./static_rnnoise.sh
  meson build --buildtype release --prefix "/Library/Audio/Plug-Ins/LV2" && cd build
  ninja && sudo ninja install
```
In linux prefix should be --prefix "/usr/local/lib/lv2" or something similar depending on your distro filesystem requirments.

Alternative
-----
Make sure to check out Noise Repellent LV2 plugin if this is not working for you -> https://github.com/lucianodato/noise-repellent