#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "Usage: $0 --version <Qt version>"
}

if [ "$#" -ne 2 ] || [ "$1" != "--version" ]; then
    usage >&2
    exit 2
fi

QTOPCUA_VERSION="$2"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$(mktemp -d)"
STAGING_DIR="$SCRIPT_DIR/staging"

cleanup() {
    rm -rf "$BUILD_DIR" "$STAGING_DIR"
}
trap cleanup EXIT

curl --fail --location --retry 3 \
    "https://github.com/qt/qtopcua/archive/refs/tags/v${QTOPCUA_VERSION}.tar.gz" \
    --output "$BUILD_DIR/qtopcua.tar.gz"
tar -C "$BUILD_DIR" -xzf "$BUILD_DIR/qtopcua.tar.gz"

cmake -B "$BUILD_DIR/build" -S "$BUILD_DIR/qtopcua-${QTOPCUA_VERSION}" -G Ninja \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DQT_FEATURE_open62541=ON \
    -DQT_BUILD_EXAMPLES=OFF
cmake --build "$BUILD_DIR/build" --parallel "$(nproc)"

mkdir -p "$STAGING_DIR"
cp -a "$SCRIPT_DIR/DEBIAN" "$STAGING_DIR/DEBIAN"
DESTDIR="$STAGING_DIR" cmake --install "$BUILD_DIR/build" --prefix /usr
sed -i "s/^Version: .*/Version: ${QTOPCUA_VERSION}/" "$STAGING_DIR/DEBIAN/control"
sed -i "s/^Architecture: .*/Architecture: $(dpkg --print-architecture)/" "$STAGING_DIR/DEBIAN/control"
chmod 0755 "$STAGING_DIR/DEBIAN/postinst"
dpkg-deb --build "$STAGING_DIR" "$SCRIPT_DIR/qtopcua_qt6_$(dpkg --print-architecture).deb"
