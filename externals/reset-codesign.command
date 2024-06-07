#!/bin/sh

PATH_DIR_SCRIPT="$(cd "$(dirname "${BASH_SOURCE:-$0}")" && pwd)"
pushd $PATH_DIR_SCRIPT

for mxo in ./*.mxo
do
    xattr -cr "${mxo}"
    codesign --force --deep --sign - "${mxo}"
done

popd