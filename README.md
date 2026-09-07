# Ishihara

![Ishihara on a color Pebble](evidence/basalt-hero.png)

The local hour hides in a circle of colored dots. Look for a number from one
to twelve. Every minute brings a fresh arrangement.

Tap your wrist to show the hour and minute hands; tap again to hide them.
The watch remembers your choice. Hands start hidden, leaving the plate on
its own. Monochrome Pebbles show the numeral with filled dots among outlined
circles. This is visual art. Its colors are not a calibrated vision test.

## Watches and build

The face supports Basalt, Chalk, Diorite, Emery, Flint, and Gabbro. It runs
offline without phone configuration or permissions. The plate sits in the
center of each screen and uses 88% of its shorter dimension.

Install Pebble SDK 4.33.1 and Node.js, then run:

```sh
make test
pebble build --sdk 4.33.1
pebble install --emulator basalt build/ishihara.pbw
```

Host tests check all twelve masks at each native display size, minute changes,
and noon and midnight encoding. They also compare 48 complete fields directly
with the pinned source: circle positions, radii, palette selection, and every
quantized color must agree.

The native checks use the Python environment installed with `pebble-tool`:

```sh
python test/emulator.py basalt --fresh
```

Reports and captures go to `build/evidence/`. Each report records the SHA-256
of the installed package and checks natural rollovers, minute regeneration,
idle stability, and the hands preference after relaunch. Native field
signatures must match the source-verified host geometry and colors. Physical-watch battery
life and outdoor readability still need observation.

## Source

Luke Steuber created the original
[Ishihara clock](https://datapoems.io/clocks/ishihara/) and this native adaptation
under the MIT license. The supplied HTML and its checksum remain in `reference/`.

Version 0.1.1 restores the source's Arial numeral proportions, centered plate,
and variable-radius Poisson packing. It uses the original CIELAB palette
recipes and per-dot variation, then rounds sRGB colors to Pebble's 64-color
palette. The watch redraws once per minute. A tap adds hour and minute hands.

The checked-in masks contain raster spans from the source's Canvas text
operation: Arial at weight 900, a font size of 55% of the plate diameter for
one digit or 42% for two, and an 8% stroke. They need no font during a normal
build. To regenerate them, use macOS with Arial installed and a Chromium
browser, then run `python3 tools/masks.py`. Set `CHROME` to select its executable.
The project does not distribute the Arial font.

`release/` retains each version's watch package and source archive. The source
archive includes native evidence and builds independently outside this checkout.
