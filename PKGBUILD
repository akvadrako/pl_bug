pkgname=pl_bug
pkgver=1
pkgrel=1
arch=('x86_64')
license=('GPL')
depends=('wayland' 'libplacebo-git' 'mesa')

build() {
    make -C ..
}

package() {
    install -D  ../pl_bug "$pkgdir/usr/bin/pl_bug"
}
