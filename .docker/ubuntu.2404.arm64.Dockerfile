FROM ubuntu:24.04 AS builder

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
        debhelper \
        build-essential \
        libglib2.0-dev \
        libgtk-3-dev \
        libgtk-4-dev \
        libgtk2.0-dev \
        libqt5core5a \
        libqt5gui5 \
        qtbase5-dev \
        qtbase5-private-dev \
        qt6-base-dev \
        qt6-base-private-dev \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        libwayland-dev \
        libxkbcommon-dev \
        libhangul-dev \
        libhangul1 \
        libhangul-data \
        libanthy-dev \
        anthy \
        librime-dev \
        libm17n-dev \
        m17n-db \
        libxklavier-dev \
        libayatana-appindicator3-dev \
        librsvg2-bin \
        fonts-noto-cjk \
        gtk-doc-tools \
        lsb-release

COPY . /src
WORKDIR /src

# 자동으로 yes 응답을 전달
RUN yes | mk-build-deps --install --root-cmd sudo --remove /src/debian/control

# Build the entire project
RUN autoreconf --force --install --verbose \
 && ./configure --prefix=/usr \
 && make -j $(nproc)

ENTRYPOINT ["bash"]