#!/bin/bash -e
rm -f base-7.0.11.* || true
rm -rf epics-base-7.0.11 || true
rm -f epics-base_7.0.11* || true
rm -f epics-base-dbgsym* || true
git clone https://github.com/epics-base/epics-base.git --branch R7.0.11 --depth 1 base-7.0.11
cd base-7.0.11
git submodule update --init --recursive --jobs $(nproc)
cd ..
tar -czf base-7.0.11.tar.gz base-7.0.11
mv base-7.0.11 epics-base-7.0.11
cd epics-base-7.0.11
dh_make --createorig -s -y
cp -rf ../debian/* debian/
rm -rf debian/*.ex
rm -f debian/README.Debian
rm -f debian/README.source
pwd
dpkg-buildpackage -us -uc
