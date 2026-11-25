FROM alpine:latest AS builder

RUN apk update
RUN apk upgrade

# Install build tools and dependencies
RUN apk add --no-cache \
    build-base \
    cmake \
    git \
    qt6-qtbase \
    qt6-qtbase-dev \
    qt6-qtbase-private-dev \
    qt6-qttools \
    qt6-qttools-dev \
    qt6-qt5compat \
    qt6-qt5compat-dev \
    qt6-qtwebsockets \
    qt6-qtwebsockets-dev \
    qt6-qtsvg \
    qt6-qtsvg-dev \
    qt6-qtpositioning \
    qt6-qtpositioning-dev \
    qt6-qtlocation \
    qt6-qtlocation-dev \
    qt6-qtserialbus \
    qt6-qtserialbus-dev \
    qt6-qtimageformats \
    qt6-qtimageformats-dev \
    zeromq \
    zeromq-dev \
    python3 \
    python3-dev \
    ncurses \
    ncurses-dev \
    perl \
    readline \
    readline-dev \
    qt6-qwt \
    qwt-dev \
    zlib-dev

WORKDIR /build
RUN git clone --branch R7.0.11 --depth 1 --recursive https://github.com/epics-base/epics-base.git
WORKDIR /build/epics-base
RUN make INSTALL_LOCATION=/opt/epics -j$(nproc)

WORKDIR /build
COPY . /build/caQtDM
WORKDIR /build/caQtDM

RUN mkdir -p /opt/caqtdm/lib/qt6 && \
    export EPICS_BASE=/opt/epics && \
    export EPICS_HOST_ARCH=$(perl /build/epics-base/src/tools/EpicsHostArch.pl) && \
    export EPICSINCLUDE=/opt/epics/include && \
    export EPICSLIB=/opt/epics/lib/${EPICS_HOST_ARCH} && \
    export QTHOME=/usr && \
    export QWTHOME=/usr && \
    export QWTLIB=/usr/lib && \
    export QWTINCLUDE=/usr/include/qwt && \
    export QWTLIBNAME=qwt-qt6 && \
    export QWTVERSION=6.1 && \
    export PYTHONVERSION=$(python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')") && \
    export PYTHONINCLUDE=/usr/include/python${PYTHONVERSION} && \
    export PYTHONLIB=/usr/lib && \
    export ZMQLIB=/usr/lib && \
    export ZMQINC=/usr/include && \
    export CAQTDM_MODBUS=1 && \
    export CAQTDM_GPS=1 && \
    export CAQTDM_WEB=1 && \
    export CAQTDM_COLLECT=/opt/caqtdm/lib/qt6 && \
    export QTDM_RPATH=/opt/caqtdm/lib/qt6 && \
    export QTCONTROLS_LIBS=/opt/caqtdm/lib/qt6 && \
    export QTBASE=/opt/caqtdm/lib/qt6 && \
    qmake6 all.pro && \
    make -j$(nproc)

WORKDIR /build
RUN git clone --depth 1 https://github.com/caqtdm/qnovnc-platform-plugin.git

WORKDIR /build/qnovnc-platform-plugin
RUN cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr && \
    cmake --build build --parallel $(nproc) --target install


FROM alpine:latest

RUN apk --no-cache update
RUN apk --no-cache upgrade

# Install runtime dependencies
RUN apk add --no-cache \
    qt6-qtbase \
    qt6-qttools \
    qt6-qttools-libs \
    qt6-qtwebsockets \
    qt6-qtsvg \
    qt6-qtpositioning \
    qt6-qtlocation \
    qt6-qtserialbus \
    qt6-qtimageformats \
    qt6-qwt \
    zeromq \
    python3 \
    ncurses \
    readline \
    perl \
    zlib \
    ttf-dejavu \
    fontconfig \
    mesa-gl 

COPY caQtDM_Tests/ /opt/caqtdm/caqtdm_tests/

COPY --from=builder /opt/caqtdm /opt/caqtdm
COPY --from=builder /opt/epics /opt/epics
COPY --from=builder /usr/lib/qt6/plugins/platforms/libqnovnc.so /usr/lib/qt6/plugins/platforms/libqnovnc.so

RUN mkdir -p /app/caqtdm_display_path

RUN mkdir -p /usr/share/fonts/lucida
COPY caQtDM_Viewer/lucida-sans-typewriter.ttf /usr/share/fonts/lucida/lucida-sans-typewriter.ttf
RUN fc-cache -f -v

ENV CAQTDM_DISPLAY_PATH=/app/caqtdm_display_path:/opt/caqtdm/caqtdm_tests
ENV EPICS_CA_ADDR_LIST=
ENV EPICS_CA_MAX_ARRAY_BYTES=40000000

ENV ENTRY_PANEL=tests.ui
ENV EXTRA_ARGS=

WORKDIR /app

CMD ["sh", "-c", "/opt/caqtdm/lib/qt6/caQtDM -server -novnc -host 0.0.0.0 $EXTRA_ARGS $ENTRY_PANEL"]
