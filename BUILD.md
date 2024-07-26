# Building Nimf Packages

## Debian Package
To build a Debian package for Nimf, follow these steps:

### Install Dependencies:

```
sudo apt-get update
sudo apt-get install build-essential devscripts debhelper autoconf automake libglib2.0-dev intltool gtk-doc-tools libgtk-3-dev libgtk2.0-dev libqt5core5a libqt5gui5 libqt5widgets5 qtbase5-dev libayatana-appindicator3-dev librsvg2-bin libhangul-dev anthy libxkbcommon-dev libxklavier-dev libm17n-dev m17n-db librime-dev

```

### Clone the Repository:

```
git clone https://github.com/hamonikr/nimf.git
cd nimf
```

Build the Package:
```
debuild -us -uc
```
Install the Package:
```
sudo dpkg -i ../nimf_*.deb
```

## RPM Package
To build an RPM package for Nimf, follow these steps:

### Install Dependencies:

```
# fedora
sudo dnf install anthy-devel gcc-c++ glib2-devel gtk-doc gtk2-devel gtk3-devel intltool \
    libappindicator-gtk3-devel libhangul-devel librime-devel librsvg2-tools libtool \
    libxkbcommon-devel libxklavier-devel m17n-db-devel m17n-lib-devel \
    qt5-qtbase-devel qt5-qtbase-private-devel qt6-qtbase-devel qt6-qtbase-private-devel \
    wayland-devel naver-nanum-fonts-all wayland-protocols-devel expat expat-devel \
    libayatana-appindicator-gtk3-devel.x86_64 im-chooser rpm-build rpmdevtools

# opensuse-leap
sudo zypper install -y rpm-build rpmdevtools anthy anthy-devel libexpat-devel \
    gcc-c++ glib2-devel gtk-doc gtk2-devel gtk3-devel intltool libhangul-devel 
    libtool libxkbcommon-devel libxklavier-devel wayland-devel wayland-protocols-devel \
    google-noto-sans-cjk-fonts libappindicator3-devel libayatana-appindicator3-devel librsvg-devel \
    libqt5-qtbase-devel libqt5-qtbase-private-headers-devel \
    qt6-base-devel qt6-base-private-devel rsvg-convert librime-devel m17n-lib-devel rpm-build rpmdevtools

```

### Create RPM and Install:
```
rpmdev-setuptree

cd ~/

wget https://github.com/hamonikr/nimf/releases/download/v1.3.8/nimf-1.3.8-1.fc40.src.rpm

rpm -ivh nimf-1.3.8-1.fc40.src.rpm

rpmbuild -ba rpmbuild/SPECS/nimf.spec

sudo rpm -ivh rpmbuild/RPMS/x86_64/nimf-*.rpm
```

## Arch Linux Package
To build an Arch Linux package for Nimf, follow these steps:

### Install Dependencies:

```
sudo pacman -S --needed base-devel git glib2 gtk3 gtk2 qt5-base qt6-base libappindicator-gtk3 librsvg noto-fonts-cjk libhangul anthy librime libxkbcommon wayland libxklavier m17n-lib m17n-db gtk-doc
```

### Clone the Repository:

```
# Install latest libhangul-git
git clone https://aur.archlinux.org/libhangul-git.git

cd libhangul-git

makepkg -si 

# Install nimf
git clone https://github.com/hamonikr/nimf.git

cd nimf

makepkg -si 
```
By following these steps, you can build and install Nimf on Debian, RPM-based, and Arch Linux systems.

## opensuse
```
sudo zypper install -y rpm-build rpmdevtools anthy anthy-devel libexpat-devel \
    gcc-c++ glib2-devel gtk-doc gtk2-devel gtk3-devel intltool libhangul-devel 
    libtool libxkbcommon-devel libxklavier-devel wayland-devel wayland-protocols-devel \
    google-noto-sans-cjk-fonts libappindicator3-devel libayatana-appindicator3-devel librsvg-devel \
    libqt5-qtbase-devel libqt5-qtbase-private-headers-devel \
    qt6-base-devel qt6-base-private-devel rsvg-convert librime-devel m17n-lib-devel

rpmdev-setuptree

git clone --recurse-submodules https://github.com/hamonikr/nimf

cd nimf

./build_packages.sh rpm

sudo rpm -ivh  ./build/RPMS/x86_64/nimf-*.rpm

# if require GPG-KEY
sudo rpm --import RPM-GPG-KEY-nimf

echo -e "\nexport GTK_IM_MODULE=nimf\nexport QT4_IM_MODULE=xim\nexport QT_IM_MODULE=nimf\nexport XMODIFIERS=@im=nimf" >> ~/.bashrc \
 && echo -e "\nexport GTK_IM_MODULE=nimf\nexport QT4_IM_MODULE=xim\nexport QT_IM_MODULE=nimf\nexport XMODIFIERS=@im=nimf" >> ~/.zshrc

```
### if need sign for rpm package
```
echo "%_signature gpg" >>  ~/.rpmmacros
echo "%_gpg_name HamoniKR Team <pkg@hamonikr.org>" >>  ~/.rpmmacros
rpm --addsign ./build/RPMS/x86_64/nimf-1.3.8-1.x86_64.rpm
rpm -Kv ./build/RPMS/x86_64/nimf-1.3.8-1.x86_64.rpm
```

## Build from Source
Open the terminal and run the following commands step by step.

### Install Dependencies:
```
# debian, ubuntu, linuxmint
sudo apt-get install build-essential devscripts debhelper autoconf automake libglib2.0-dev \
    intltool gtk-doc-tools libgtk-3-dev libgtk2.0-dev libqt5core5a libqt5gui5 libqt5widgets5 \
    qtbase5-dev libayatana-appindicator3-dev librsvg2-bin libhangul-dev anthy libxkbcommon-dev \
    libxklavier-dev libm17n-dev m17n-db librime-dev

# fedora
sudo dnf install anthy-devel gcc-c++ glib2-devel gtk-doc gtk2-devel gtk3-devel intltool \
    libappindicator-gtk3-devel libhangul-devel librime-devel librsvg2-tools libtool \
    libxkbcommon-devel libxklavier-devel m17n-db-devel m17n-lib-devel \
    qt5-qtbase-devel qt5-qtbase-private-devel qt6-qtbase-devel qt6-qtbase-private-devel \
    wayland-devel naver-nanum-fonts-all wayland-protocols-devel expat expat-devel \
    libayatana-appindicator-gtk3-devel.x86_64 im-chooser rpm-build rpmdevtools

# opensuse-leap
sudo zypper install rpm-build rpmdevtools anthy anthy-devel libexpat-devel \
    gcc-c++ glib2-devel gtk-doc gtk2-devel gtk3-devel intltool libhangul-devel \
    libtool libxkbcommon-devel libxklavier-devel wayland-devel wayland-protocols-devel \
    google-noto-sans-cjk-fonts libappindicator3-devel libayatana-appindicator3-devel librsvg-devel \
    libqt5-qtbase-devel libqt5-qtbase-private-headers-devel \
    qt6-base-devel qt6-base-private-devel rsvg-convert librime-devel m17n-lib-devel rpm-build rpmdevtools

```

### Compile and Install:
```
git clone --recurse-submodules https://github.com/hamonikr/nimf

cd nimf

./autogen.sh
./configure --prefix=/usr/local
make -j $(nproc)
sudo make install

echo -e "\nexport GTK_IM_MODULE=nimf\nexport QT4_IM_MODULE=xim\nexport QT_IM_MODULE=nimf\nexport XMODIFIERS=@im=nimf" >> ~/.bashrc \
 && echo -e "\nexport GTK_IM_MODULE=nimf\nexport QT4_IM_MODULE=xim\nexport QT_IM_MODULE=nimf\nexport XMODIFIERS=@im=nimf" >> ~/.zshrc
```

There are configuration options. Use it for your situation.
```
--disable-hardening     Disable hardening
--disable-nimf-anthy    Disable nimf-anthy
--disable-nimf-m17n     Disable nimf-m17n
--disable-nimf-rime     Disable nimf-rime
--with-im-config-data   Install im-config data
--with-imsettings-data  Install imsettings data

ex)
./configure --disable-nimf-m17n
```
If you are using im-config
```
./autogen.sh --with-im-config-data
```
If you are using im-chooser
```
./autogen.sh --with-imsettings-data
```

### Uninstall:
To uninstall nimf, run the following command.
```
sudo make uninstall
```

## Debugging
```
nimf --debug

# for nimf-indicator
nimf-settings --gapplication-service & 

tail -f /var/log/syslog

export GTK_IM_MODULE="nimf"
export QT4_IM_MODULE="xim"
export QT_IM_MODULE="nimf"
export CLUTTER_IM_MODULE="xim"
export XMODIFIERS="@im=nimf"
export G_MESSAGES_DEBUG=nimf

# run application
gedit or kate # for Qt
```

## How to remove completely?
```
sudo apt purge '*nimf*'
```