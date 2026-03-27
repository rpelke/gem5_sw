FROM ubuntu:24.04
WORKDIR /app
RUN apt-get update && apt-get -y install \
	build-essential \
	libncurses-dev \
	file \
	wget \
	cpio \
	python3 \
	unzip \
	rsync \
	bc \
	git \
	libssl-dev \
	device-tree-compiler

# Baremetal toolchain for bootcode
RUN wget "https://developer.arm.com/-/media/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf.tar.xz" && tar -xf gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf.tar.xz && rm gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf.tar.xz
ENV PATH="/app/gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf/bin:${PATH}"

ENTRYPOINT ["/app/docker_entrypoint.bash"]
