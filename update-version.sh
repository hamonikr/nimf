#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <version>"
    exit 1
fi

VERSION=$1

# Update configure.ac
sed -i "s/AC_INIT(nimf, .*)/AC_INIT(nimf, $VERSION)/" configure.ac

# Update debian/changelog
sed -i "1s/.*/nimf ($VERSION) unstable; urgency=medium/" debian/changelog

# Update nimf.spec
sed -i "s/^%define version .*$/%define version $VERSION/" nimf.spec

# Update PKGBUILD
sed -i "s/^pkgver=.*$/pkgver=$VERSION/" PKGBUILD

# Create git tag
# git tag -a "v$VERSION" -m "Release version $VERSION"
