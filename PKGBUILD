# Maintainer: lrdsnow
# Contributor: lrdsnow
pkgname=libpuredrive
pkgver=0.1.0
pkgrel=1
pkgdesc="PureDrive SDK for Linux"
arch=('i686' 'x86_64')
url="https://github.com/Lrdsnow/PureDriveSDKCPP"
license=('GPLv3' 'AGPLv3')
depends=('bluez-libs' 'dbus') # also requires simpleble
options=('!strip' '!emptydirs')
source=()

build() {
    cd "$startdir"
    make lib
}

package() {
    cd "$startdir"

    mkdir -p "${pkgdir}/usr/include"
    cp include/PureDrive.hpp "${pkgdir}/usr/include"

    mkdir -p "${pkgdir}/usr/lib"
    cp build/lib/libPureDrive.a "${pkgdir}/usr/lib"
}
