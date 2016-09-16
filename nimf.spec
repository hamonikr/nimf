Name:     nimf
Summary:  An input method framework
Version:  2016.09.16
Release:  1%{?dist}
License:  LGPLv3+
Group:    User Interface/Desktops
URL:      https://github.com/cogniti/nimf
Source0:  %{name}-%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: libtool
BuildRequires: gobject-introspection-devel
BuildRequires: glib2-devel
BuildRequires: pkgconfig
BuildRequires: intltool >= 0.50.1
BuildRequires: gtk3-devel
BuildRequires: gtk2-devel
BuildRequires: qt4-devel
BuildRequires: qt5-qtbase-devel
BuildRequires: libappindicator-gtk3-devel
BuildRequires: librsvg2-tools
BuildRequires: google-noto-cjk-fonts
BuildRequires: sunpinyin-devel
BuildRequires: sunpinyin-data
BuildRequires: libhangul-devel
BuildRequires: anthy-devel
BuildRequires: anthy
BuildRequires: libchewing-devel
BuildRequires: librime-devel >= 1.2.9

Requires:         im-chooser
Requires:         anthy
Requires:         sunpinyin-data
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
Requires: gobject-introspection-devel
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
%{_libdir}/girepository-1.0/*
%{_libdir}/gtk-2.0/*
%{_libdir}/gtk-3.0/*
%{_libdir}/libnimf.so.*
%{_libdir}/nimf/*
%{_libdir}/qt4/*
%{_libdir}/qt5/*
%{_datadir}/glib-2.0/*
%{_datadir}/icons/*
%{_datadir}/locale/*

%files devel
%{_includedir}/*
%{_libdir}/libnimf.so
%{_libdir}/pkgconfig/*
%{_datadir}/gir-1.0/*

%changelog
* Fri Sep 16 2016 Hodong Kim <cogniti@gmail.com> - 2016.09.16-1
- nimf-anthy: Do not use the return key to selece an item (issue #20)
- Added nimf-chewing, nimf-rime (issue #18)
- Redesigned candidate window and APIs
- Enhanced stability
- nimf-sunpinyin: Changed trigger key
- nimf-indicator: Changed website

* Sat Aug 06 2016 Hodong Kim <cogniti@gmail.com> - 2016.08.06-1
- Fixed scroll bug in the candidate window
- Fixed trivial bugs
- Added fedora-specific files related
- Remove libnimf.la
- Fixed missing nimf-context.h
- nimf-settings: Improved dialog to capture keys

* Sat Jul 23 2016 Hodong Kim <cogniti@gmail.com> - 2016.07.23-1
- nimf-indicator: Revert to 2016-06-04
- Show comments in the candidate window
- Added an extra column to the candidate window
- Added debian-specific files
