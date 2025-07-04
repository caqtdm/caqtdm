#!/bin/bash -e
rm -f base-7.0.9.* || true
rm -rf epics-base-7.0.9 || true
rm -f epics-base_7.0.9* || true
rm -f epics-base-dbgsym* || true
wget https://epics-controls.org/download/base/base-7.0.9.tar.gz
tar xf base-7.0.9.tar.gz
mv base-7.0.9 epics-base-7.0.9
cd epics-base-7.0.9
dh_make --createorig -s -y
cp -rf ../debian/* debian/
rm -rf debian/*.ex
rm -f debian/README.Debian
rm -f debian/README.source
pwd
dpkg-buildpackage -us -uc
