# Use the latest Fedora image as the base
FROM fedora:latest AS builder

# Install the required packages
RUN yum install -y \
    cmake \
    autoconf \
    automake \
    libtool \
    pkg-config \
    make \
    gcc \
    gcc-c++ \
    sudo \
    rpm-build \
    redhat-rpm-config \
    glib2-devel \
    gtk3-devel \
    gtk2-devel \
    wayland-devel \
    wayland-protocols-devel \
    libxkbcommon-devel \
    libxklavier-devel \
    libappindicator-gtk3-devel \
    libayatana-appindicator-gtk3-devel.x86_64 \
    librsvg2-tools \
    google-noto-cjk-fonts \
    m17n-lib-devel \
    m17n-db-devel \
    anthy-devel \
    qt5-qtbase-devel \
    qt5-qttools-devel \
    qt5-qtbase-private-devel \
    qt6-qtbase-devel \
    qt6-qttools-devel \
    librime-devel \
    gtk-doc \
    intltool \
    gettext-devel \
    git \
    expat

# Clone, build, and install libhangul
RUN git clone https://github.com/libhangul/libhangul.git \
 && cd libhangul \
 && ./autogen.sh \
 && ./configure --prefix=/usr \
 && make \
 && sudo make install

# Copy the source code and set the working directory
COPY . /src
WORKDIR /src

# Build, compile, and install the project
RUN ./autogen.sh \
 && ./configure --prefix=/usr/local \
 && make -j$(nproc) \
 && make install

# Set the default entrypoint to bash
ENTRYPOINT ["bash"]
