The files within this folder can be opened and modified with the GNURadio Companion software.

Autocorrelation.grc - A first attempt at writing a custom module for on-the-fly autocorrelation.  Incomplete and not presently being developed.

FM_receiver.grc - A utility program for converting .dat files at a FM radio frequency to .wav files or audio output.  This is used primarily as verification that a sample-taking system is functional.

wav_player.grc - A utility program for playing .wav files as audio output; made because the LiveUSB GNURadio environment didn't have a ready way to play .wav files.

udp_receiver.grc - A utility program for catching UDP packets sent by the USRP; I initially had some thoughts about using this to speed up sample-taking, but eventually decided on writing a new C++ application instead of using GRC or UDP.

