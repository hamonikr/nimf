FROM fedora:latest AS builder

# 필요한 패키지 설치
RUN yum install -y \
    cmake \
    autoconf \
    automake \
    libtool \
    pkg-config \
    make \
    gcc \
    gcc-c++ \
    glib2-devel \
    pkgconfig \
    intltool \
    gtk3-devel \
    gtk2-devel \
    qt5-qtbase-devel \
    qt5-qtbase-private-devel \
    qt6-qtbase-devel \
    qt6-qtbase-private-devel \
    libappindicator-gtk3-devel \
    librsvg2-tools \
    google-noto-cjk-fonts \
    libhangul-devel \
    anthy-devel \
    anthy \
    libxkbcommon-devel \
    wayland-devel \
    libxklavier-devel \
    gtk-doc \
    librime-devel \
    m17n-lib-devel \
    m17n-db-devel \
    sudo \
    rpm-build \
    redhat-rpm-config

# 소스 코드 복사 및 작업 디렉토리 설정
COPY . /src
WORKDIR /src

# Build the entire project
RUN cd /src \
 && ./autogen.sh \
 && ./configure --prefix=/usr/local \
 && make -j $(nproc) \
 && make install
 
ENTRYPOINT ["bash"]
