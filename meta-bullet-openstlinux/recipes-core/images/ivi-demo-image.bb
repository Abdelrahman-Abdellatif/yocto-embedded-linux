# Image recipe for the IVI demo image, based on the ST demo image, with additional packages and features.
require recipes-st/images/st-image-qt.bb

SUMMARY = "Bullet infotainment demo image - Qt6 + Weston + ST demos + custom packages"

inherit audio

IMAGE_INSTALL:append = " nano miraclecast"
IMAGE_FEATURES:append = " debug-tweaks"