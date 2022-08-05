#!/bin/bash

export LLVM_DISABLE_ENV=yes
eval "optbin -s /opt/llvm/bin"
eval "optbin -s /opt/llvm/bin/libexec"
eval "optlib -s /opt/llvm/lib"
eval "optlib -s /opt/llvm/lib/x86_64-unknown-linux-gnu"

