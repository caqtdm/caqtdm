#!/usr/bin/env bash
set -euo pipefail

run_with_heartbeat() {
  local label="$1"
  shift
  "$@" &
  local command_pid=$!
  (
    while kill -0 "${command_pid}" 2>/dev/null; do
      sleep 60
      if kill -0 "${command_pid}" 2>/dev/null; then
        echo "$(date -u '+%Y-%m-%dT%H:%M:%SZ') Still running: ${label} (pid ${command_pid})"
      fi
    done
  ) &
  local heartbeat_pid=$!
  local result=0
  wait "${command_pid}" || result=$?
  kill "${heartbeat_pid}" 2>/dev/null || true
  wait "${heartbeat_pid}" 2>/dev/null || true
  return "${result}"
}

mkdir -p "${QT_ROOT}" "${RUNNER_TEMP}/qt-src"
qt_tarball="qt-everywhere-src-${QT_VERSION}.tar.xz"
qt_minor="$(echo "${QT_VERSION}" | cut -d. -f1,2)"
qt_url="https://download.qt.io/official_releases/qt/${qt_minor}/${QT_VERSION}/single/${qt_tarball}"
qt_archive="${RUNNER_TEMP}/${qt_tarball}"
echo "::group::Download Qt ${QT_VERSION} source"
echo "Source URL: ${qt_url}"
echo "Saving to: ${qt_archive}"
curl --fail --location --show-error --retry 3 --progress-bar \
  --write-out $'\nHTTP %{http_code}; downloaded %{size_download} bytes in %{time_total}s\n' \
  "${qt_url}" -o "${qt_archive}"
test -s "${qt_archive}"
echo "Archive size: $(stat -f%z "${qt_archive}") bytes"
shasum -a 256 "${qt_archive}"
echo "::endgroup::"

echo "::group::Extract Qt ${QT_VERSION} source"
echo "Extracting ${qt_archive} into ${RUNNER_TEMP}/qt-src"
tar -xf "${qt_archive}" -C "${RUNNER_TEMP}/qt-src"
qt_source="${RUNNER_TEMP}/qt-src/qt-everywhere-src-${QT_VERSION}"
test -d "${qt_source}"
echo "Qt source directory ready: ${qt_source}"
echo "::endgroup::"

qt_host_submodules='qtbase,qt5compat,qtsvg,qttools,qtimageformats,qtpositioning,qtopcua'
qt_ios_submodules='qtbase,qt5compat,qtsvg,qttools,qtimageformats,qtpositioning,qtopcua,qtserialbus'
mkdir -p "${RUNNER_TEMP}/qt-build-host"
cd "${RUNNER_TEMP}/qt-build-host"
echo "::group::Configure Qt ${QT_VERSION} host tools"
echo "Install prefix: ${QT_HOST_ROOT}"
echo "Qt modules: ${qt_host_submodules}"
"${qt_source}/configure" \
  -prefix "${QT_HOST_ROOT}" \
  -submodules "${qt_host_submodules}" \
  -release -opensource -confirm-license \
  -nomake tests -nomake examples
echo "::endgroup::"
echo "::group::Build and install Qt host tools"
run_with_heartbeat "Build Qt host tools" cmake --build . --parallel
run_with_heartbeat "Install Qt host tools" cmake --install .
echo "Verifying host OPC UA code generator and CMake package"
test -x "${QT_HOST_ROOT}/bin/qopcuaxmldatatypes2cpp"
test -f "${QT_HOST_ROOT}/lib/cmake/Qt6OpcUaTools/Qt6OpcUaToolsConfig.cmake"
echo "Host Qt installation ready at ${QT_HOST_ROOT}"
echo "::endgroup::"

for target in iphoneos iphonesimulator; do
  if [ "${target}" = iphoneos ]; then
    qt_prefix="${QT_IOS_DEVICE_ROOT}"
    openssl_prefix="${OPENSSL_IOS_ROOT}/${OPENSSL_VERSION}/iphoneos"
  else
    qt_prefix="${QT_IOS_SIMULATOR_ROOT}"
    openssl_prefix="${OPENSSL_IOS_ROOT}/${OPENSSL_VERSION}/iphonesimulator-arm64"
  fi
  build_dir="${RUNNER_TEMP}/qt-build-${target}"
  mkdir -p "${build_dir}"
  cd "${build_dir}"
  echo "::group::Configure Qt ${QT_VERSION} for ${target}"
  echo "SDK version: $(xcrun --sdk "${target}" --show-sdk-version)"
  echo "Install prefix: ${qt_prefix}"
  echo "OpenSSL prefix: ${openssl_prefix}"
  echo "Qt modules: ${qt_ios_submodules}"
  "${qt_source}/configure" \
    -prefix "${qt_prefix}" \
    -platform macx-ios-clang -sdk "${target}" \
    -qt-host-path "${QT_HOST_ROOT}" \
    -submodules "${qt_ios_submodules}" \
    -openssl-linked -feature-open62541-security \
    -release -opensource -confirm-license \
    -nomake tests -nomake examples \
    -- \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="${IOS_DEPLOYMENT_TARGET}" \
    -DOPENSSL_ROOT_DIR="${openssl_prefix}" \
    -DOPENSSL_USE_STATIC_LIBS=ON \
    -DFEATURE_open62541_security=ON
  summary_file="${build_dir}/config.summary"
  echo "Checking Qt's configure summary for Open62541 security support"
  if [ ! -f "${summary_file}" ]; then
    echo "Qt configure did not create ${summary_file}" >&2
    exit 1
  fi
  if ! grep -Eq 'Open62541 security support[.[:space:]]+yes' "${summary_file}"; then
    cat "${summary_file}"
    echo "Open62541 security support was not enabled for ${target}; stopping before the Qt build" >&2
    exit 1
  fi
  grep -E 'Open62541 security support' "${summary_file}"
  echo "::endgroup::"
  echo "::group::Build and install Qt ${QT_VERSION} for ${target}"
  run_with_heartbeat "Build Qt for ${target}" cmake --build . --parallel
  run_with_heartbeat "Install Qt for ${target}" cmake --install .
  echo "Qt ${target} installation ready at ${qt_prefix}"
  echo "::endgroup::"
done

test -x "${QT_HOST_ROOT}/bin/qmake"
for qt_prefix in "${QT_IOS_DEVICE_ROOT}" "${QT_IOS_SIMULATOR_ROOT}"; do
  test -x "${qt_prefix}/bin/qmake"
  for module in QtCore QtCore5Compat QtUiTools QtSvg QtPositioning QtOpcUa QtSerialBus; do
    test -d "${qt_prefix}/lib/${module}.framework"
  done
  test -f "${qt_prefix}/plugins/opcua/libopen62541_backend.a"
  test -f "${qt_prefix}/mkspecs/modules/qt_plugin_open62541_backend.pri"
done
