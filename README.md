speech-denoiser
------
A speech denoise lv2 plugin based on Xiph's RNNoise library

IT DOES NOT WORK YET!!!

You will require git gnu-make gnu-autoconf gnu-m4 gnu-libtool to be installed on your system in order to compile the static instance of RNNoise library

For the plugin itself you'll need LV2 SDK, gnu-make, a c-compiler, git and pkg-config

Commands to compile everything needed
```bash
  chmod +x static_rnnoise.sh
  ./static_rnnoise.sh
  make
  sudo make install
```
