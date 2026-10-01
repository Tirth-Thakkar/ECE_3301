# Lab 3 Traffic Light
- Code is located in src/main.c
- PIC Configuration is in include/Configuration.h

## Demo Video

- [YouTube Video](https://youtu.be/55LVCpZ060Y) containing the demo of the lab.
- Note: one red LED is rather dim but as seen in the video is still on.  

## Build Instructions
- In the Lab 3 Dir run the following 

```bash
mkdir build && cd build 
cmake -DCMAKE_TOOLCHAIN_FILE=../PIC18F46K22.cmake ../
make -j$(nproc)

# Note: Need to symlink ipecmd.sh to ipecmd and have it on PATH
./../prog.sh 
```