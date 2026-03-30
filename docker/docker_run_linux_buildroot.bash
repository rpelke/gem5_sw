#!/bin/bash
set -euo pipefail

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
	DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
	SOURCE="$(readlink "$SOURCE")"
	[[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

BUILDROOT_DIR="$DIR/../buildroot"
BOOTCODE_DIR="$DIR/../bootcode"
IMAGES_DIR="$DIR/../images"
CONFIG_DIR="$DIR/../config"
BUILD_DIR="$DIR/../BUILD"
OVERLAY_DIR="$DIR/../overlay"

mkdir -p "${BUILD_DIR}"
mkdir -p "${IMAGES_DIR}"

source "$DIR/../common/setup.bash"

$CONTAINER_PROGRAM run \
    --rm \
	-it \
    $CONTAINER_PROGRAM_FLAGS \
	-v "$BUILDROOT_DIR":/app/buildroot:Z \
	-v "$BOOTCODE_DIR":/app/bootcode:Z \
	-v "$IMAGES_DIR":/app/images:Z \
	-v "$CONFIG_DIR":/app/config:Z \
	-v "$BUILD_DIR":/app/build:Z \
	-v "$OVERLAY_DIR":/app/overlay:ro,Z \
	-v "$DIR/docker_entrypoint_linux_buildroot.bash":/app/docker_entrypoint.bash:ro,Z \
	gem5_linux_buildroot "$1"
