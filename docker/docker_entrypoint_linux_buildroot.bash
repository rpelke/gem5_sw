#!/bin/bash
set -euo pipefail

export BR2_DEFCONFIG=/app/config/buildroot-config

if [ "$1" == "build" ]; then
	echo "Building Linux buildroot"
	mkdir -p /app/images/system/binaries
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ defconfig
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ all
	cp /app/build/buildroot/output/linux/images/vmlinux /app/images/system/binaries/
	cp /app/build/buildroot/output/linux/images/rootfs.ext2 /app/images/system/binaries/
	dtc -I dts -O dtb -o /app/images/system/system.dtb /app/config/system.dts
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
	mkdir -p /app/images/system/binaries
	make -C /app/bootcode/arm64 all
	cp /app/bootcode/arm64/boot.arm64 /app/images/system/binaries/
else
	echo "Unsupported argument"
fi
