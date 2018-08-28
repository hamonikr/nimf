#!/bin/sh
# Run this to generate all the initial makefiles, etc.
#
echo "법적 고지"
echo
echo "Nimf 소프트웨어는 대한민국 저작권법과 국제 조약의 보호를 받습니다."
echo "Nimf 개발자는 대한민국 법률의 보호를 받습니다."
echo "커뮤니티의 위력을 이용하여 개발자의 시간과 노동력을 약탈하려는 행위를 금하시기 바랍니다."
echo
echo "* 커뮤니티 게시판에 개발자를 욕(비난)하거나"
echo "* 욕보이는(음해하는) 글을 작성하거나"
echo "* 허위 사실을 공표하거나"
echo "* 명예를 훼손하는"
echo
echo "등의 행위는 정보통신망 이용촉진 및 정보보호 등에 관한 법률의 제재를 받습니다."
echo
echo "면책 조항"
echo
echo "Nimf 는 무료로 배포되는 오픈소스 소프트웨어입니다."
echo "Nimf 개발자는 개발 및 유지보수에 대해 어떠한 의무도 없고 어떠한 책임도 없습니다."
echo "어떠한 경우에도 보증하지 않습니다. 도덕적 보증 책임도 없고, 도의적 보증 책임도 없습니다."
echo "Nimf 개발자는 리브레오피스, 이클립스 등 귀하가 사용하시는 소프트웨어의 버그를 해결해야 할 의무가 없습니다."
echo "Nimf 개발자는 귀하가 사용하시는 배포판에 대해 기술 지원을 해드려야 할 의무가 없습니다."
echo

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

mkdir -p m4

PKGCONFIG=`which pkg-config`
if test -z "$PKGCONFIG"; then
    echo "pkg-config not found, please install pkg-config"
    exit 1
fi

pkg-config --print-errors glib-2.0
if [ $? != 0 ]; then
    echo "You probably need to install libglib2.0-dev or glib2-devel"
    exit 1
fi

LIBTOOLIZE=`which libtoolize`
if test -z $LIBTOOLIZE; then
    echo "libtoolize not found, please install libtool package"
    exit 1
fi

INTLTOOLIZE=`which intltoolize`
if test -z $INTLTOOLIZE; then
    echo "intltoolize not found, please install intltool package"
    exit 1
else
    intltoolize --force --copy --automake || exit $?
fi

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
    echo "autoreconf not found, please install autoconf package"
    exit 1
else
    autoreconf --force --install --verbose || exit $?
fi

cd "$olddir"
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
