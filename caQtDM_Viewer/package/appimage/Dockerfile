# Build environment for caQtDM AppImage.
#
# The baseline is Rocky Linux 8 / glibc 2.28 so the resulting AppImage runs on
# RHEL, Rocky, and AlmaLinux 8 or newer. In addition to RHEL based distros it
# will also almost certainly run on any distro with glibc 2.28 or newer.
# Qt is built from source and installed into /opt/qt so the AppImage
# does not depend on distribution Qt packages.
#
# The OpenSSL, Qt, Qwt, and EPICS builds are Docker layers. They are cached until
# this Dockerfile, the versions below, or earlier layers change.

FROM rockylinux:8

ARG GCC_TOOLSET=15
ARG OPENSSL_VERSION=3.5.9
ARG QT_VERSION=6.11.1
ARG QT_BUILD_JOBS=auto

ENV OPENSSL_PREFIX=/opt/openssl-${OPENSSL_VERSION}
ENV QT_PREFIX=/opt/qt-${QT_VERSION}
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV PATH=/opt/rh/gcc-toolset-${GCC_TOOLSET}/root/usr/bin:${QT_PREFIX}/bin:${PATH}
ENV LD_LIBRARY_PATH=/opt/rh/gcc-toolset-${GCC_TOOLSET}/root/usr/lib64:${OPENSSL_PREFIX}/lib64:${QT_PREFIX}/lib
ENV PKG_CONFIG_PATH=${OPENSSL_PREFIX}/lib64/pkgconfig:${QT_PREFIX}/lib/pkgconfig:/usr/lib64/pkgconfig:/usr/share/pkgconfig
ENV CMAKE_PREFIX_PATH=${OPENSSL_PREFIX}:${QT_PREFIX}
ENV OPENSSL_ROOT_DIR=${OPENSSL_PREFIX}

# Base repositories and build tools. Rocky Linux 8 keeps the glibc baseline at
# 2.28 while gcc-toolset provides a newer compiler for the bundled Qt build.
RUN dnf -y install dnf-plugins-core epel-release \
    && (dnf config-manager --set-enabled powertools || dnf config-manager --set-enabled PowerTools || true) \
    && dnf -y --setopt=install_weak_deps=False install \
      appstream \
      bison \
      bzip2 \
      ca-certificates \
      binutils \
      cmake \
      file \
      findutils \
      flex \
      fuse \
      fuse-libs \
      gcc-toolset-${GCC_TOOLSET}-gcc \
      gcc-toolset-${GCC_TOOLSET}-gcc-c++ \
      gcc-toolset-${GCC_TOOLSET}-libstdc++-devel \
      git \
      gperf \
      make \
      ninja-build \
      perl \
      pkgconf-pkg-config \
      python3 \
      python3-devel \
      python3-pip \
      readline-devel \
      squashfs-tools \
      tar \
      wget \
      which \
      xz \
      zlib-devel \
    && python3 -m pip install --no-cache-dir --upgrade pip \
    && python3 -m pip install --no-cache-dir jinja2 lxml six \
    && dnf clean all

# Development headers required by QtBase platform plugins, QtSvg, QtPositioning,
# QtSerialBus, QtOpcUa, and the caQtDM build.
RUN dnf -y --setopt=install_weak_deps=False install \
      cups-devel \
      dbus-devel \
      fontconfig-devel \
      freetype-devel \
      glib2-devel \
      harfbuzz-devel \
      libICE-devel \
      libSM-devel \
      libX11-devel \
      libXcursor-devel \
      libXext-devel \
      libXfixes-devel \
      libXi-devel \
      libXrender-devel \
      libicu-devel \
      libjpeg-turbo-devel \
      libpng-devel \
      libuuid-devel \
      libxcb-devel \
      libxkbcommon-devel \
      libxkbcommon-x11-devel \
      mesa-libEGL-devel \
      mesa-libGL-devel \
      mesa-libGLU-devel \
      sqlite-devel \
      systemd-devel \
      wayland-devel \
      wayland-protocols-devel \
      xcb-util-cursor-devel \
      xcb-util-devel \
      xcb-util-image-devel \
      xcb-util-keysyms-devel \
      xcb-util-renderutil-devel \
      xcb-util-wm-devel \
      xkeyboard-config \
      zeromq-devel \
    && dnf clean all

# OpenSSL 3.x for Qt Network TLS and Qt OPC UA/open62541 security support. Build
# it side-by-side with the system OpenSSL so we do not alter the base image ABI.
RUN set -eux; \
    cd /tmp; \
    wget -q "https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/openssl-${OPENSSL_VERSION}.tar.gz"; \
    tar -xzf "openssl-${OPENSSL_VERSION}.tar.gz"; \
    cd "openssl-${OPENSSL_VERSION}"; \
    ./config shared zlib \
      --prefix="${OPENSSL_PREFIX}" \
      --openssldir="${OPENSSL_PREFIX}/ssl"; \
    make -j"$(nproc)"; \
    make install_sw; \
    echo "${OPENSSL_PREFIX}/lib64" > /etc/ld.so.conf.d/caqtdm-openssl.conf; \
    ldconfig; \
    rm -rf "/tmp/openssl-${OPENSSL_VERSION}" "/tmp/openssl-${OPENSSL_VERSION}.tar.gz"

# Qt from source. The selected modules cover caQtDM's Qt requirements:
# Widgets, Designer/UiTools, SVG, SerialPort, SerialBus, Positioning, OPC UA,
# QtWayland, Qt5Compat/QTextCodec, OpenGL, PrintSupport, XCB platform and image
# plugins.
RUN set -eux; \
    cd /tmp; \
    wget -q "https://download.qt.io/official_releases/qt/${QT_VERSION%.*}/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.tar.xz"; \
    tar -xf "qt-everywhere-src-${QT_VERSION}.tar.xz"; \
    mkdir /tmp/qt-build; \
    cd /tmp/qt-build; \
    "/tmp/qt-everywhere-src-${QT_VERSION}/configure" \
      -prefix "${QT_PREFIX}" \
      -opensource -confirm-license \
      -release -shared \
      -no-pch \
      -nomake examples -nomake tests \
      -submodules qtbase,qttools,qtsvg,qtserialport,qtserialbus,qtpositioning,qtopcua,qtwayland,qt5compat,qtimageformats \
      -openssl-linked \
      -feature-open62541-security \
      -feature-gds \
      -- \
      -DOPENSSL_ROOT_DIR="${OPENSSL_PREFIX}" \
      -DOPENSSL_USE_STATIC_LIBS=OFF \
      -DCMAKE_PREFIX_PATH="${OPENSSL_PREFIX}" \
      -DINPUT_open62541=qt \
      -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON \
      -DFEATURE_open62541_security=ON \
      -DFEATURE_gds=ON \
      -DQT_BUILD_EXAMPLES_BY_DEFAULT=OFF \
      -DQT_BUILD_TESTS_BY_DEFAULT=OFF \
      -DQT_BUILD_TOOLS_BY_DEFAULT=ON \
      -DQT_INSTALL_CONFIG_INFO_FILES=ON; \
    if [ "${QT_BUILD_JOBS}" = auto ]; then QT_BUILD_JOBS="$(nproc)"; fi; \
    cmake --build . --parallel "${QT_BUILD_JOBS}"; \
    cmake --install .; \
    rm -rf "/tmp/qt-everywhere-src-${QT_VERSION}" "/tmp/qt-everywhere-src-${QT_VERSION}.tar.xz" /tmp/qt-build

# Validate the installed Qt layer separately so failed assertions do not force a
# full Qt rebuild.
RUN set -eux; \
    test "$(${QT_PREFIX}/bin/qmake6 -query QT_VERSION)" = "${QT_VERSION}"; \
    test -x "${QT_PREFIX}/bin/designer"; \
    test -f "${QT_PREFIX}/lib/libQt6OpcUa.so.6"; \
    test -f "${QT_PREFIX}/lib/libQt6WaylandClient.so.6"; \
    test -f "${QT_PREFIX}/plugins/opcua/libopen62541_backend.so"; \
    compgen -G "${QT_PREFIX}/plugins/platforms/libqwayland*.so" >/dev/null; \
    test -d "${QT_PREFIX}/plugins/wayland-shell-integration"; \
    find "${QT_PREFIX}/plugins/platforms" -maxdepth 1 -name 'libqwayland*.so' -print; \
    find "${QT_PREFIX}/plugins/wayland-shell-integration" -maxdepth 1 -type f -print; \
    test -f "${QT_PREFIX}/include/QtOpcUa/QOpcUaX509CertificateSigningRequest"; \
    strings "${QT_PREFIX}/plugins/opcua/libopen62541_backend.so" | grep -q 'SecurityPolicy#Basic256Sha256'; \
    ldd -r "${QT_PREFIX}/lib/libQt6OpcUa.so.6"; \
    ldd -r "${QT_PREFIX}/lib/libQt6WaylandClient.so.6"; \
    ldd -r "${QT_PREFIX}/plugins/opcua/libopen62541_backend.so"; \
    for plugin in "${QT_PREFIX}"/plugins/platforms/libqwayland*.so; do ldd -r "$plugin"; done; \
    ln -sf "${QT_PREFIX}/bin/qmake6" /usr/local/bin/qmake6; \
    ln -sf "${QT_PREFIX}/bin/qmake6" /usr/local/bin/qmake; \
    ln -sf "${QT_PREFIX}/bin/qmake6" /usr/bin/qmake6; \
    ln -sf "${QT_PREFIX}/bin/qmake6" /usr/bin/qmake; \
    echo "${QT_PREFIX}/lib" > /etc/ld.so.conf.d/caqtdm-qt.conf; \
    ldconfig

# Qwt 6 built against the cached Qt build.
ARG QWT_VERSION=6.3.0
ARG QWT_REPOSITORY=https://git.code.sf.net/p/qwt/git
ARG QWT_REF=v6.3.0
RUN set -eux; \
    cd /tmp; \
    git clone --depth 1 --single-branch --branch "${QWT_REF}" "${QWT_REPOSITORY}" /tmp/qwt; \
    cd /tmp/qwt; \
    sed -i 's|QWT_INSTALL_PREFIX[[:space:]]*=.*|QWT_INSTALL_PREFIX = /usr|' qwtconfig.pri; \
    sed -i 's|QWT_INSTALL_LIBS[[:space:]]*=.*|QWT_INSTALL_LIBS = /usr/lib64|' qwtconfig.pri; \
    sed -i 's|QwtPlayground||g; s|QwtExamples||g; s|QwtDesigner||g' qwtconfig.pri; \
    qmake6 qwt.pro; \
    make -j"$(nproc)"; \
    make install; \
    ldconfig; \
    rm -rf /tmp/qwt

ENV QMAKE_BIN=${QT_PREFIX}/bin/qmake6
ENV QTHOME=${QT_PREFIX}
ENV QWTINCLUDE=/usr/include/qwt
ENV QWTLIB=/usr/lib64
ENV QWTLIBNAME=qwt
ENV QWTVERSION=6.3
ENV EPICS_BASE=/usr/lib/epics

# EPICS Base 7.0.10.
ARG EPICS_VERSION_TAG=R7.0.10
RUN set -eux; \
    mkdir -p "$(dirname "${EPICS_BASE}")"; \
    git clone --depth 1 --branch "${EPICS_VERSION_TAG}" https://github.com/epics-base/epics-base.git "${EPICS_BASE}"; \
    cd "${EPICS_BASE}"; \
    git submodule update --init --recursive --jobs "$(nproc)"; \
    epics_host_arch="$(perl -CSD src/tools/EpicsHostArch.pl)"; \
    printf 'GNU_DIR = /opt/rh/gcc-toolset-%s/root/usr\nAR = /usr/bin/ar -rc\nLD = /usr/bin/ld -r\nRANLIB = /usr/bin/ranlib\n' "${GCC_TOOLSET}" >> "configure/os/CONFIG_SITE.${epics_host_arch}.${epics_host_arch}"; \
    make -j"$(nproc)" XSUBPP= EPICS_HOST_ARCH="${epics_host_arch}"; \
    echo "${EPICS_BASE}/lib/${epics_host_arch}" > /etc/ld.so.conf.d/caqtdm-epics.conf; \
    ldconfig

WORKDIR /src

ENTRYPOINT ["/src/caQtDM_Viewer/package/appimage/create-appimage.sh"]
