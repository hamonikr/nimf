FROM fedora:latest AS builder

# 필요한 패키지 설치
RUN yum install -y \
    sudo \
    rpm-build \
    redhat-rpm-config \
    gnome-shell-extension-appindicator \
    libappindicator-gtk3-devel \
    wayland-devel \
    wayland-protocols-devel \
    libxkbcommon-devel \
    libayatana-appindicator-gtk3-devel.x86_64 \
    autoconf \
    automake \
    libtool \
    intltool \
    gettext-devel \
    glib2-devel \
    libhangul-devel \
    m17n-lib-devel \
    anthy-devel \
    qt5-qtbase-devel \
    qt5-qttools-devel \
    qt6-qtbase-devel \
    qt6-qttools-devel \
    gtk3-devel \
    gtk2-devel \
    librsvg2-tools

# 소스 코드 복사 및 작업 디렉토리 설정
COPY . /src
WORKDIR /src

# Build the entire project
RUN ./autogen.sh \
 && ./configure --prefix=/usr/local \
 && make -j$(nproc) \
 && make install

ENTRYPOINT ["bash"]
