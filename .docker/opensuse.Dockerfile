# Use the openSUSE Leap 15.6 as the base
FROM opensuse/leap:15.6 AS builder

# Change fastest repo and set priority to 50
# RUN zypper addrepo --priority 50 -f https://ftp.kaist.ac.kr/opensuse/update/leap/15.6/oss/ Kaist_Main_Update_Repo \
#  && zypper addrepo --priority 50 -f https://ftp.kaist.ac.kr/opensuse/distribution/leap/15.6/repo/oss/ Kaist_Main_OSS_Repo \
#  && zypper addrepo --priority 50 -f https://ftp.kaist.ac.kr/opensuse/distribution/leap/15.6/repo/non-oss/ Kaist_Main_NON-OSS_Repo \
#  && zypper addrepo --priority 50 -f https://ftp.kaist.ac.kr/opensuse/update/leap/15.6/backports/ Kaist_Update_Backports_Repo \
#  && zypper addrepo --priority 50 -f https://ftp.kaist.ac.kr/opensuse/update/leap/15.6/sle/ Kaist_Update_SUSE_Enterprise_Linux_Repo

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
 git

# Clone, build, and install libhangul
RUN git clone https://github.com/libhangul/libhangul.git \
 && cd libhangul \
 && ./autogen.sh \
 && ./configure --prefix=/usr \
 && make \
 && make install

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
