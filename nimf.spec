Name:     nimf
Summary:  An input method framework
Version:  1.3.7
Release:  1%{?dist}
License:  LGPLv3+
Group:    User Interface/Desktops
URL:      https://github.com/hamonikr/nimf

BuildRequires: gcc-c++
BuildRequires: libtool
BuildRequires: glib2-devel
BuildRequires: pkgconfig
BuildRequires: intltool >= 0.50.1
BuildRequires: gtk3-devel
BuildRequires: gtk2-devel
BuildRequires: git
%if 0%{?is_opensuse}
BuildRequires: libqt5-qtbase-devel
BuildRequires: libQt5Gui-private-headers-devel
BuildRequires: libappindicator3-devel
BuildRequires: ayatana-appindicator3-0.1
BuildRequires: rsvg-view
BuildRequires: noto-sans-cjk-fonts
BuildRequires: libqt6-qtbase-devel
BuildRequires: libQt6Gui-private-headers-devel
%else
BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtbase-private-devel
BuildRequires: qt6-qtbase-devel
BuildRequires: qt6-qtbase-private-devel
BuildRequires: libappindicator-gtk3-devel
BuildRequires: librsvg2-tools
BuildRequires: google-noto-cjk-fonts
%endif
BuildRequires: libhangul-devel
BuildRequires: anthy-devel
BuildRequires: anthy
BuildRequires: libxkbcommon-devel
BuildRequires: wayland-devel
BuildRequires: libxklavier-devel
BuildRequires: gtk-doc
%if 0%{?fedora} || 0%{?is_opensuse}
BuildRequires: librime-devel >= 1.2.9
BuildRequires: m17n-lib-devel >= 1.7.0
%endif
%if 0%{?fedora}
BuildRequires: m17n-db-devel >= 1.7.0
%endif
%if 0%{?is_opensuse}
BuildRequires: m17n-db >= 1.7.0
%endif

Requires: anthy
Requires: glib2
Requires: gtk3
Requires: im-chooser
%if 0%{?is_opensuse}
Requires: libappindicator3
%else
Requires: libappindicator-gtk3
%endif
Requires: libhangul
Requires: libxkbcommon
Requires: libxklavier
Requires: qt5-qtbase
Requires: qt6-qtbase
%if 0%{?fedora} || 0%{?is_opensuse}
Requires: librime
Requires: m17n-lib >= 1.7.0, m17n-db >= 1.7.0
%endif
Requires(post):   %{_sbindir}/alternatives
Requires(postun): %{_sbindir}/alternatives

%define _xinputconf %{_sysconfdir}/X11/xinit/xinput.d/nimf.conf

%description
Nimf is a lightweight, fast and extensible input method framework.

%package devel
Summary:  Development files for nimf
Group:    Development/Libraries
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: glib2-devel
Requires: gtk3-devel

%description devel
This package contains development files.

%prep
%autosetup -c -T
git clone --branch fedora40 https://github.com/hamonikr/nimf.git .
git submodule update --init --recursive
autoreconf -ivf

%build
%if 0%{?rhel}
./autogen.sh --prefix=/usr --libdir=%{_libdir} --enable-gtk-doc \
  --with-imsettings-data --disable-nimf-m17n --disable-nimf-rime
%else
./autogen.sh --prefix=/usr --libdir=%{_libdir} --with-imsettings-data --enable-gtk-doc
%endif
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
%make_install

%clean

%post
/sbin/ldconfig
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
%{_bindir}/update-gtk-immodules %{_host} || :
%{_bindir}/gtk-query-immodules-3.0-%{__isa_bits} --update-cache || :
%{_sbindir}/alternatives --install %{_sysconfdir}/X11/xinit/xinputrc xinputrc %{_xinputconf} 55 || :

%postun
/sbin/ldconfig
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi
%{_bindir}/update-gtk-immodules %{_host} || :
%{_bindir}/gtk-query-immodules-3.0-%{__isa_bits} --update-cache || :
if [ "$1" = "0"] ; then
  %{_sbindir}/alternatives --remove xinputrc %{_xinputconf} || :
  # if alternative was set to manual, reset to auto
  [ -L %{_sysconfdir}/alternatives/xinputrc -a "`readlink %{_sysconfdir}/alternatives/xinputrc`" = "%{_xinputconf}" ] && %{_sbindir}/alternatives --auto xinputrc || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/glib-compile-schemas %{_datadir}/glib-2.0/schemas &> /dev/null || :

%files
%config /etc/X11/xinit/xinput.d/nimf.conf
%config /etc/apparmor.d/abstractions/nimf
%config /etc/input.d/nimf.conf
%config /etc/xdg/autostart/nimf-settings-autostart.desktop
%{_bindir}/nimf
%{_bindir}/nimf-settings
%{_includedir}/nimf/*
/usr/lib/debug/usr/bin/nimf-1.3.7-1.fc40.x86_64.debug
/usr/lib/debug/usr/bin/nimf-settings-1.3.7-1.fc40.x86_64.debug
/usr/lib/debug/usr/lib/x86_64-linux-gnu/libnimf.so.1.0.0-1.3.7-1.fc40.x86_64.debug
/usr/lib/x86_64-linux-gnu/libnimf.so
/usr/lib/x86_64-linux-gnu/libnimf.so.1
/usr/lib/x86_64-linux-gnu/libnimf.so.1.0.0
/usr/lib/x86_64-linux-gnu/nimf/modules/*.so
/usr/lib/x86_64-linux-gnu/nimf/modules/services/*.so
/usr/lib/x86_64-linux-gnu/nimf/mssymbol.txt
/usr/lib/x86_64-linux-gnu/pkgconfig/nimf.pc
/gtk-2.0/immodules/im-nimf-gtk2.so
/usr/lib64/gtk-3.0/3.0.0/immodules/im-nimf-gtk3.so
/usr/lib64/qt5/plugins/platforminputcontexts/libqt5im-nimf.so
/usr/lib64/qt6/plugins/platforminputcontexts/libqt6im-nimf.so
%{_datadir}/applications/nimf-settings.desktop
%{_datadir}/glib-2.0/schemas/*.xml
%{_datadir}/gtk-doc/html/nimf/*
%{_datadir}/icons/hicolor/*/status/*.png
%{_datadir}/icons/hicolor/*/status/*.svg
%{_datadir}/locale/*/LC_MESSAGES/nimf.mo
%{_datadir}/man/man1/nimf-settings.1.gz
%{_datadir}/man/man1/nimf.1.gz

%files devel
%{_includedir}/nimf/*
/usr/lib/x86_64-linux-gnu/libnimf.so
/usr/lib/x86_64-linux-gnu/pkgconfig/nimf.pc
%{_datadir}/gtk-doc/html/nimf/*

%changelog
* Wed Sep 23 2020 HamoniKR <pkg@hamonikr.org> - 1.3.7-1
- See https://github.com/hamonikr/nimf/blob/master/debian/changelog
