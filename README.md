# Ishihara

![Ishihara on a Pebble Time](evidence/basalt-hero.png)

A Pebble watchface that hides the hour in a circle of colored dots, like a
color-vision test plate. Look for a number from one to twelve. Every minute
brings a fresh arrangement.

Tap your wrist to show the hour and minute hands; tap again to hide them. The
watch remembers your choice. Hands start hidden.

This is visual art, not a vision test. The colors are rounded to Pebble's
64-color palette and are not calibrated.

[Get it on the Pebble Appstore](https://apps.repebble.com/6fa77e3f4a294024abca332a),
where it is listed as Colorblind Clock.

## Watches

Version 0.1.2 runs on the color watches: Pebble Time and Time Steel (Basalt),
Pebble Time Round (Chalk), Pebble Time 2 (Emery) and Pebble Round 2 (Gabbro).
It works offline, with no phone settings or permissions. The plate sits in the
center of the screen and fills 88% of its shorter side. The face redraws once
a minute.

## How it works

I made the original [Ishihara clock](https://datapoems.io/clocks/ishihara/)
for the web; this is the native port. The HTML it was ported from and its
checksum are in `reference/`.

The port keeps the source's Arial numeral shapes, centered plate and
variable-radius Poisson dot packing. It uses the original CIELAB palette
recipes and per-dot variation, then rounds each color to the nearest Pebble
color.

The numeral masks in `src/c/masks.h` are raster spans from the source's
canvas text drawing: Arial at weight 900, a font size of 55% of the plate
diameter for one digit or 42% for two, and an 8% stroke. A normal build needs
no font. To regenerate them, use macOS with Arial installed and a Chromium
browser, then run `python3 tools/masks.py` (set `CHROME` to pick the browser
executable). This repository does not include the Arial font.

## Build

Install Pebble SDK 4.33.1, Node.js and clang, then run:

```sh
make test
pebble build --sdk 4.33.1
pebble install --emulator basalt build/ishihara.pbw
```

`make test` checks all twelve masks at each screen size, minute changes, and
noon and midnight. It also compares 48 complete plates with the original web
code: dot positions, radii, palette choice and every rounded color must match.

The emulator check uses the Python environment that comes with `pebble-tool`:

```sh
python test/emulator.py basalt --fresh
```

It writes reports and screenshots to `build/evidence/`, and checks rollovers,
the minute redraw, a still plate between minutes, and the hands setting after
a relaunch.

`release/` holds the 0.1.0 and 0.1.1 watch packages and source archives. The
screenshots in `evidence/` come from 0.1.1, which also ran on the
black-and-white Pebble 2 and Pebble 2 Duo; 0.1.2 is color-only. Build logs in
earlier commits and the release archives contain absolute paths from the
original build machine.

## License

MIT. See [LICENSE](LICENSE).
