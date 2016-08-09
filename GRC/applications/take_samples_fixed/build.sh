rm -r build
mkdir build
cd build
cmake ..
make 2>&1 | tee ../out.txt
cd ..

