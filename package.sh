#!/bin/bash

PACKAGE_NAME="violoop-capture-rk"
ARCH="aarch64"
VERSION=$(cat version | tr -d '\n' | xargs)

# copy bin file
mkdir -p package/usr/local/bin
cp dist/capture package/usr/local/bin/capture

# set control
mkdir -p package/DEBIAN
sed -i "s/^Package:.*/Package: ${PACKAGE_NAME}/" package/DEBIAN/control
sed -i "s/^Version:.*/Version: ${VERSION}/" package/DEBIAN/control

# build
dpkg-deb --build package

# rename
mv package.deb ${PACKAGE_NAME}_${VERSION}_${ARCH}.deb

exit 0
