#!/bin/bash

export LLVM_DISABLE_ENV=yes
export LLVM_INSTALL_DIR=/opt/clang
export LLVM_CONFIG=/opt/clang/bin/llvm-config
export LLVM_DIR=/opt/clang/lib/cmake/llvm
eval "optbin -s /opt/clang/bin"
eval "optbin -s /opt/clang/bin/libexec"
eval "optlib -s /opt/clang/lib"
eval "optlib -s /opt/clang/lib/x86_64-unknown-linux-gnu"

