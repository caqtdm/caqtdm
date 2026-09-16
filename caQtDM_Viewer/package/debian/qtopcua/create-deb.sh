QTOPCUA_SRC="$GITHUB_WORKSPACE/qtopcua-src"
rm -rf "$QTOPCUA_SRC"

git clone https://code.qt.io/qt/qtopcua.git "$QTOPCUA_SRC"
cd "$QTOPCUA_SRC"
git checkout v5.15.2

# Workaround for conflicting compiler flags created with qmake from qtbase5-dev
sed -i 's/-Wno-format//g' src/3rdparty/open62541.pri

mkdir build
cd build

qmake ..
cat config.summary 

make -j$(nproc)

# Now create a deb package
STAGING_DIR="$GITHUB_WORKSPACE/caQtDM_Viewer/package/debian/qtopcua"
make install INSTALL_ROOT="$STAGING_DIR"

cd $STAGING_DIR

chmod 0755 "DEBIAN/postinst"
dpkg-deb --build "$STAGING_DIR" "$STAGING_DIR/qtopcua_amd64.deb"
