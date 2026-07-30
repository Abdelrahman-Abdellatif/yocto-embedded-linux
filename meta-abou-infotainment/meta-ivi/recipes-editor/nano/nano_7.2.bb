# --- Package metadata ---
SUMMARY = "Small and friendly console text editor"
# One-line description shown by tools like `bitbake-layers show-recipes`.

HOMEPAGE = "https://www.nano-editor.org/"
# Where the upstream project lives — informational only.

LICENSE = "GPL-3.0-only"
# The real nano source is GPL-3.0-only. Your original recipe added
# "GFDL-1.2" too because recipetool scanned COPYING.DOC (the docs license),
# but since we're not packaging the docs, we don't need to declare it.

LIC_FILES_CHKSUM = "file://COPYING;md5=f27defe1e96c2e1ecd4e0c9be8967949"
# Bitbake verifies the license file's md5 hasn't changed between builds/versions.
# This is a checksum of the COPYING file's *content*, not the tarball itself —
# it's how bitbake catches "the license text changed, go re-check it."

# --- Source fetching ---
PV = "7.2"
# Package Version. This now matches a real released version instead of
# the placeholder "1.0+git" — because we're no longer building from a
# moving git branch, PV should reflect an actual release.

SRC_URI = "https://nano-editor.org/dist/v7/nano-${PV}.tar.xz"
# The fix: fetch the official release tarball instead of "git://...".
# The tarball already contains the gnulib-bootstrapped lib/ directory
# and ABOUT-NLS that autoreconf needs — that's the whole bug, solved.

SRC_URI[sha256sum] = "86f3442768bd2873cec693f83cdf80b4b444ad3cc14760b74361474fc87a4526"
# Bitbake refuses to build if the downloaded tarball's sha256 doesn't match
# this. Protects against a corrupted download or a tampered mirror.
# (No more SRCREV — that variable only applies to git/svn/etc. fetches,
# not plain tarball downloads.)

# S = "${WORKDIR}/git" removed.
# Bitbake's default S is "${WORKDIR}/${BPN}-${PV}", which for a tarball
# named nano-7.2.tar.xz already unpacks to nano-7.2/ — matches the default,
# so we don't need to set S at all anymore.

# --- Build-time dependencies ---
DEPENDS = "ncurses file"
# ncurses: nano needs its terminal-drawing library at compile time.
# file: provides libmagic, which nano links against for file-type detection.
# (Dropped "zlib" — the real recipe doesn't need it; nano's own compression
# use, if any, is optional and not required for a base build.)

# --- Build system ---
inherit autotools gettext pkgconfig
# autotools: gives you the standard do_configure/do_compile/do_install tasks
#   built around ./configure && make && make install.
# gettext: nano supports translated UI strings; this class wires up the
#   native gettext tools needed during the build.
# pkgconfig: lets configure scripts locate library flags via .pc files
#   (used here to find ncurses' compile/link flags).

# No custom do_configure() needed — inheriting `autotools` already
# provides a working do_configure that calls oe_runconf for you.
# Your original recipe's do_configure(){ oe_runconf } was redundant;
# it just re-declared what the class already does by default.