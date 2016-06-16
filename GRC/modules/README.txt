To make and add a module's contents:

In /gr-modulename$ 

If build is already present:
sudo rm -r build

Then:
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
