依次在终端执行以下命令
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j16
./RaceCar
即可运行该工程