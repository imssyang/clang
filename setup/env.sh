#!/bin/bash

export LLVM_DISABLE_ENV=yes
export LLVM_INSTALL_DIR=/opt/llvm
export LLVM_CONFIG=/opt/llvm/bin/llvm-config
export LLVM_DIR=/opt/llvm/lib/cmake/llvm
eval "optbin -s /opt/llvm/bin"
eval "optbin -s /opt/llvm/bin/libexec"
eval "optlib -s /opt/llvm/lib"
eval "optlib -s /opt/llvm/lib/x86_64-unknown-linux-gnu"

