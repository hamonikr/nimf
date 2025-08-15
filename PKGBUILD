# Maintainer: Kevin Kim <root@hamonikr.org>

pkgname=nimf
pkgver=1.3.10
pkgrel=1
pkgdesc="Nimf is an input method framework."
arch=('any')
url="https://github.com/hamonikr/nimf"
license=('LGPL3')
makedepends=('binutils' 'autoconf' 'automake' 'gcc' 'make' 'glib2' 'glib2-devel' 'intltool'
             'gtk3' 'gtk2' 'qt5-base' 'qt6-base' 'libappindicator-gtk3' 'libayatana-appindicator' 'librsvg'
             'noto-fonts-cjk' 'libhangul-git' 'anthy' 'librime' 'libxkbcommon'
             'wayland' 'wayland-protocols' 'libxklavier' 'm17n-lib' 'm17n-db' 'gtk-doc')
depends=('glib2' 'gtk3' 'gtk2' 'qt5-base' 'qt6-base' 'libappindicator-gtk3' 'libhangul-git'
         'anthy' 'librime' 'libxkbcommon' 'wayland' 'libxklavier' 'm17n-lib'
         'm17n-db')
provides=('nimf-git')
conflicts=('nimf-git')         
optdepends=('brise: Rime schema repository'
            'noto-fonts-cjk: Google Noto CJK fonts')
source=("git+https://github.com/hamonikr/nimf.git#tag=v1.3.8")
md5sums=('SKIP')

build() {
	cd nimf
	./autogen.sh --prefix=/usr --enable-gtk-doc --disable-nimf-rime
	make -j $(nproc)
}

package() {
	cd nimf
	make DESTDIR="${pkgdir}/" install
}

post_install() {
	cat <<EOF
To use Nimf as your input method framework, add the following lines to your ~/.xprofile:

export GTK_IM_MODULE=nimf
export QT4_IM_MODULE="nimf"
export QT_IM_MODULE=nimf
export QT6_IM_MODULE=nimf
export XMODIFIERS="@im=nimf"

EOF
}
