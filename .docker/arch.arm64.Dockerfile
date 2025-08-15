FROM archlinux:latest AS builder

# 패키지 업데이트 및 기본 도구 설치
RUN pacman -Sy --noconfirm archlinux-keyring && \
    pacman -Su --noconfirm && \
    pacman -S --noconfirm \
        base-devel \
        cmake \
        git \
        autoconf \
        automake \
        libtool \
        intltool \
        pkg-config \
        gtk3 \
        gtk4 \
        qt5-base \
        qt6-base \
        glib2 \
        wayland \
        libxkbcommon \
        libhangul \
        anthy \
        librime \
        m17n-lib \
        m17n-db \
        libxklavier \
        libayatana-appindicator \
        librsvg \
        gtk-doc \
        sudo

# builduser 생성 (makepkg는 root로 실행할 수 없음)
RUN useradd -m -G wheel builduser && \
    echo "builduser ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

COPY . /home/builduser/src
RUN chown -R builduser:builduser /home/builduser/src

WORKDIR /home/builduser/src

# autoreconf가 필요한 경우를 대비해 빌드 설정
USER builduser
RUN autoreconf --force --install --verbose && \
    ./configure --prefix=/usr && \
    make -j $(nproc)

ENTRYPOINT ["bash"]