gr-custom: contains custom waveform generators e.g. a triangle pulse.

gr-customprocessing: contains an attempt at moving autocorrelation of a waveform.  This isn't quite functional, mostly due to scaling issues.

To make and add a module's contents:

In /gr-modulename$ 

If build is already present:
```
sudo rm -r build
```

Then:
```
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
```
