#!/bin/sh
PATH_DIR_SCRIPT="$(cd "$(dirname "${BASH_SOURCE:-$0}")" && pwd)"
pushd $PATH_DIR_SCRIPT
xattr -cr .
for mxo in externals/*.mxo
do
    xattr -cr "${mxo}"
    codesign --force --deep --sign - "${mxo}"
done
popd

