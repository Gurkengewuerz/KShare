#!/usr/bin/env bash
VERSION=$(grep setApplicationVersion ../src/main.cpp | head -n1 | cut -d \" -f2)
echo "Make Debian package for v$VERSION" >&2

mkdir work/ || true
cp deb/* work -r
sed "s/%ver/$VERSION/g" deb/DEBIAN/control > work/DEBIAN/control
mkdir -p work/usr/bin

if [[ "$1" == "" ]]
then
    echo "Fresh Compile Binary" >&2
    mkdir compiling
    cd compiling
    qmake ../../KShare.pro
    if make
    then
    cd ..
    cp compiling/src/kshare work/usr/bin/kshare
    else
        rm -rf compiling
        echo "Failed to make!"
        exit 2
    fi
else
    echo "Using pre-compiled binary (please only use in circleci)" >&2
    pwd

    cp ../build/src/kshare work/usr/bin/kshare
fi

cd work
echo "md5sum" >&2
md5sum usr/bin/kshare usr/share/applications/KShare.desktop > DEBIAN/md5sums
cd ..
echo "dpkg-deb" >&2
dpkg-deb -b work/
mv work.deb kshare_v${VERSION}.deb
rm -rf work
