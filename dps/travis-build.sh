#!/bin/bash

set -x

if [ -z "${TRAVIS}" ]; then
    echo "This script is only meant to run on travis-ci" >&2
    exit 1
fi

build_config=Release
cmake_configure_args="-H./dps -B./dps/build"
cmake_build_args="--build ./dps/build"
case "${TRAVIS_OS_NAME}" in
    linux|osx)
        cmake_configure_args="${cmake_configure_args} -DCMAKE_BUILD_TYPE=${build_config}"
    ;;
    windows)
        cmake_build_args="${cmake_build_args} --config ${build_config}"
    ;;
esac

cmake ${cmake_configure_args}
cmake ${cmake_build_args}
cmake ${cmake_build_args} --target test
cmake ${cmake_build_args} --target package

find ./dps/build
