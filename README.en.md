Nimf is a lightweight, fast and extensible input method framework.

Nimf provides:
  * Input Method Server:
    * nimf
  * Language Engines:
    * System keyboard
    * Chinese (based on librime)
    * Japanese (based on anthy)
    * Korean (based on libhangul)
    * Various languages (based on m17n)
  * Service Modules:
    * Indicator (based on appindicator)
    * Wayland
    * NIM (Nimf Input Method)
    * XIM (based on IMdkit)
    * Preedit window
    * Candidate
  * Client Modules:
    * GTK+2, GTK+3, Qt4, Qt5
  * Settings tool to configure the Nimf:
    * nimf-settings
  * Development files:
    * C library, headers and documents

Project Homepage:
  * https://github.com/hamonikr/nimf

## Configure

* For GNOME Shell, use 3rd party gnome-shell-extension-appindicator

  https://github.com/ubuntu/gnome-shell-extension-appindicator
  https://extensions.gnome.org/extension/615/appindicator-support/

* How to enable Nimf on systems using im-config
  ```
  im-config -n nimf
  ```

* How to enable Nimf on systems using im-chooser
  ```
  imsettings-switch nimf
  ```
* How to enable Nimf on systems using systemd v233 or later
  1. Run `nimf-settings`.
  2. Turn on the "Setup environment variables" option in the Nimf menu.

## Debugging
```bash
  nimf --debug
  nimf-settings --gapplication-service & # for nimf-indicator
  tail -f /var/log/daemon.log # or /var/log/syslog

  export GTK_IM_MODULE="nimf"
  export QT4_IM_MODULE="nimf"
  export QT_IM_MODULE="nimf"
  export XMODIFIERS="@im=nimf"
  export G_MESSAGES_DEBUG=nimf
  gedit # or kate for Qt
```


## Build

### install prequisite

* Debian, Ubuntu
```bash
sudo apt install -y libglib2.0-dev libgtk-3-dev libgtk2.0-dev qtbase5-dev \
  qtbase5-private-dev libappindicator3-dev librsvg2-bin libhangul-dev \
  libanthy-dev anthy librime-dev libxkbcommon-dev libwayland-dev wayland-protocols \
  libxklavier-dev libm17n-dev m17n-db gtk-doc-tools devscripts build-essential debhelper
```

* fedora, CentOS, RHEL
```bash
sudo yum install gcc-c++ libtool glib2-devel \
  gtk3-devel gtk2-devel qt5-qtbase-devel \
  qt5-qtbase-private-devel libappindicator-gtk3-devel librsvg2-tools \
  google-noto-cjk-fonts libhangul-devel anthy-devel anthy \
  libxkbcommon-devel wayland-devel libxklavier-devel gtk-doc
```

* archlinux

```bash
sudo pacman -S binutils base-devel libappindicator-gtk3 libhangul anthy librime m17n-lib m17n-db gtk-doc
```

### install
```
meson build
ninja -C build
sudo ninja -C build install
```

## References

* APIs

  http://www.x.org/releases/X11R7.6/doc/libX11/specs/XIM/xim.html
  http://www.w3.org/TR/ime-api/
  https://developer.chrome.com/extensions/input_ime
  https://docs.enlightenment.org/stable/efl/group__Ecore__IMF__Lib__Group.html
  http://doc.qt.io/qt-4.8/qinputcontext.html
  http://doc.qt.io/qt-5/qinputmethod.html
  https://git.gnome.org/browse/gtk+/tree/gtk/gtkimcontext.c

* Language Engines (alphabetically listed)

  http://anonscm.debian.org/cgit/collab-maint/anthy.git
  https://github.com/libhangul/libhangul
  https://github.com/rime/librime
  https://www.nongnu.org/m17n/

* Implementations

  https://github.com/libhangul/nabi
  https://github.com/libhangul/imhangul
  https://github.com/libhangul/ibus-hangul
  https://github.com/ibus/ibus
  https://github.com/fcitx/fcitx
  https://github.com/fcitx/fcitx-qt5
  https://github.com/uim/uim
