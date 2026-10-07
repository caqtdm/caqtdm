#!/usr/bin/env bash
set -euo pipefail

target="$1"
case "${target}" in
  iphoneos)
    configure_target=ios64-xcrun
    minimum_version_flag="-miphoneos-version-min=${IOS_DEPLOYMENT_TARGET}"
    ;;
  iphonesimulator-arm64)
    configure_target=iossimulator-arm64-xcrun
    minimum_version_flag="-mios-simulator-version-min=${IOS_DEPLOYMENT_TARGET}"
    ;;
  *)
    echo "Unsupported OpenSSL target: ${target}" >&2
    exit 1
    ;;
esac

openssl_prefix="${OPENSSL_IOS_ROOT}/${OPENSSL_VERSION}/${target}"
openssl_archive="${RUNNER_TEMP}/openssl-${OPENSSL_TAG_SHA}.tar.gz"
openssl_source="${RUNNER_TEMP}/openssl-${target}"

echo "Downloading OpenSSL ${OPENSSL_VERSION} (${OPENSSL_TAG_SHA}) for ${target}"
if [ ! -s "${openssl_archive}" ]; then
  curl --fail --location --show-error --retry 3 --progress-bar \
    --write-out $'\nHTTP %{http_code}; downloaded %{size_download} bytes in %{time_total}s\n' \
    "https://github.com/openssl/openssl/archive/${OPENSSL_TAG_SHA}.tar.gz" \
    -o "${openssl_archive}"
fi
test -s "${openssl_archive}"
mkdir -p "${openssl_source}"
tar -xzf "${openssl_archive}" -C "${openssl_source}" --strip-components=1
cd "${openssl_source}"

./Configure "${configure_target}" no-shared no-tests \
  "${minimum_version_flag}" \
  --prefix="${openssl_prefix}" --openssldir="${openssl_prefix}/ssl"
make -j"$(sysctl -n hw.ncpu)"
make install_sw
