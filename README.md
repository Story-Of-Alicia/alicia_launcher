# Alicia launcher
Simple Alicia 1.0 and 2.0 launcher built for Windows.
![image](https://github.com/rgnter/alicia_launcher/assets/32541639/28aba830-fe81-41f6-a948-2cfdd03867ad)

## Building on Linux using mingw-w64-gcc
```bash
$ cmake -DCMAKE_BUILD_TYPE=Debug --toolchain toolchains/linux-mingw.cmake . -Bbuild
$ cd build/
$ cmake --build .
```