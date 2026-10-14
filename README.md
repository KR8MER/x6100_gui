# X6100 LVGL GUI — KR8MER Lightning Edition

This fork by **KR8MER** adds the **Lightning** theme (Settings → Theme), styled
after the EAS Station Lightning theme: storm-navy panels, electric-yellow and
cyan edges, a glowing scope-style spectrum, gradient buttons and menus, a red
flash on key-down, and a KR8MER boot logo and splash. Lightning is the default on
a fresh install. Settings → UI → Clock time zone (and an optional second zone)
changes the time the clock shows; the radio's own clock stays on UTC. Images are on this fork's
[Releases](https://github.com/KR8MER/x6100_gui/releases) page.

Built on the X6100 GUI by R1CBU and the OpenX6100 contributors.


This is part of an alternative firmware for X6100 using the LVGL library

## Installing

Open [Releases](https://github.com/gdyuldin/x6100_gui/releases/latest) page and download `sdcard.img` file (in Assets section). With balenaEtcher or Rufus
burn img file to microSD card. Insert card to the transceiver and boot it.

## Importing ADI log

Application could mark already worked callsign in the UI.
To load information about previous QSOs - copy your ADI log to the `DATA` partition and rename it to `incoming_log.adi`.
Application will import records to own log on the next boot and will rename `incoming_log.adi` to `incoming_log.adi.bak`.

*Note*: `DATA` partition will be created after first launch transceiver with inserted SD card.


## Exporting ADI log

Application stores FT8/FT4 QSOs to the `ft_log.adi` file on the `DATA` partition of SD card. This file might be used to load QSOs to online log.


## Building


* Clone repositories

```
mkdir x6100
cd x6100
git clone https://github.com/gdyuldin/AetherX6100Buildroot
git clone https://github.com/gdyuldin/x6100_gui
```

* Build buildroot

```
cd AetherX6100Buildroot
git submodule init
git submodule update
./br_config.sh
cd build
make
cd ../..
```

* Build app

```
cd x6100_gui
git submodule init
git submodule update
cd buildroot
./build.sh
```
