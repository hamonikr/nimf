Name:     nimf
Summary:  An input method framework
Version:  2019.02.17
Release:  1%{?dist}
License:  LGPLv3+
Group:    User Interface/Desktops
URL:      https://gitlab.com/nimf-i18n/nimf
Source0:  %{name}-%{version}.tar.xz

BuildRequires: gcc-c++
BuildRequires: libtool
BuildRequires: glib2-devel
BuildRequires: pkgconfig
BuildRequires: intltool >= 0.50.1
BuildRequires: gtk3-devel
BuildRequires: gtk2-devel
BuildRequires: libqt4-devel
BuildRequires: libqt5-qtbase-devel
BuildRequires: libQt5Gui-private-headers-devel
BuildRequires: libappindicator3-devel
BuildRequires: rsvg-view
BuildRequires: noto-sans-cjk-fonts
BuildRequires: libhangul-devel
BuildRequires: anthy-devel
BuildRequires: anthy
BuildRequires: librime-devel >= 1.2.9
BuildRequires: libxkbcommon-devel
BuildRequires: wayland-devel
BuildRequires: libxklavier-devel
BuildRequires: m17n-lib-devel

Requires:         im-chooser
Requires:         anthy
Requires(post):   %{_sbindir}/alternatives
Requires(postun): %{_sbindir}/alternatives

%define _xinputconf %{_sysconfdir}/X11/xinit/xinput.d/nimf.conf

%description
Nimf is an input method framework which has a module-based client-server
architecture in which an application acts as a client and communicates
synchronously with the Nimf server via a unix socket.

%package devel
Summary:  Development files for nimf
Group:    Development/Libraries
Requires: %{name}%{?_isa} = %{version}-%{release}
Requires: glib2-devel
Requires: gtk3-devel

%description devel
This package contains development files.

%prep
%setup -q

%build
./autogen.sh --prefix=/usr --libdir=%{_libdir} --with-imsettings-data
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
if [ "$1" = "0" ]; then
  %{_sbindir}/alternatives --remove xinputrc %{_xinputconf} || :
  # if alternative was set to manual, reset to auto
  [ -L %{_sysconfdir}/alternatives/xinputrc -a "`readlink %{_sysconfdir}/alternatives/xinputrc`" = "%{_xinputconf}" ] && %{_sbindir}/alternatives --auto xinputrc || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/glib-compile-schemas %{_datadir}/glib-2.0/schemas &> /dev/null || :

%files
%config %{_xinputconf}
%config %{_sysconfdir}/apparmor.d/abstractions/nimf
%{_bindir}/*
%{_libdir}/gtk-2.0/*
%{_libdir}/gtk-3.0/*
%{_libdir}/libnimf.so.*
%{_libdir}/nimf/*
%{_libdir}/qt4/*
%{_libdir}/qt5/*
%{_datadir}/applications/*
%{_datadir}/glib-2.0/*
%{_datadir}/icons/*
%{_datadir}/locale/*
%{_datadir}/man/*

%files devel
%{_includedir}/*
%{_libdir}/libnimf.so
%{_libdir}/pkgconfig/*

%changelog
* Sun Feb 17 2019 Hodong Kim <cogniti@gmail.com> - 2019.02.17-1
- See https://gitlab.com/nimf-i18n/nimf/blob/master/debian/changelog
