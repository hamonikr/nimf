pkgname=nimf
pkgver=1.3.0.r33.68a65d6
pkgrel=1
pkgdesc="Nimf is an input method framework."
arch=('any')
url="https://github.com/hamonikr/nimf"
license=('LGPL3')
makedepends=('binutils' 'autoconf' 'automake' 'gcc' 'make' 'glib2' 'intltool'
             'gtk3' 'gtk2' 'qt5-base' 'libappindicator-gtk3' 'libayatana-appindicator' 'librsvg'
             'noto-fonts-cjk' 'libhangul-git' 'anthy' 'librime' 'libxkbcommon'
             'wayland' 'wayland-protocols' 'libxklavier' 'm17n-lib' 'm17n-db' 'gtk-doc')
depends=('glib2' 'gtk3' 'gtk2' 'qt5-base' 'libappindicator-gtk3' 'libhangul-git'
         'anthy' 'librime' 'libxkbcommon' 'wayland' 'libxklavier' 'm17n-lib'
         'm17n-db')
provides=('nimf-git')
conflicts=('nimf-git')         
optdepends=('brise: Rime schema repository'
            'noto-fonts-cjk: Google Noto CJK fonts')
source=("git+https://github.com//hamonikr/nimf.git")
md5sums=('SKIP')

pkgver() {
	cd nimf
	printf "%s" "$(git describe --long | sed 's/nimf-//' | sed 's/\([^-]*-\)g/r\1/;s/-/./g')"
}

build() {
	cd nimf
	./autogen.sh --prefix=/usr --enable-gtk-doc
	make -j $(nproc)
}

package() {
	cd nimf
	make DESTDIR="${pkgdir}/" install
}
