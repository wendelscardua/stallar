# NES Template

A template for my NES games, using C and the MMC3 mapper, using nesdoug and neslib public domain libraries.

## Build dependencies

- cc65 for compiling
- FamiTone 5.0 (set `NSF2DATA` and `TEXT2DATA` paths on Makefile)
- FamiTracker (set `FAMITRACKER` path on Makefile).

Intermediate sfx and soundtrack files are commited, so FamiTone and FamiTracker
are only needed if their original files are changed.

## Building

To compile a release version:

```sh
$ make
```

...or to include the commit on the rom name:

```sh
$ make release
```

To compile a debug version:

```sh
$ make debug
```

To compile a debug version and open it on an
emulator (with path defined by the `EMULATOR` variable on Makefile):

```sh
$ make run
```

### Public domain content
* Music: Ted Karr
* SFX: Shiru, Baŝto
