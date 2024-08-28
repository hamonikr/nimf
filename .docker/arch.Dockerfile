FROM archlinux:latest AS builder

# 필요한 패키지 설치
RUN pacman -Syu --noconfirm \
    binutils \
    autoconf \
    automake \
    gcc \
    make \
    glib2 \
    glib2-devel \
    gtk3 \
    gtk2 \
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
    wget

# 일반 사용자 생성
RUN useradd -m builduser && echo "builduser ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# 소스 코드 복사 및 작업 디렉토리 설정
COPY . /home/builduser/src
RUN chown -R builduser:builduser /home/builduser/src

# 일반 사용자로 전환
USER builduser
WORKDIR /home/builduser/src

# AUR 도우미 yay 설치
RUN git clone https://aur.archlinux.org/yay.git /home/builduser/yay \
 && cd /home/builduser/yay \
 && makepkg -si --noconfirm

# AUR에서 libhangul-git 패키지 설치
RUN yay -S --noconfirm libhangul-git

# 패키지 빌드 및 설치
# RUN makepkg -si --noconfirm

# 루트 사용자로 전환
USER root
RUN rm -rf /home/builduser/yay

# 프로젝트 빌드
RUN cd /home/builduser/src \
 && ./autogen.sh --prefix=/usr --enable-gtk-doc \
 && make -j $(nproc)

# 패키지 설치
RUN cd /home/builduser/src \
 && make DESTDIR="/pkgdir/" install

# 설치 후 설정 안내
RUN echo 'To use Nimf as your input method framework, add the following lines to your ~/.xprofile:' \
 && echo 'export GTK_IM_MODULE=nimf' \
 && echo 'export QT4_IM_MODULE="nimf"' \
 && echo 'export QT_IM_MODULE=nimf' \
 && echo 'export QT6_IM_MODULE=nimf' \
 && echo 'export XMODIFIERS="@im=nimf"' 

ENTRYPOINT ["bash"]
