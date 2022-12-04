#
# spec file for package libhangul
#
# Copyright (c) 2019 SUSE LINUX GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#
%define soname 1
Name:           libhangul
Version:        1.0.0~gitga3d8eb6
Release:        0
Summary:        The Hangul input library
License:        LGPL-2.1-or-later
Group:          System/I18n/Korean
URL:            https://github.com/libhangul/libhangul
Source:         %{name}-%{version}.tar.xz
# FIX-FOR-SLES downgrade gettext requirement to 0.14 from 0.18
BuildRequires:  intltool
BuildRequires:  libexpat-devel
BuildRequires:  libtool
BuildRequires:  pkgconfig

%description
Hangul input library used by scim-hangul and ibus-hangul

%package -n %{name}%{soname}
Summary:        The Hangul input library
Group:          System/I18n/Korean

%description -n %{name}%{soname}
Hangul input library used by scim-hangul and ibus-hangul

%package devel
Summary:        Development headers for libhangul
Group:          Development/Libraries/C and C++
Requires:       %{name}%{soname} = %{version}

%description devel
This package contains all necessary include files and libraries needed
to develop applications that require libhangul.

%prep
%setup -q
%if 0%{?sles_version}
%patch -p1
%endif
# Fix for factory gettext version
%if 0%{?suse_version} > 1310
sed -i "s/0\.18/0\.19/" configure.ac
%endif
# fix for configure.ac
if [ ! -f config.rpath ]; then
  touch config.rpath
fi
NOCONFIGURE=1 ./autogen.sh
# Can't merge with the %%if 0%{?sles_version} above,
# This applies _after_ the autogen.sh
%if 0%{?sles_version}
# MKINSTALLDIRS doesn't exist, use MKDIR_P
sed -i "s/@MKINSTALLDIRS@/@MKDIR_P@/" po/Makefile.in.in
sed -i "32d" po/Makefile.in.in
sed -i "32imkinstalldirs= @mkdir_p@" po/Makefile.in.in
cat po/Makefile.in.in
%endif

%build
export CFLAGS="%{optflags}"
export CXXFLAGS="%{optflags}"
%configure --disable-static --with-pic
make %{?_smp_mflags}

%install
%make_install
find %{buildroot} -type f -name "*.la" -delete -print
%find_lang %{name}

%post -n %{name}%{soname} -p /sbin/ldconfig
%postun -n %{name}%{soname} -p /sbin/ldconfig

%files -n %{name}%{soname} -f %{name}.lang
# /usr/share/licenses is not owned by any package on SLE 12 SP2 and older
%if 0%{?sle_version} <= 120200 && !0%{?is_opensuse}
%doc COPYING
%else
%license COPYING
%endif
%doc AUTHORS NEWS README
%{_bindir}/hangul
%{_libdir}/libhangul.so.1
%{_libdir}/libhangul.so.1.0.0
%{_datadir}/libhangul/

%files devel
%dir %{_includedir}/hangul-1.0/
%{_includedir}/hangul-1.0/*
%{_libdir}/libhangul.so
%{_libdir}/pkgconfig/libhangul.pc

%changelog
