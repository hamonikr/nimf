# Use Fedora 40 image as the base
FROM fedora:40 AS builder

# Update and install the required packages
RUN dnf update -y && \
    dnf install -y --setopt=install_weak_deps=False \
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
    libayatana-appindicator-gtk3-devel \
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
    qt6-qtbase-private-devel \    
    librime-devel \
    gtk-doc \
    intltool \
    gettext-devel \
    git \
    expat.x86_64 \
    expat-devel.x86_64 \
    im-chooser \
    libhangul-devel && \
    dnf clean all 

# Copy the source code and set the working directory
COPY . /src
WORKDIR /src

# Clean any existing build artifacts and MOC files
RUN find . -name "*.moc" -delete && \
    find . -name "*.lo" -delete && \
    find . -name "*.la" -delete && \
    make clean 2>/dev/null || true

# Build the project (without install for package building)
RUN ./autogen.sh && \
    ./configure --prefix=/usr && \
    make -j$(nproc)

# Set the default entrypoint to bash
ENTRYPOINT ["bash"]