# Use the openSUSE Leap 15.6 as the base
FROM --platform=linux/arm64 opensuse/leap:15.6 AS builder

# Install the required packages
RUN zypper install -y \
 rpm-build \
 rpmdevtools \
 anthy \
 anthy-devel \
 libexpat-devel \
 gcc-c++ \
 glib2-devel \
 gtk-doc \
 gtk2-devel \
 gtk3-devel \
 intltool \
 libhangul-devel \
 libtool \
 libxkbcommon-devel \
 libxklavier-devel \
 wayland-devel \
 wayland-protocols-devel \
 google-noto-sans-cjk-fonts \
 libappindicator3-devel \
 libayatana-appindicator3-devel \
 librsvg-devel \
 libqt5-qtbase-devel \
 libqt5-qtbase-private-headers-devel \
 qt6-base-devel \
 qt6-base-private-devel \
 rsvg-convert \
 librime-devel \
 m17n-lib-devel \
 m17n-db \
 git

# Copy the source code and set the working directory
COPY . /src
WORKDIR /src

# Clean any existing build artifacts and MOC files
RUN find . -name "*.moc" -delete && \
    find . -name "*.lo" -delete && \
    find . -name "*.la" -delete && \
    make clean 2>/dev/null || true

# Build, compile, and install the project
RUN ./autogen.sh \
 && ./configure --prefix=/usr/local \
 && make -j$(nproc) \
 && make install

# Set the default entrypoint to bash
ENTRYPOINT ["bash"]