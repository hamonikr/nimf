FROM debian:trixie AS builder

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get -qq upgrade

RUN apt-get install --no-install-recommends -y \
        cmake \
        g++ \
        make \
        pkg-config \
        python3-dev \
        devscripts \
        equivs \
        sudo \
        xterm \
        apt-utils \
        git \
        autotools-dev \
        autoconf \
        libtool \
        intltool \
        libglib2.0-dev \
        libgtk-3-dev \
        libqt5core5a \
        libqt5gui5 \
        qtbase5-dev \
        qtbase5-dev-tools \
        qt6-base-dev \
        qt6-base-dev-tools \
        libwayland-dev \
        libxkbcommon-dev \
        libhangul-dev \
        libhangul1 \
        libhangul-data

COPY . /src
WORKDIR /build

# 자동으로 yes 응답을 전달
RUN yes | mk-build-deps --install --root-cmd sudo --remove /src/debian/control

# Set correct architecture for ARM64 package building
ENV DEB_BUILD_ARCH=arm64
ENV DEB_HOST_ARCH=arm64

# Build the entire project
RUN cd /src \
 && autoreconf --force --install --verbose \
 && ./configure --prefix=/usr/local \
 && make -j $(nproc) \
 && make install

ENTRYPOINT ["bash"]
