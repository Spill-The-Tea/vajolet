pushd .. & mkdir build & pushd build & cmake -DVAJOLET_CPU_TYPE=64BMI2 -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" .. & mingw32-make & popd & popd

