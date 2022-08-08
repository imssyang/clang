#!/bin/bash

APP=llvm
HOME=/opt/$APP
VER=v1406

_create_symlink() {
  src=$1
  dst=$2
  if [[ ! -d $dst ]] && [[ ! -s $dst ]]; then
    ln -s $src $dst
    echo "($APP) create symlink: $src -> $dst"
  fi
}

_delete_symlink() {
  dst=$1
  if [[ -d $dst ]] || [[ -s $dst ]]; then
    rm -rf $dst
    echo "($APP) delete symlink: $dst"
  fi
}

_archive_item() {
  fpath=$1
  fdir=$(dirname $fpath)
  fname=$(basename $fpath)
  if [[ ! -f $HOME/archive/$fname-$VER.7z ]]; then
    echo "($APP) archive $fpath"
    7z a -mx9 -mmt4 $HOME/archive/$fname-$VER.7z $fpath
  fi
}

_unarchive_item() {
  fpath=$1
  fdir=$(dirname $fpath)
  fname=$(basename $fpath)
  if [[ ! -f $fpath ]]; then
    echo "($APP) unarchive $HOME/archive/$fname-$VER.7z -> $fpath"
    7z x $HOME/archive/$fname-$VER.7z -o$fdir
  fi
}

_large_files=(
  $HOME/bin/bugpoint
  $HOME/bin/clang-14
  $HOME/bin/clang-check
  $HOME/bin/clang-linker-wrapper
  $HOME/bin/clang-repl
  $HOME/bin/clang-scan-deps
  $HOME/bin/clang-tidy
  $HOME/bin/dsymutil
  $HOME/bin/llc
  $HOME/bin/llvm-c-test
  $HOME/bin/llvm-dwp
  $HOME/bin/llvm-lto
  $HOME/bin/llvm-lto2
  $HOME/bin/llvm-reduce
  $HOME/bin/opt
  $HOME/lib/libclang-cpp.so.14
  $HOME/lib/libclang.so.14.0.6
  $HOME/lib/libLLVM-14.so
  $HOME/lib/liblldb.so.14.0.6
  $HOME/lib/libLTO.so.14
)

init() {
  chown -R root:root $HOME
  chmod 755 $HOME
}

deinit() {
  pass
}

archive() {
  for fpath in "${_large_files[@]}"; do
    _archive_item $fpath
  done
}

unarchive() {
  for fpath in "${_large_files[@]}"; do
    _unarchive_item $fpath
  done
}

show() {
  for fpath in "${_large_files[@]}"; do
    echo $fpath
  done
}

case "$1" in
  init) init ;;
  deinit) deinit ;;
  archive) archive ;;
  unarchive) unarchive ;;
  show) show ;;
  *) SCRIPTNAME="${0##*/}"
     echo "Usage: $SCRIPTNAME {init|deinit|archive|unarchive|show}"
     exit 3
     ;;
esac

exit 0

# vim: syntax=sh ts=4 sw=4 sts=4 sr noet
