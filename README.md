# Gem5 – AArch64 Linux Image

## Build & Install
1. Clone the repository including submodules:
    ```bash
    git clone --recursive git@github.com:rpelke/gem5_sw.git
    ```

1. The default configuration files for buildroot and Linux can be found in the [config](./config) folder.
If you want to change the configuration(s), use:
    ```bash
    ./scripts/create_buildroot_config.bash
    ./scripts/create_linux_config.bash
    ```

1. Build the Linux system:
    ```bash
    ./scripts/build_image.bash
    ```

1. Build the bootcode and device tree:
    ```bash
    ./scripts/build_bootcode.bash
    ```

## Output
Build artifacts are written to the [images](./images) folder:

```text
images/
├── aarch64-buildroot-linux-gnu_sdk-buildroot.tar.gz
└── system/
    ├── system.dtb
    └── binaries/
        ├── boot.arm
        ├── boot.arm64
        ├── rootfs.ext2
        └── vmlinux
```

- The generated device tree blob: [system.dtb](./images/system/system.dtb)
- The bootcode binaries: [boot.arm64](./images/system/binaries/boot.arm64) and [boot.arm](./images/system/binaries/boot.arm)
- The compiled kernel as `elf` file: [vmlinux](./images/system/binaries/vmlinux)
- The root file system: [rootfs.etx2](./images/system/binaries/rootfs.ext2)
- The aarch64 toolchain: [aarch64-buildroot-linux-gnu_sdk-buildroot.tar.gz](./images/aarch64-buildroot-linux-gnu_sdk-buildroot.tar.gz)
