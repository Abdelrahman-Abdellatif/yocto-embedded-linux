SUMMARY = "Connect external monitors to your system via Wi-Fi Display (miracast)"

LICENSE = "GPL-2.0-only & LGPL-2.1-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=7314adf793af1c4ce355355a659e6891 \
                    file://LICENSE_gdhcp;md5=42c8401a5a2eda9c8e4419921d4e8559 \
                    file://LICENSE_htable;md5=2d5025d4aa3495befef8f17206a5b0a1 \
                    file://LICENSE_lgpl;md5=4fbd65380cdd255951079008b364516c"

SRC_URI = "git://github.com/albfan/miraclecast.git;protocol=https;branch=master"
PV = "1.0+git"
SRCREV = "0b7f1f1f6586dc65ff480f3cda5c2170a70aa020"
S = "${WORKDIR}/git"

# Build-time: what the C source actually compiles/links against
DEPENDS = "systemd glib-2.0 readline"

# Run-time: separate processes miraclecast spawns once it's running, never linked
RDEPENDS:${PN} = "gstreamer1.0 gstreamer1.0-plugins-base wpa-supplicant dbus"

FILES:${PN} += "${datadir}/bash-completion"
RDEPENDS:${PN} += "python3-core bash"

inherit cmake pkgconfig

EXTRA_OECMAKE = "-DBUILD_TESTS=OFF -DENABLE_SYSTEMD=ON -DBUILD_ENABLE_CPPCHECK=OFF"