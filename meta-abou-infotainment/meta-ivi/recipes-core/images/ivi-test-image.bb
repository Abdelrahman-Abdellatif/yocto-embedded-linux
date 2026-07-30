require recipes-graphics/images/core-image-weston.bb

SUMMARY = "IVI test image for STM32MP25, that includes QT app"

inherit audio

IMAGE_FEATURES:remove = "ssh-server-dropbear"

IMAGE_INSTALL:append = " openssh nano miraclecast qtbase qtbase-plugins qtwayland qtwayland-plugins"

IMAGE_FEATURES:append = " debug-tweaks"