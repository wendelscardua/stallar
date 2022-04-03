#include "lib/neslib.h"
#include "lib/nesdoug.h"
#include "main.h"
#include "irq_buffer.h"
#include "temp.h"
#include "mmc3/mmc3_code.h"
#include "../assets/metatiles.h"
#include "../assets/nametables.h"
#include "../assets/palettes.h"
#include "../assets/levels.h"
#include "music/soundtrack.h"

#pragma bss-name(push, "ZEROPAGE")
unsigned char * current_level_ptr;
unsigned char current_level_columns;
unsigned char next_metatile_column;
#pragma bss-name(pop)

#pragma bss-name(push, "BSS")
unsigned char attributes[128];
unsigned char first_column_stripe[30];
unsigned char second_column_stripe[30];
#pragma bss-name(pop)

#pragma rodata-name ("RODATA")
#pragma code-name ("CODE")

void select_level (void);
void load_next_column (void);

void main_start (void) {
  current_game_state = Main;
  if (irq_array[0] != 0xff) {
    while(!is_irq_done() ){}
    irq_array[0] = 0xff;
    double_buffer[0] = 0xff;
  }

  clear_vram_buffer();

  pal_fade_to(4, 0);
  ppu_off(); // screen off
  // draw some things
  vram_adr(NTADR_A(0,0));
  vram_unrle(empty_nametable);

  next_metatile_column = 0;
  current_level_ptr = (unsigned char *) levels[0];
  select_level();

  for(i = 0; i < 32; i++) { // TODO: at least 16 columns
    clear_vram_buffer();
    load_next_column();
    flush_vram_update_nmi();
  }
  vram_adr(NTADR_A(0,0));

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

void main_upkeep (void) {
}

void main_sprites (void) {
}

void select_level (void) {
  current_level_columns = *current_level_ptr++;
  // TODO: load level configs, like entities and stuff - stop at start of metatile data
}

void load_next_column (void) {
  if (next_metatile_column < 16) {
    temp_int = NTADR_A((next_metatile_column * 2), 0);
  } else {
    temp_int = NTADR_B(((next_metatile_column & 15) * 2), 0);
  }

  for(j = 0; j < 30; j += 2) {
    temp_char = *current_level_ptr++;
    temp = 5 * temp_char;
    first_column_stripe[j] = metatiles[temp];
    first_column_stripe[j+1] = metatiles[temp+2];
    second_column_stripe[j] = metatiles[temp+1];
    second_column_stripe[j+1] = metatiles[temp+3];

    temp_attr = metatiles[temp+4];
    temp_char = 0b11;

    temp_y = j >> 1;
    temp_x = (next_metatile_column & 15);
    if (temp_y & 1) {
      temp_attr <<= 4;
      temp_char <<= 4;
    }
    if (temp_x & 1) {
      temp_attr <<= 2;
      temp_char <<= 2;
    }
    temp_y >>= 1;
    temp_x >>= 1;
    temp = temp_y * 8 + temp_x;
    if (next_metatile_column >= 16) {
      temp += 64;
    }
    attributes[temp] = (attributes[temp] & (~temp_char)) | temp_attr;
  }

  multi_vram_buffer_vert((const char *)first_column_stripe, 30, temp_int);
  multi_vram_buffer_vert((const char *)second_column_stripe, 30, temp_int + 1);

  for(j = 0; j < 8; j++) {
    temp = j * 8 + ((next_metatile_column & 15) >> 1);
    if (next_metatile_column < 16) {
      one_vram_buffer(attributes[temp], 0x23c0 + temp);
    } else {
      one_vram_buffer(attributes[temp], 0x27c0 + temp);
    }
  }

  --current_level_columns;
  next_metatile_column++;
  if (next_metatile_column == 32) next_metatile_column = 0;
}
