# Ishihara

![Ishihara on a color Pebble](evidence/basalt-hero.png)

A new field of colored dots every minute. A smooth numeral within the plate
shows the local hour from one to twelve; a small caption gives the exact time
in your watch's 12- or 24-hour format.

Tap the watch to show or hide the hour and minute hands. The watch remembers
your choice. Hands start hidden so the whole plate is visible. The face redraws
on the minute, on a tap, or when it returns to view.

Color watches alternate among coral and mint, pink and green, and violet and
yellow treatments. Monochrome watches use filled dots for the numeral and
small outlined dots around it. Pebble's limited palette makes this a decorative
clock, not a color-vision screening test.

## Watches and build

The project targets Basalt, Chalk, Diorite, Emery, Flint, and Gabbro. The plate
adapts to the display size and leaves room for the time caption. It runs offline without phone
configuration or permissions.

Install Pebble SDK 4.33.1, then run:

```sh
make test
pebble build --sdk 4.33.1
pebble install --emulator basalt build/ishihara.pbw
```

Host tests use Clang and check all twelve numeral masks, dot spacing,
deterministic packing, minute changes, and midnight/noon encodings.
`test/emulator.py` uses the Python environment installed with `pebble-tool`.
It checks native rollovers, regeneration, idle stability, tap controls, and
hand preference persistence after relaunch.

```sh
python test/emulator.py basalt --fresh
```

Reports and captures go to `build/evidence/`. Each report records the SHA-256
of the installed package. Physical-watch battery life and outdoor readability
need separate observation.

## Source

Luke Steuber created the original
[Ishihara clock](https://datapoems.io/clocks/ishihara/) and this native
adaptation under the MIT license. The supplied HTML and its checksum remain
in `reference/`.

The face retains the source's local hour, per-minute seeded regeneration,
organic varied-radius packing, and changing palettes. Its bounded native
packing uses spatial rejection with a visible gap between circles. The native
palette approximates the source colors; it does not preserve the source's
CIELAB confusion-line claims. A minute cadence replaces the web animation and
second hand.

The smooth masks and caption use Barlow Condensed Bold under the SIL Open
Font License in `resources/fonts/OFL.txt`. To rebuild the checked-in masks,
install Pillow and run `python3 tools/masks.py`.
