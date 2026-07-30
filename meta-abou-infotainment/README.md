# ABOU Infotainment

A custom Yocto/OpenEmbedded Linux distribution for an automotive-style In-Vehicle
Infotainment (IVI) system, built for the **STM32MP257F-DK** (STM32MP2 series,
Cortex-A35 + Cortex-M33 coprocessor), built entirely from upstream Poky and
OpenEmbedded layers.

This repository contains my own custom Yocto layers: two distro configurations
and an image/application layer, built on top of Poky, OpenEmbedded, ST's BSP
layer, and meta-qt6.

## Table of Contents
- [Hardware & Software Stack](#hardware--software-stack)
- [Repository Structure](#repository-structure)
- [Two Distro Variants](#two-distro-variants)
- [What's Included / What's Not (and Why)](#whats-included--whats-not-and-why)
- [How to Use This Repository](#how-to-use-this-repository)
- [Flashing the Image](#flashing-the-image)
- [Challenges Faced & How I Solved Them](#challenges-faced--how-i-solved-them)
- [Roadmap](#roadmap)
- [License](#license)

## Hardware & Software Stack

| Component           | Choice |
|---------------------|--------|
| Board                | STM32MP257F-DK (STM32MP25, Cortex-A35 + Cortex-M33) |
| Yocto release        | Scarthgap (5.0) |
| Kernel               | linux-stm32mp, pinned to 6.6.x |
| BSP layer            | meta-st-stm32mp |
| Display/compositor   | Weston (Wayland) |
| Package management   | apt/dpkg (on-target package management enabled) |
| Security             | OP-TEE + ARM Trusted Firmware, secure boot chain |

> Built on the Poky/OpenEmbedded build system (BitBake + OE-Core), using a
> fully custom distro configuration (`infotainment`) rather than the Poky
> reference distro — required for a real hardware target with its own
> DISTRO_FEATURES, init system, and packaging policy.
## Repository Structure

```
meta-abou-infotainment/
├── meta-ivi/            # Image recipe, custom recipes, audio bbclass
├── meta-info-distro/    # "infotainment" distro: systemd + Qt6
└── meta-audio-distro/   # "audio" distro: sysvinit, no Qt6
```

### meta-ivi
- `recipes-core/images/ivi-test-image.bb` — the image recipe, built on top of
  `core-image-weston`, adding SSH, Qt6, a Wayland-based screen-mirroring stack
  (miraclecast), and audio.
- `classes/audio.bbclass` — a reusable class any image can `inherit audio` to
  pull in the full ALSA + PipeWire userspace audio stack.
- `recipes-connectivity/miraclecast/` — screen-mirroring support (Miracast
  sink/source), used as a stand-in for automotive screen-mirroring use cases.
- `recipes-editor/nano/` — on-target text editor for debugging/development.

### meta-info-distro
- `conf/distro/infotainment.conf` — the "infotainment" distro: systemd,
  Qt6, Wayland, Vulkan, Bluetooth, wifi.
- `conf/distro/include/systemd.inc` — systemd-as-init-manager configuration.

### meta-audio-distro
- `conf/distro/audio.conf` — a second, leaner distro variant: sysvinit as
  init manager, no Qt6/Wayland/Vulkan, aimed at an audio-only target
  configuration.
- `conf/distro/include/sysvinit.inc` — sysvinit-as-init-manager configuration.

## Two Distro Variants

This project deliberately builds the same hardware two different ways, to
compare a systemd + Qt6 graphical stack against a lighter sysvinit,
non-graphical audio-focused stack:

| | `infotainment` (meta-info-distro) | `audio` (meta-audio-distro) |
|---|---|---|
| Init system | systemd | sysvinit |
| Qt6 / Wayland / Vulkan | Yes | No |
| Target use case | Full IVI graphical experience | Audio-only build |
| Build status | Built and verified on hardware | Configured; not yet build-verified |

## What's Included / What's Not (and Why)

This repo intentionally contains **only my own original layers**. It does not
include:
- `poky`, `meta-openembedded`, `meta-qt6` — large upstream projects; clone them
  from their own repos (links below).
- `meta-st-stm32mp` — ST's BSP layer, which includes components covered by an
  End User License Agreement. You'll need to obtain this directly from ST and
  accept their license terms yourself; it's not something I can or should
  redistribute.

## How to Use This Repository

```bash
# 1. Clone the core layers
git clone -b scarthgap git://git.yoctoproject.org/poky.git
git clone -b scarthgap git://git.openembedded.org/meta-openembedded
git clone -b 6.8 https://github.com/qt/meta-qt6.git

# 2. Get meta-st-stm32mp from ST directly (requires accepting ST's EULA)
#    See: https://wiki.st.com/stm32mpu/wiki/STM32MP2_Distribution_Package

# 3. Clone this repo into the same parent directory
git clone https://github.com/<your-username>/meta-abou-infotainment.git

# 4. Source the build environment
source poky/oe-init-build-env build

# 5. Edit build/conf/bblayers.conf to include:
#    meta, meta-poky, meta-yocto-bsp, meta-oe, meta-python,
#    meta-st-stm32mp, meta-qt6, meta-ivi, meta-info-distro
#    (add meta-audio-distro too if building the "audio" distro variant)

# 6. Set your MACHINE and DISTRO in build/conf/local.conf
echo 'MACHINE = "stm32mp25-disco"' >> conf/local.conf
echo 'DISTRO = "infotainment"' >> conf/local.conf   # or DISTRO = "audio"

# 7. Build
bitbake ivi-test-image
```

## Flashing the Image

After a successful build, ST's flashlayout tooling (shipped via meta-st-stm32mp)
generates a `.tsv` flash layout description alongside the built partitions.
A helper script turns that into a single raw SD card image:

```bash
cd tmp-glibc/deploy/images/stm32mp25-disco/scripts
./create_sdcard_from_flashlayout.sh \
    ../flashlayout_ivi-test-image/optee/FlashLayout_sdcard_stm32mp257f-dk-optee.tsv

sudo dd if=<generated-image>.raw of=/dev/sdX bs=8M conv=fdatasync status=progress
```

## Challenges Faced & How I Solved Them

Building a Yocto distro from scratch surfaces a wide range of problems —
network, toolchain, packaging, init-system, and resource-limit issues. Here's
what actually broke during this build, and how I diagnosed and fixed each one.

### 1. Git fetch failing with a generic error
**Symptom:** `do_fetch` failed cloning a GitHub dependency (QCBOR, for
TF-M/trusted firmware) with a bare `exit code 128` — no useful detail.

**Root cause:** Tracing through the fetcher's debug log down to the actual git
error revealed `Could not resolve host: github.com` — a DNS failure. Testing
against a public resolver (`nslookup github.com 1.1.1.1`) worked fine while my
system's configured DNS (my router) returned `SERVFAIL` specifically for that
lookup.

**Fix:** Pointed the network interface at a public DNS resolver
(`resolvectl dns <iface> 1.1.1.1 8.8.8.8`) instead of relying on the router's
DNS forwarder.

**Lesson:** A generic exit code from a build tool is rarely the real error —
always trace down to the underlying command's own diagnostic output.

### 2. A stale object file breaking a completely unrelated compile
**Symptom:** `binutils` failed to link its `gold/dwp` tool with
`undefined reference to 'main'`, despite `dwp.cc` (which defines `main`)
compiling without any reported error.

**Root cause:** An earlier build had been interrupted (by the DNS failure
above), leaving a partially-written, stale `dwp.o` object file on disk that
Make's timestamp logic considered "up to date" and never recompiled.

**Fix:** `bitbake binutils -c cleansstate` to force a full, clean rebuild.

**Lesson:** Interrupted builds can leave subtly corrupt intermediate artifacts;
when a failure makes no logical sense given the surrounding code, suspect stale
build state before suspecting the code.

### 3. A package that "doesn't exist"
**Symptom:** `do_rootfs` failed: `Unable to locate package qtbase-examples`.

**Root cause:** Two different clones of meta-qt6 existed on disk at different
Qt versions (6.8 vs 6.11); the active `bblayers.conf` pointed at the 6.8
branch, which — unlike 6.11 — never split Qt's examples into their own
package at all.

**Fix:** Confirmed via `grep` that the 6.8 branch's `qtbase` recipe has no
`examples` PACKAGECONFIG or package split whatsoever, and simply removed
`qtbase-examples` from the image's `IMAGE_INSTALL`.

**Lesson:** "Unable to locate package" means the package was never even
defined for this recipe/version — not that something failed to build.

### 4. Two SSH servers fighting over the same slot
**Symptom:** `apt-get install` failed: `packagegroup-core-ssh-dropbear`
depends on `dropbear`, which wasn't installable.

**Root cause:** `core-image-weston` (which my image recipe extends) defaults
to including `ssh-server-dropbear` in `IMAGE_FEATURES`, while I'd separately
added `openssh` explicitly — the dropbear package itself was never being
built successfully in my configuration.

**Fix:** `IMAGE_FEATURES:remove = "ssh-server-dropbear"` in my own image
recipe, cleanly overriding the inherited default without touching the
upstream `core-image-weston.bb` file.

**Lesson:** Yocto's `:remove`/`:append` override syntax exists specifically so
you never need to fork or edit a base/upstream recipe just to change one
feature.

### 5. A recipe with a split brain about its own init system
**Symptom:** `do_rootfs` failed on an ALSA-state package's postinstall script,
trying to run `update-rc.d` on an init script file that had never been
installed.

**Root cause:** My distro config added `systemd` to `DISTRO_FEATURES` but
never excluded Yocto's default backfilled `sysvinit` feature — so both
`systemd` and `sysvinit` ended up present simultaneously.
ST's `alsa-state-script.bb` recipe checks `DISTRO_FEATURES` twice, in two
different places, for two different features: `do_install()` checks for
`systemd` (and correctly skipped installing the SysV init script), while the
inherited `update-rc.d` bbclass checks for `sysvinit` (and unconditionally
wired up a postinstall script to register that now-nonexistent file). Two
halves of the same recipe, disagreeing about which init system was active.

**Fix:** Added `DISTRO_FEATURES_BACKFILL_CONSIDERED += "sysvinit"` to my
`systemd.inc`, the correct Yocto mechanism for explicitly opting a distro out
of a backfilled default feature.

**Lesson:** "Systemd-only" isn't automatic just because you add `systemd` to
`DISTRO_FEATURES` — you also have to explicitly exclude `sysvinit` from being
silently backfilled in, or recipes with dual init-system support can end up
with contradictory logic.

### 6. The compiler getting killed mid-build
**Symptom:** `aarch64-oe-linux-g++: fatal error: Killed signal terminated
program cc1plus` while building Qt's `ptest` (package test) suite —
thousands of individual C++ test binaries compiling in parallel.

**Root cause:** Confirmed via `dmesg`/`journalctl` — the kernel OOM killer
had killed `cc1plus` directly (`Out of memory: Killed process ... (cc1plus)`)
after RAM was exhausted running 8 parallel, memory-heavy Qt test compiles at
once, on top of a running desktop session.

**Fix:** Dropped `ptest` from `DISTRO_FEATURES` entirely (an embedded product
image doesn't need upstream test suites built in), reduced
`PARALLEL_MAKE`/`BB_NUMBER_THREADS`, and doubled available swap as a safety
margin for future heavy recipes.

**Lesson:** `ptest` is a global feature switch that silently multiplies build
cost across every ptest-capable recipe in the build — it's meant for
automated hardware QA labs, not day-to-day product image builds.

## License

This repository's original content (meta-ivi, meta-info-distro,
meta-audio-distro) is licensed under the MIT License — see [LICENSE](LICENSE).
