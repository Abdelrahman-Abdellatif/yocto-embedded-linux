# Bullet OpenSTLinux Infotainment

A custom Yocto layer building an automotive-style In-Vehicle Infotainment (IVI)
image for the **STM32MP257F-DK** (STM32MP2 series, Cortex-A35 + Cortex-M33
coprocessor), built on top of **ST's OpenSTLinux distribution**.

## Highlights

- Built and verified on real hardware (STM32MP257F-DK)
- Deliberately paired with a second project,
  [meta-abou-infotainment](https://github.com/Abdelrahman-Abdellatif/meta-abou-infotainment),
  which builds the *same board* fully from scratch on Poky/OpenEmbedded — see
  the [comparison section](#poky-vs-openstlinux-whats-the-actual-difference)
  below for what building it both ways actually revealed.
- Shares a common `audio` bbclass across both projects rather than duplicating
  it, since audio support has nothing to do with which base distro is used.
- Clean separation between original work and vendor/upstream layers — see
  [What's Included / What's Not](#whats-included--whats-not-and-why).

## Table of Contents
- [Hardware & Software Stack](#hardware--software-stack)
- [Repository Structure](#repository-structure)
- [What's Included / What's Not (and Why)](#whats-included--whats-not-and-why)
- [Dependency on meta-ivi](#dependency-on-meta-ivi)
- [Poky vs. OpenSTLinux: What's the Actual Difference?](#poky-vs-openstlinux-whats-the-actual-difference)
- [How to Use This Repository](#how-to-use-this-repository)
- [Challenges Faced & How I Solved Them](#challenges-faced--how-i-solved-them)
- [License](#license)

## Hardware & Software Stack

| Component           | Choice |
|---------------------|--------|
| Board                | STM32MP257F-DK (STM32MP25, Cortex-A35 + Cortex-M33) |
| Base distribution    | ST OpenSTLinux (Weston variant) |
| Qt version           | Qt 6.11 (meta-qt6) |
| Display/compositor   | Weston (Wayland) |
| Base image           | ST's `st-image-qt.bb` (Qt6 + Weston + ST demo apps) |

## Repository Structure

```
meta-bullet-openstlinux/
└── recipes-core/
    └── images/
        └── ivi-demo-image.bb   # Extends ST's st-image-qt image
```

### ivi-demo-image.bb
Built directly on top of ST's own `st-image-qt.bb` reference image (Qt6 +
Weston + ST's demo applications), adding:
- `nano` — on-target text editor for debugging
- `miraclecast` — Miracast-based wireless screen mirroring (phone/laptop
  screen casting to the board)
- The `audio` bbclass (see below) — ALSA + PipeWire audio stack
- `debug-tweaks` image feature for development convenience

## What's Included / What's Not (and Why)

This repo contains **only this custom layer**. It does not include:
- `openembedded-core`, `meta-openembedded`, `meta-qt6` — large upstream
  projects, cloned from their own repos.
- `meta-st-stm32mp`, `meta-st-openstlinux`, `meta-st-x-linux-qt` — ST's BSP
  layers, which include components covered by an End User License Agreement.
  These come directly from ST (typically via their `repo`/manifest-based
  distribution package) and require accepting ST's license terms yourself —
  they are not something I can or should redistribute.
- `meta-ivi` — already published separately in `meta-abou-infotainment` (see
  below); reused here rather than duplicated.

## Dependency on meta-ivi

The `audio` bbclass used by `ivi-demo-image.bb` is inherited directly from
[meta-abou-infotainment](https://github.com/Abdelrahman-Abdellatif/meta-abou-infotainment)'s
`meta-ivi` layer, rather than duplicated here. Audio support has nothing to do
with which base distro (Poky vs. OpenSTLinux) is in use, so it made sense to
share one implementation across both projects instead of maintaining two
copies. To build this image, `meta-ivi` from that repo must also be present
in your `bblayers.conf`.

## Poky vs. OpenSTLinux: What's the Actual Difference?

Building the same hardware both ways surfaced a concrete, first-hand answer
to "what does a vendor BSP distribution actually save you?":

| | Poky/OpenEmbedded build (meta-abou-infotainment) | OpenSTLinux build (this repo) |
|---|---|---|
| Distro config | Written from scratch (`infotainment.conf`) | ST's OpenSTLinux distro, used as-is |
| Base image | `core-image-weston.bb` (OE-Core reference) | `st-image-qt.bb` (ST's own reference image, already includes Qt6 + Weston + demo apps) |
| Init system integration | Had to explicitly exclude `sysvinit` backfill myself | Already resolved correctly by ST's distro config |
| Secure boot / OP-TEE / TF-A integration | Assembled from ST's BSP layer manually, alongside Poky | Pre-wired as part of ST's distribution |
| Flashing tooling | Same `create_sdcard_from_flashlayout.sh` script either way | Same |
| Build issues encountered | DNS, stale build artifacts, package/version mismatches, systemd/sysvinit conflicts | Primarily resource limits (OOM) on large Qt components |

The short version: **OpenSTLinux gets you to a working, board-specific image
much faster**, because ST has already resolved the countless small
integration decisions (which init system, which reference image, how secure
boot wires in) that you're otherwise responsible for when starting from bare
Poky/OpenEmbedded. The tradeoff is depth of understanding — building it
yourself surfaces, and forces you to actually understand, every one of those
decisions individually.

## How to Use This Repository

This build uses the following layer set (verified via `bitbake-layers
show-layers` on a working build):

```
openembedded-core         (meta)
meta-openembedded          (meta-python, meta-oe, meta-gnome,
                             meta-multimedia, meta-networking, meta-webserver)
meta-qt6                   (Qt 6.11)
meta-st-stm32mp            \
meta-st-openstlinux         >  from ST, requires accepting ST's EULA
meta-st-x-linux-qt         /
meta-ivi                   (from meta-abou-infotainment)
meta-bullet-openstlinux    (this repo)
```

```bash
mkdir -p ~/yocto/layers && cd ~/yocto/layers

# Core layers
git clone <openembedded-core-url> openembedded-core
git clone <meta-openembedded-url> meta-openembedded
git clone -b 6.11 https://github.com/qt/meta-qt6.git

# ST's OpenSTLinux distribution package (BSP layers + build scripts)
# Obtained via ST's repo/manifest workflow — see ST's own instructions:
# https://wiki.st.com/stm32mpu/wiki/STM32MP2_Distribution_Package
# (provides meta-st-stm32mp, meta-st-openstlinux, meta-st-x-linux-qt)

# My own layers
git clone https://github.com/Abdelrahman-Abdellatif/meta-abou-infotainment.git
git clone https://github.com/Abdelrahman-Abdellatif/meta-bullet-openstlinux.git

# Add all of the above to build/conf/bblayers.conf, then:
bitbake ivi-demo-image
```

## Challenges Faced & How I Solved Them

### Compiler and packaging processes killed mid-build
**Symptom:** `qtdeclarative-native` (Qt's QML engine, built as a host tool)
failed compiling with `g++: fatal error: Killed signal terminated program
cc1plus`. Separately, `qtbase`'s `.deb` packaging step failed with
`dpkg-deb: error: <compress> from tar -cf subprocess was killed by signal
(Killed)` while compressing the `qtbase-dbg` debug-symbols package.

**Root cause:** The Linux OOM killer terminating memory-heavy processes —
the same underlying issue encountered in the Poky build, but surfacing in two
new places: compiling Qt 6.11's notably larger `qtdeclarative` component, and
compressing a large `-dbg` package, both under high build parallelism.

**Fix:** Reduced `PARALLEL_MAKE` and `BB_NUMBER_THREADS` in this build's own
`local.conf`, then force-rebuilt the affected recipes with
`bitbake <recipe> -c cleansstate`.

**Lesson:** Build parallelism settings are per build-directory, not global —
tuning them in one Yocto build (e.g. a Poky-based one) has no effect on a
separate, independent build directory for a different distro. Each
`build/conf/local.conf` needs its own resource tuning based on what it's
actually compiling.

## License

This repository's original content is licensed under the MIT License — see
[LICENSE](LICENSE).
