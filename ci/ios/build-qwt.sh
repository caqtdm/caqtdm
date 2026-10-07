#!/usr/bin/env bash
set -euo pipefail

sdk="$1"
qwt_prefix="$2"
qt_target_root="$3"

git clone --depth 1 --single-branch --branch "${QWT_VERSION}" \
  https://git.code.sf.net/p/qwt/git "${RUNNER_TEMP}/qwt-src"
sed -i '' -E "s|^[[:space:]]*QWT_INSTALL_PREFIX[[:space:]]*=.*|QWT_INSTALL_PREFIX = ${qwt_prefix}|" \
  "${RUNNER_TEMP}/qwt-src/qwtconfig.pri"
sed -i '' -E 's@^QWT_CONFIG[[:space:]]*\+=[[:space:]]*Qwt(Dll|Designer|Examples|Playground|Tests|Framework|PkgConfig).*@# disabled for mobile: &@' \
  "${RUNNER_TEMP}/qwt-src/qwtconfig.pri"
grep -q '^QWT_CONFIG[[:space:]]*+=[[:space:]]*QwtStatic' "${RUNNER_TEMP}/qwt-src/qwtconfig.pri" || \
  echo 'QWT_CONFIG += QwtStatic' >> "${RUNNER_TEMP}/qwt-src/qwtconfig.pri"

mkdir -p "${RUNNER_TEMP}/qwt-build"
cd "${RUNNER_TEMP}/qwt-build"
SDKROOT="$(xcrun --sdk "${sdk}" --show-sdk-path)" \
  "${qt_target_root}/bin/qmake" "${RUNNER_TEMP}/qwt-src/qwt.pro" \
  "CONFIG+=${sdk}" \
  "QMAKE_IOS_DEPLOYMENT_TARGET=${IOS_DEPLOYMENT_TARGET}" \
  QMAKE_APPLE_DEVICE_ARCHS=arm64 QMAKE_APPLE_SIMULATOR_ARCHS=arm64
SDKROOT="$(xcrun --sdk "${sdk}" --show-sdk-path)" \
  CODE_SIGNING_ALLOWED=NO make -j"$(sysctl -n hw.ncpu)"
make install
test -f "${qwt_prefix}/lib/libqwt.a"
