#!/bin/sh
if [ -d "CMakeFiles" ];then rm -rf CMakeFiles; fi
if [ -f "Makefile" ];then rm -f Makefile; fi
if [ -f "cmake_install.cmake" ];then rm -f cmake_install.cmake; fi
if [ -f "CMakeCache.txt" ];then rm -f CMakeCache.txt; fi
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../tools/cmake_toolchain_files/armgcc.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=uf2_debug  .
make -j 2>&1 | tee build_log.txt
objcopy -O ihex uf2_debug/flexio_neopixel_int.elf uf2_debug/flexio_neopixel_int.hex
python ~/sd_card/github/uf2/utils/uf2conv.py -f MIMXRT10XX -c -o uf2_debug/flexio_neopixel_int.uf2 uf2_debug/flexio_neopixel_int.hex
