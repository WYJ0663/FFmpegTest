
#!/bin/sh
dos2unix *
NDK="/home/qiqi/Desktop/android-ndk-r15c"
PLATFORM=$NDK/platforms/android-18/arch-arm/
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
ARM_INC=$PLATFORM/usr/include/
ARM_LIB=$PLATFORM/usr/lib/
PREFIX=$(pwd)/android
function build_one {
    ./configure --prefix=$PREFIX --enable-pic --disable-opencl --enable-static --enable-shared --disable-lavf --disable-asm --host=arm-linux --cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- --sysroot=$PLATFORM --extra-cflags="-fPIC -marm -DX264_VERSION -DANDROID -DHAVE_PTHREAD -DNDEBUG -static -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__ -O3 -march=armv7-a -mfpu=neon -mtune=generic-armv7-a -mfloat-abi=softfp -ftree-vectorize -mvectorize-with-neon-quad -ffast-math"
make clean
make -j4
make install
}
build_one
