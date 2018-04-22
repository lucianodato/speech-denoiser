speech-denoiser
------
A speech denoise lv2 plugin based on Xiph's RNNoise library

You will require git gnu-make gnu-autoconf gnu-m4 gnu-libtool to be installed on your system in order to compile the static instance of RNNoise library

For the plugin itself you'll need LV2 SDK, gnu-make, a c-compiler, git and pkg-config

Installation (Use whatever --prefix folder your OS needs) for example in MacOS:
```bash
  chmod +x static_rnnoise.sh
  ./static_rnnoise.sh
  meson build --buildtype release --prefix "/Library/Audio/Plug-Ins/LV2" && cd build
  ninja && sudo ninja install
```
In linux prefix should be --prefix "/usr/local/lib/lv2" or something similar depending on your distro filesystem requirments.