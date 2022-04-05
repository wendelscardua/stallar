#include "lib/neslib.h"
#include "lib/nesdoug.h"
#include "charmap.h"
#include "main.h"
#include "irq_buffer.h"
#include "temp.h"
#include "mmc3/mmc3_code.h"
#include "../assets/nametables.h"
#include "../assets/palettes.h"
#include "music/soundtrack.h"

#pragma bss-name(push, "ZEROPAGE")
extern unsigned char score[4];
#pragma zpsym("score")
extern unsigned char victory_lap;
#pragma zpsym("victory_lap")
#pragma bss-name(pop)

#pragma rodata-name ("RODATA")
#pragma code-name ("CODE")

const char win[] = "You won!!!";

void game_over_start (void) {
  current_game_state = GameOver;
  if (irq_array[0] != 0xff) {
    while(!is_irq_done() ){}
    irq_array[0] = 0xff;
    double_buffer[0] = 0xff;
  }

  clear_vram_buffer();

  pal_fade_to(4, 0);

  oam_clear();

  ppu_off(); // screen off
  // draw some things
  vram_adr(NTADR_A(0,0));
  vram_unrle(game_over_nametable);

  if (victory_lap) {
    multi_vram_buffer_horz(win, 10, NTADR_A(8, 9));
  }

  for(j = 0; j < 3; j++) {
    if (score[j] != '0') {
        break;
    }
  }
  multi_vram_buffer_horz((const char *) score + j, 4 - j, NTADR_A(10 + j, 13));
  flush_vram_update_nmi();
  clear_vram_buffer();

  music_play(Cave);

  set_scroll_x(0);
  set_scroll_y(0);

  set_chr_mode_2(BG_MAIN_0);
  set_chr_mode_3(BG_MAIN_1);
  set_chr_mode_4(BG_MAIN_2);
  set_chr_mode_5(BG_MAIN_3);
  set_chr_mode_0(SPRITE_0);
  set_chr_mode_1(SPRITE_1);

  pal_bg(bg_palette);
  pal_spr(sprites_palette);

  ppu_on_all(); //	turn on screen
  pal_fade_to(0, 4);
}

void game_over_upkeep (void) {
  if (pad1_new & PAD_START) {
    title_start();
  }
}

void game_over_sprites (void) {
}
