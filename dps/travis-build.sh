#!/bin/bash

set -x
set -e

pushd $(dirname $0)
source_dir=$(pwd)
build_dir=${source_dir}/build

if [ -z "${TRAVIS}" ]; then
    echo "This script is only meant to run on travis-ci" >&2
    exit 1
fi

build_config=Release
cmake_configure_args="${source_dir}"
cmake_build_args="--build ."
case "${TRAVIS_OS_NAME}" in
    linux|osx)
        cmake_configure_args="${cmake_configure_args} -DCMAKE_BUILD_TYPE=${build_config}"
    ;;
    windows)
        cmake_build_args="${cmake_build_args} --config ${build_config}"
    ;;
esac

mkdir -p ${build_dir}
pushd ${build_dir}
cmake ${cmake_configure_args}
cmake ${cmake_build_args}
ctest -C ${build_config} --output-on-failure
cmake ${cmake_build_args} --target package
popd
popd
