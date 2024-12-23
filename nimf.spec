# detect os_id 
%define os_id %(source /etc/os-release && echo $ID | tr - _)

Name:     nimf
Summary:  An input method framework
Version:  1.3.8
Release:  2%{?dist}.%{?os_id}
License:  LGPLv3+
Group:    User Interface/Desktops
URL:      https://github.com/hamonikr/nimf
Source0:  %{name}-%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: libtool
BuildRequires: glib2-devel
BuildRequires: pkgconfig
BuildRequires: intltool >= 0.50.1
BuildRequires: gtk3-devel
BuildRequires: gtk2-devel
BuildRequires: git
%if 0%{?suse_version}
BuildRequires: libexpat-devel
BuildRequires: libqt5-qtbase-devel
BuildRequires: libQt5Gui-private-headers-devel
BuildRequires: libqt5-qtbase-private-headers-devel
BuildRequires: libappindicator3-devel
BuildRequires: libayatana-appindicator3-devel
BuildRequires: rsvg-convert
BuildRequires: librsvg-devel
BuildRequires: google-noto-sans-cjk-fonts
BuildRequires: qt6-base-devel
BuildRequires: qt6-base-private-devel
%else
BuildRequires: expat
BuildRequires: expat-devel
BuildRequires: im-chooser
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
%if 0%{?fedora} || 0%{?suse_version}
BuildRequires: librime-devel
BuildRequires: m17n-lib-devel
%endif
%if 0%{?fedora}
BuildRequires: m17n-db-devel >= 1.7.0
%endif
%if 0%{?suse_version}
BuildRequires: m17n-db >= 1.7.0
%endif

Requires: anthy
Requires: glib2
Requires: gtk3
Requires: git
%if 0%{?suse_version}
Requires: libappindicator3-1
BuildRequires: libexpat-devel
Requires: libhangul1
Requires: libxkbcommon0
Requires: libxklavier16
Requires: libqt5-qtbase-devel
Requires: qt6-base-devel
Requires: librime1
Requires: m17n-lib >= 1.7.0
Requires: m17n-db >= 1.7.0
%else
Requires: im-chooser
Requires: libappindicator-gtk3
Requires: expat
Requires: expat-devel
Requires: libhangul
Requires: libxkbcommon
Requires: libxklavier
Requires: qt5-qtbase
Requires: qt6-base-devel
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
%setup -q -n nimf
autoreconf -ivf
# Clone and build libhangul
git submodule update --init --recursive
cd libhangul
./autogen.sh
./configure --prefix=/usr
make
sudo make install
cd ..

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
echo "Install latest libhangul ..."
# Compile and install libhangul
{
  cd /tmp
  git clone https://github.com/libhangul/libhangul.git
  cd libhangul
  ./autogen.sh
  ./configure --prefix=/usr
  make
  make install
  cd ..
  rm -rf libhangul
} > /dev/null 2>&1
echo "Finished install latest libhangul ..."

/sbin/ldconfig
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
if [ -x %{_bindir}/update-gtk-immodules ]; then
  %{_bindir}/update-gtk-immodules %{_host} || :
fi
%{_bindir}/gtk-query-immodules-3.0-%{__isa_bits} --update-cache || :
%{_sbindir}/alternatives --install %{_sysconfdir}/X11/xinit/xinputrc xinputrc %{_xinputconf} 99 || :

# OpenSUSE와 Fedora 모두에 환경변수 추가
%if 0%{?suse_version} || 0%{?fedora}
echo "Add environment variables ..."
for dir in /home/*; do
  if [ -d "$dir" ]; then
    # .xprofile 파일에 환경변수 추가
    echo -e "\n# Nimf environment variables\nexport GTK_IM_MODULE=nimf\nexport QT4_IM_MODULE=xim\nexport QT_IM_MODULE=nimf\nexport XMODIFIERS=@im=nimf" >> "$dir/.xprofile"
    # 파일 권한 설정
    chown $(basename $dir):$(basename $dir) "$dir/.xprofile"
  fi
done
%endif

%postun
/sbin/ldconfig
if [ $1 -eq 0 ]; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi
if [ -x %{_bindir}/update-gtk-immodules ]; then
  %{_bindir}/update-gtk-immodules %{_host} || :
fi
%{_bindir}/gtk-query-immodules-3.0-%{__isa_bits} --update-cache || :
if [ "$1" = "0" ]; then
  %{_sbindir}/alternatives --remove xinputrc %{_xinputconf} || :
  # if alternative was set to manual, reset to auto
  [ -L %{_sysconfdir}/alternatives/xinputrc -a "`readlink %{_sysconfdir}/alternatives/xinputrc`" = "%{_xinputconf}" ] && %{_sbindir}/alternatives --auto xinputrc || :
fi

# OpenSUSE와 Fedora 모두에서 환경변수 제거
%if 0%{?suse_version} || 0%{?fedora}
echo "Remove environment variables ..."
for dir in /home/*; do
  if [ -d "$dir" ]; then
    if [ -f "$dir/.xprofile" ]; then
      sed -i '/# Nimf environment variables/,+4d' "$dir/.xprofile"
    fi
  fi
done
%endif

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/glib-compile-schemas %{_datadir}/glib-2.0/schemas &> /dev/null || :

%files
%config %{_xinputconf}
%config %{_sysconfdir}/apparmor.d/abstractions/nimf
%{_bindir}/*
%{_libdir}/gtk-2.0/*
%{_libdir}/gtk-3.0/*
/usr/lib/x86_64-linux-gnu/libnimf.so*
/usr/lib/x86_64-linux-gnu/nimf/*
%{_libdir}/qt5/*
%{_libdir}/qt6/*
%{_datadir}/applications/*
%{_datadir}/glib-2.0/*
%{_datadir}/icons/*
%{_datadir}/locale/*
%{_datadir}/man/*
%{_sysconfdir}/input.d/nimf.conf
%{_sysconfdir}/xdg/autostart/*

%files devel
%{_datadir}/gtk-doc/*
%{_includedir}/*
/usr/lib/x86_64-linux-gnu/libnimf.so
/usr/lib/x86_64-linux-gnu/pkgconfig/*

%changelog
* Mon Jul 26 2024 Kevin Kim <chaeya@gmail.com> - 1.3.8-2
- Fixed dependancy for opensuse-leap

* Mon Jul 08 2024 HamoniKR <pkg@hamonikr.org> - 1.3.8-1
- See https://github.com/hamonikr/nimf/blob/master/debian/changelog
