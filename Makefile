PROJECT=stallar
LD65_FLAGS=
CA65_FLAGS=
EMULATOR=/mnt/c/NESDev/Mesen.exe
NSF2DATA=/mnt/c/NESDev/famitone5.0/nsf2data/nsf2data5.exe
TEXT2DATA=/mnt/c/NESDev/famitone5.0/text2data/text2vol5.exe
FAMITRACKER=/mnt/c/NESDev/famitracker/FamiTracker.exe
VERSION := $(shell git describe --exact-match --tags 2> /dev/null || git rev-parse --short HEAD)

TARGET=${PROJECT}.nes

.PHONY: debug run usage release FORCE

default: ${TARGET}

debug: LD65_FLAGS += --dbgfile ${PROJECT}.dbg
debug: CA65_FLAGS += -g -DDEBUG=1
debug: ${TARGET}

${TARGET}: MMC3.cfg \
           src/main.o src/crt0.o \
           src/lib/unrle.o src/lib/subrand.o src/lib/farcall.o \
           src/nametable_loader.o \
           src/irq_buffer.o \
           src/temp.o \
           src/wram.o \
           src/title-game-state.o \
           src/main-game-state.o \
           src/game-over-game-state.o \
           assets/nametables.o \
           assets/palettes.o \
           assets/sprites.o \
           assets/metatiles.o \
           assets/levels.o \
           assets/metasprites.o
	ld65 -C $^ nes.lib -m map.txt -o ${TARGET} ${LD65_FLAGS}

%.o: %.s
	ca65 $< ${CA65_FLAGS}

src/main.s: src/main.c \
            src/lib/nesdoug.h \
            src/lib/neslib.h \
            src/lib/unrle.h \
            src/mmc3/mmc3_code.h \
            src/music/soundtrack.h \
            src/charmap.h \
            src/irq_buffer.h \
            src/nametable_loader.h \
            src/wram.h \
            assets/nametables.h \
            assets/palettes.h \
            assets/sprites.h
	cc65 -Oirs $< --add-source ${CA65_FLAGS}

src/nametable_loader.s: src/nametable_loader.c \
                        src/lib/nesdoug.h
	cc65 -Oirs $< --add-source ${CA65_FLAGS}

src/irq_buffer.s: src/irq_buffer.c
	cc65 -Oirs $< --add-source ${CA65_FLAGS}

src/temp.s: src/temp.c
	cc65 -Oirs $< --add-source ${CA65_FLAGS}

src/wram.s: src/wram.c \
            src/lib/neslib.h
	cc65 -Oirs $< --add-source ${CA65_FLAGS}

src/title-game-state.s: src/title-game-state.c
	cc65 -Oirs $< --add-source ${CA65_FLAGS}

src/main-game-state.s: src/main-game-state.c
	cc65 -Oirs $< --add-source ${CA65_FLAGS}

src/game-over-game-state.s: src/game-over-game-state.c
	cc65 -Oirs $< --add-source ${CA65_FLAGS}

src/crt0.o: src/crt0.s src/mmc3/mmc3_code.asm src/lib/neslib.s src/lib/nesdoug.s assets/*.chr \
            src/music/soundtrack.s src/music/soundfx.s
	ca65 $< ${CA65_FLAGS}

assets/nametables.o: assets/nametables.s assets/nametables.h \
                     assets/nametables/empty.rle \
                     assets/nametables/title.rle
	ca65 $< ${CA65_FLAGS}

assets/palettes.o: assets/palettes.s assets/palettes.h \
                   assets/bg.pal assets/sprites.pal
	ca65 $< ${CA65_FLAGS}

assets/sprites.o: assets/sprites.s assets/sprites.h
	ca65 $< ${CA65_FLAGS}

assets/metatiles.o: assets/metatiles.s
	ca65 $< ${CA65_FLAGS}

assets/levels.o: assets/levels.s \
                 assets/maps/level-00.inc \
                 assets/maps/level-01.inc \
                 assets/maps/level-02.inc
	ca65 $< ${CA65_FLAGS}

assets/metatiles.s: assets/metatiles.map tools/generate-metatiles.rb
	ruby tools/generate-metatiles.rb $< $@

assets/level.s:

assets/maps/%.inc: assets/maps/%.tmx tools/process-level.rb
	ruby tools/process-level.rb $< $@

assets/metasprites.s: assets/metasprites.c src/constants.h
	cc65 $< ${CA65_FLAGS}

src/music/soundtrack.s: src/music/soundtrack.txt
	${TEXT2DATA} $^ -ca65 -allin

src/music/soundtrack.txt: src/music/soundtrack.ftm
	${FAMITRACKER} $^ -export $@

src/music/soundfx.nsf: src/music/soundfx.ftm
	${FAMITRACKER} src/music/soundfx.ftm -export src/music/soundfx.nsf

src/music/soundfx.s: src/music/soundfx.nsf
	${NSF2DATA} src/music/soundfx.nsf -ca65 -ntsc

%.rle: %.nam
	ruby tools/rle-compress.rb $< $@

clean:
	rm src/*.o src/main.s src/*/*.o \
           src/music/soundfx.nsf src/music/soundfx.s \
           src/music/soundtrack.txt src/music/soundtrack.s \
           assets/*.o assets/nametables/*.rle \
           *.nes *.dbg map.txt -f

run: debug
	${EMULATOR} ${TARGET}

usage: tools/ld65-map.json

tools/ld65-map.json: map.txt MMC3.cfg tools/ld65-map.rb
	ruby tools/ld65-map.rb map.txt MMC3.cfg tools/ld65-map.json

release: ${TARGET}
	cp ${TARGET} ${PROJECT}-${VERSION}.nes

FORCE:
