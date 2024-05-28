#!/bin/sh

for mxo in externals/*.mxo
do
    xattr -cr "${mxo}"
    codesign --force --deep --sign - "${mxo}"
done

