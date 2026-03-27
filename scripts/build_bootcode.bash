#!/bin/env bash
set -euo pipefail

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
	DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
	SOURCE="$(readlink "$SOURCE")"
	[[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
DOCKER_DIR="$DIR/../docker"

source "$DIR/../common/setup.bash"

# Build Linux image using buildroot
$CONTAINER_PROGRAM build --tag gem5_linux_buildroot "$DOCKER_DIR"
"$DOCKER_DIR/docker_run_linux_buildroot.bash" build-bootcode
