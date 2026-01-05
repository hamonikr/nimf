FROM archlinux:latest AS builder

# 필요한 패키지 설치
# Note: gtk2 removed as it's no longer available in Arch Linux repositories
RUN pacman -Syu --noconfirm \
    binutils \
    autoconf \
    automake \
    gcc \
    make \
    glib2 \
    glib2-devel \
    gtk3 \
    qt5-base \
    qt6-base \
    libappindicator-gtk3 \
    libayatana-appindicator \
    librsvg \
    noto-fonts-cjk \
    anthy \
    librime \
    libxkbcommon \
    wayland \
    wayland-protocols \
    libxklavier \
    m17n-lib \
    m17n-db \
    gtk-doc \
    git \
    base-devel \
    sudo \
    wget \
    intltool \
    libtool \
    pkg-config \
    gettext

# 일반 사용자 생성
RUN useradd -m builduser && echo "builduser ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# 소스 코드 복사 및 작업 디렉토리 설정
COPY . /home/builduser/src
RUN chown -R builduser:builduser /home/builduser/src

# 일반 사용자로 전환
USER builduser
WORKDIR /home/builduser/src

# libhangul 서브모듈 빌드 및 설치
RUN echo "Building libhangul from submodule..." && \
    cd /home/builduser/src/libhangul && \
    if [ ! -f "configure.ac" ] && [ ! -f "configure.in" ]; then \
        echo "libhangul submodule is empty, cloning from GitHub..." && \
        cd /home/builduser/src && \
        rm -rf libhangul && \
        git clone https://github.com/libhangul/libhangul.git libhangul && \
        cd libhangul; \
    fi && \
    echo "Configuring and building libhangul..." && \
    ./autogen.sh && \
    ./configure --prefix=/usr && \
    make -j $(nproc) && \
    sudo make install && \
    sudo ldconfig

# 루트 사용자로 전환 (libhangul 설치 완료 후)
USER root

# 프로젝트 빌드 (패키지 빌드용)
RUN cd /home/builduser/src && \
    echo "Cleaning up previous build artifacts..." && \
    make clean 2>/dev/null || echo "No Makefile found, skipping make clean" && \
    find . -name "*.moc" -delete 2>/dev/null || true && \
    find . -name "moc_*.cpp" -delete 2>/dev/null || true && \
    echo "Checking for intltoolize..." && \
    which intltoolize && \
    echo "Running autogen.sh..." && \
    ./autogen.sh --prefix=/usr --enable-gtk-doc && \
    echo "Building project..." && \
    make -j $(nproc) 

ENTRYPOINT ["bash"]
