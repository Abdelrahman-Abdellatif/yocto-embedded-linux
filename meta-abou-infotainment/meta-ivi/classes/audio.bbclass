# classes/audio.bbclass
# Adds the ALSA + PipeWire userspace audio stack for playback and recording.
# Any image that needs audio does: inherit audio

IMAGE_INSTALL:append = " \
    alsa-utils \
    pipewire \
    pipewire-pulse \
    wireplumber \
    alsa-plugins \
"