#!/bin/bash
set -euo pipefail

export BR2_DEFCONFIG=/app/config/buildroot-config
export BR2_EXTERNAL=/app/br2-external/linear_function_accelerator

if [ "$1" == "build" ]; then
	echo "Building Linux buildroot"

    # Delete previous custom model
	rm -rf /app/build/buildroot/output/linux/build/my-custom-peripheral* /app/build/buildroot/output/linux/build/linear_function_accelerator*

	# Build the Linux system and copy the kernel and root filesystem to the images directory.
	mkdir -p /app/images/system/binaries
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ defconfig
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ all
	cp /app/build/buildroot/output/linux/images/vmlinux /app/images/system/binaries/
	cp /app/build/buildroot/output/linux/images/rootfs.ext2 /app/images/system/binaries/

	# Build the SDK and copy it to the images directory.
	# This is necessary for building the aarch64 bootcode later, since the bootcode depends on the toolchain provided by the SDK.
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ sdk
	cp /app/build/buildroot/output/linux/images/aarch64-buildroot-linux-gnu_sdk-buildroot.tar.gz /app/images/

elif [ "$1" == "clean" ]; then
	echo "Cleaning Linux buildroot"
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ clean

elif [ "$1" == "menuconfig" ]; then
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ defconfig
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ menuconfig
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ savedefconfig

elif [ "$1" == "linux-menuconfig" ]; then
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ defconfig
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ linux-menuconfig
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ linux-savedefconfig
	cp /app/build/buildroot/output/linux/build/linux-*/defconfig /app/config/linux-config

elif [ "$1" == "build-bootcode" ]; then
	echo "Building Linux bootcode"
	# Delete the existing bootcode
	make -C /app/bootcode/arm64 clean
	make -C /app/bootcode/arm clean

	# Build the aarch64 bootcode and copy it to the images directory.
	mkdir -p /app/images/system/binaries
	make -C /app/bootcode/arm64 all
	cp /app/bootcode/arm64/boot.arm64 /app/images/system/binaries/

	# Build the aarch32 bootcode. This is only necessary because gem5 checks for the presence of aarch32 bootcode, and will fail if it is not found.
	# The aarch32 bootcode is not actually used in our system, since we are only running aarch64 code.
	export AARCH32_DIR=/app/build/aarch32
	export AARCH32_ARCHIVE=arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz
	export AARCH32_TOOLCHAIN_DIR=$AARCH32_DIR/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi
	# Download the toolchain if it does not exist.
	mkdir -p "$AARCH32_DIR"
	if [ ! -f "$AARCH32_DIR/$AARCH32_ARCHIVE" ]; then
		wget -O "$AARCH32_DIR/$AARCH32_ARCHIVE" https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz
	fi
	# Extract the toolchain if it has not been extracted.
	if [ ! -d "$AARCH32_TOOLCHAIN_DIR" ]; then
		tar -xvf "$AARCH32_DIR/$AARCH32_ARCHIVE" -C "$AARCH32_DIR"
	fi
	# Build the aarch32 bootcode and copy it to the images directory.
	make -C /app/bootcode/arm CROSS_COMPILE=$AARCH32_TOOLCHAIN_DIR/bin/arm-none-eabi- all
	cp /app/bootcode/arm/boot.arm /app/images/system/binaries/

	# Compile the device tree and store it in the images directory.
	echo "Compiling the device tree"
	dtc -I dts -O dtb -o /app/images/system/system.dtb /app/config/system.dts

else
	echo "Unsupported argument"
fi
