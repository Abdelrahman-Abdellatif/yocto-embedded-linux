SUMMARY = "ncurses-based Space Invaders clone"
DESCRIPTION = "nInvaders is a terminal based game following the space invaders family."
LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://gpl.txt;md5=393a5ca445f6965873eca0259a17f833"

DEPENDS = "ncurses"

SRC_URI = "https://downloads.sourceforge.net/project/ninvaders/ninvaders/0.1.1/ninvaders-0.1.1.tar.gz"
SRC_URI[sha256sum] = "bfbc5c378704d9cf5e7fed288dac88859149bee5ed0850175759d310b61fd30b"
SRC_URI += "file://joystick-support.patch"

S = "${UNPACKDIR}/ninvaders-0.1.1"

CFLAGS += "-std=gnu17 -fcommon"

EXTRA_OEMAKE = "-e"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 nInvaders ${D}${bindir}/ninvaders
}

