#!/usr/bin/env bash
set -eo pipefail

# Include CDT_INSTALL_DIR in CMAKE_FRAMEWORK_PATH
export CMAKE_FRAMEWORK_PATH="${CDT_INSTALL_DIR}:${CMAKE_FRAMEWORK_PATH}"

printf "\t=========== Building rem.bonus contract ===========\n\n"
RED='\033[0;31m'
NC='\033[0m'
CPU_CORES=$(getconf _NPROCESSORS_ONLN)
mkdir -p build
pushd build &> /dev/null
cmake -DBUILD_TESTS=${BUILD_TESTS} ../
make -j $CPU_CORES
popd &> /dev/null
