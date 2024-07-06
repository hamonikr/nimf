# Building Nimf Packages

## Debian Package
To build a Debian package for Nimf, follow these steps:

### Install Dependencies:

```sudo apt-get update
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
sudo dnf install gcc-c++ libtool glib2-devel pkgconfig intltool gtk3-devel gtk2-devel qt5-qtbase-devel qt5-qtbase-private-devel qt6-qtbase-devel qt6-qtbase-private-devel libappindicator-gtk3-devel librsvg2-tools google-noto-cjk-fonts libhangul-devel anthy-devel anthy libxkbcommon-devel wayland-devel libxklavier-devel gtk-doc librime-devel m17n-lib-devel m17n-db-devel
```

### Clone the Repository:

```
git clone https://github.com/hamonikr/nimf.git
cd nimf
```

### Prepare the Source Tarball:

```
git archive --format=tar.gz --prefix=nimf/ HEAD > nimf-1.3.5.tar.gz
```

### Build the Package:

```
rpmbuild -ta nimf-1.3.5.tar.gz
```

### Install the Package:

```
sudo rpm -i ~/rpmbuild/RPMS/x86_64/nimf-1.3.5-1.x86_64.rpm
```

## Arch Linux Package
To build an Arch Linux package for Nimf, follow these steps:

### Install Dependencies:

```
sudo pacman -S --needed base-devel git glib2 gtk3 gtk2 qt5-base qt6-base libappindicator-gtk3 librsvg noto-fonts-cjk libhangul anthy librime libxkbcommon wayland libxklavier m17n-lib m17n-db gtk-doc
```

### Clone the Repository:

```
git clone https://github.com/hamonikr/nimf.git
cd nimf
```

### Build and Install the Package:

```
makepkg -si
```

By following these steps, you can build and install Nimf on Debian, RPM-based, and Arch Linux systems.


## Build from Source
Open the terminal and run the following commands step by step.
```
git clone https://github.com/hamonikr/nimf.git
cd nimf

./autogen.sh
./configure --prefix=/usr/local
make -j $(nproc)
sudo make install
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
export XMODIFIERS="@im=nimf"
export G_MESSAGES_DEBUG=nimf

# run application
gedit or kate # for Qt
```

## If you are not recognized by pkg-config even if you install the qt6-base-dev package at Ubuntu 22.04
```
sudo vi /usr/lib/pkgconfig/Qt6Core.pc

prefix=/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include/x86_64-linux-gnu/qt6

Name: Qt6 Core
Description: Qt6 Core module
Version: 6.2.4 # Replace with the exact version of the installed Qt6
Libs: -L${libdir} -lQt6Core
Cflags: -I${includedir}/QtCore
```
```
sudo vi /usr/lib/pkgconfig/Qt6Gui.pc

prefix=/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include/x86_64-linux-gnu/qt6

Name: Qt6 Gui
Description: Qt6 Gui module
Version: 6.2.4  
Libs: -L${libdir} -lQt6Gui
Cflags: -I${includedir} -I${includedir}/QtGui

```
```
sudo vi /usr/lib/pkgconfig/Qt6Widgets.pc

prefix=/usr
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include/x86_64-linux-gnu/qt6

Name: Qt6 Widgets
Description: Qt6 Widgets module
Version: 6.2.4  
Libs: -L${libdir} -lQt6Widgets
Cflags: -I${includedir}/QtWidgets


```

## How to remove completely?
```
sudo apt purge '*nimf*'
```