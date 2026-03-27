#!/bin/bash
set -euo pipefail

export BR2_DEFCONFIG=/app/config/buildroot-config

if [ "$1" == "build" ]; then
	echo "Building Linux buildroot"
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ defconfig
	make O=/app/build/buildroot/output/linux -C /app/buildroot/ all
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
else
	echo "Unsupported argument"
fi
