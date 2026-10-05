mkdir build && cd build 
cmake -DCMAKE_TOOLCHAIN_FILE=../PIC18F46K22.cmake ../
make -j$(nproc)