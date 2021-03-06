#include "lib/neslib.h"
#include "lib/nesdoug.h"
#include "music/soundfx.h"
#include "charmap.h"
#include "directions.h"
#include "entities.h"
#include "main.h"
#include "irq_buffer.h"
#include "temp.h"
#include "mmc3/mmc3_code.h"
#include "../assets/metasprites.h"
#include "../assets/metatiles.h"
#include "../assets/nametables.h"
#include "../assets/palettes.h"
#include "../assets/levels.h"
#include "music/soundtrack.h"

#define INITIAL_H_SPEED FP(0, 1, 0x40)
#define MAX_H_SPEED FP(0, 2, 0)
#define H_ACCEL FP(0, 0, 0x08)
#define FRICTION FP(0, 0, 0x40)
#define GRAVITY FP(0, 0, 0x50)
#define JUMP_GRAVITY FP(0, 0, 0x28)
#define JUMP_IMPULSE (-FP(0, 0x03, 0x90))

#define CAM_R_LIMIT FP(0, 0x80, 0)
#define CAM_R_SUB_LIMIT FP(0, 0x50, 0)
#define CAM_L_LIMIT FP(0, 0x10, 0)

#define SPRITE_LEFT_BORDER FP(0, 0x08, 0)
#define SPRITE_RIGHT_BORDER FP(0, 0xf7, 0)

#define PLAYER_X1 ((signed char) -3)
#define PLAYER_X2 ((signed char) 3)
#define PLAYER_Y1 ((signed char) -13)
#define PLAYER_Y2 ((signed char) 0)

#define MAX_ENTITIES 16

#pragma bss-name(push, "ZEROPAGE")

unsigned char * current_level_ptr;
unsigned char current_level_columns;
unsigned char next_metatile_column;

unsigned int player_x;
unsigned int player_y;
signed int player_dx;
signed int player_dy;
direction_t player_direction;
unsigned char player_grounded;

unsigned int camera_x;

unsigned char next_inactive_entity;

unsigned char score[4];

char * dialogue_ptr;
unsigned char dialogue_column;
unsigned int dialogue_sleep;

unsigned char victory_lap;

unsigned char player_trunc_x1, player_trunc_y1, player_trunc_x2, player_trunc_y2;

#pragma bss-name(pop)

#pragma bss-name(push, "BSS")
unsigned char attributes[128];
unsigned char first_column_stripe[30];
unsigned char second_column_stripe[30];

unsigned char load_column_state;
// 0xff = nothing
// 0x80 | column = load entities on column
// column = load attributes for column

unsigned int collision_mask[32];

unsigned int entity_x[MAX_ENTITIES];
unsigned int entity_y[MAX_ENTITIES];
void (*entity_update[MAX_ENTITIES])();
void (*entity_render[MAX_ENTITIES])();
entity_state_t entity_state[MAX_ENTITIES];
unsigned char entity_state_value[MAX_ENTITIES];
unsigned char entity_arg[MAX_ENTITIES];

unsigned char death_counter;

#pragma bss-name(pop)

#pragma rodata-name ("RODATA")
#pragma code-name ("CODE")

// ff = simple end
// fe = victory lapify
// fd = next sentence
const char victory_dialogue[] =
  "You won!\xfd"
  "Congrats!\xfe";
const char pseudo_victory_dialogue[] =
  "You won!\xfd"
  "Congrat...\xfd"
  "Wait...the goal!\xfd"
  "You skipped it!\xfd"
  "You can't do that!\xff";
const char erase_dialogue[] = "                 ";

void select_level (void);
void load_next_column (void);
void update_load_column_state (void);
void start_dying (void);
void start_victory (void);

unsigned char __fastcall__ player_bg_collide (signed char dx, signed char dy);

void main_start (void) {
  current_game_state = Main;
  if (irq_array[0] != 0xff) {
    while(!is_irq_done() ){}
    irq_array[0] = 0xff;
    double_buffer[0] = 0xff;
  }

  pal_fade_to(4, 0);
  ppu_off(); // screen off
  // draw some things
  vram_adr(NTADR_A(0,0));
  vram_unrle(empty_nametable);

  next_inactive_entity = 0;

  camera_x = FP(0, 0, 0);
  player_x = FP(0, 0x30, 0x00);
  player_y = FP(0, 0xcf, 0x00);
  player_dx = 0;
  player_dy = 0;
  player_direction = Right;
  player_grounded = 1;
  dialogue_ptr = 0;
  victory_lap = 0;

  for(i = 0; i < 4; i++) {
    score[i] = '0';
  }

  for(i = 0; i < MAX_ENTITIES; i++) {
    entity_state[i] = Inactive;
  }

  next_metatile_column = 0;
  current_level_ptr = (unsigned char *) levels[0];
  select_level();

  for(i = 0; i < 18; i++) {
    load_next_column();
    flush_vram_update2();
    update_load_column_state();
    update_load_column_state();
    flush_vram_update2();
  }
  vram_adr(NTADR_A(0,0));

  music_play(RPGBattleTheme);

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

void player_input() {
  if (pad1_new & PAD_RIGHT) {
    player_direction = Right;
    if (player_dx < INITIAL_H_SPEED) player_dx = INITIAL_H_SPEED;
  } else if (pad1 & PAD_RIGHT) {
    if (player_dx < MAX_H_SPEED) { player_dx += H_ACCEL; }
    else { player_dx = MAX_H_SPEED; }
  }
  if (pad1_new & PAD_LEFT) {
    player_direction = Left;
    if (player_dx > -INITIAL_H_SPEED) player_dx = -INITIAL_H_SPEED;
  } else if (pad1 & PAD_LEFT) {
    if (player_dx > -MAX_H_SPEED) { player_dx -= H_ACCEL; }
    else { player_dx = -MAX_H_SPEED; }
  }
  if (player_grounded && (pad1_new & PAD_A)) {
    player_dy = JUMP_IMPULSE;
    sfx_play(SFXJump, 0);
  }
}

void update_player_y() {
  // update player y
  if ((pad1 & PAD_A) && player_dy < 0) {
    player_dy += JUMP_GRAVITY;
  } else {
    player_dy += GRAVITY;
  }
  temp_int_x = player_x;
  temp_int_y = player_y + player_dy;

  if (temp_int_y >= FP(0, 0xf0, 0x00)) {
    player_y = temp_int_y;
    start_dying();
    return;
  }

  if (player_dy > 0) {
    if (player_bg_collide(PLAYER_X1, PLAYER_Y2) ||
        player_bg_collide(PLAYER_X2, PLAYER_Y2)) {
      player_dy = 0;
      temp_int_y = player_y;
      player_grounded  = 1;
    } else {
      if (player_grounded && player_dy >= FP(0, 1, 0)) {
        player_grounded = 0;
      }
    }
  } else if (player_dy < 0) {
    player_grounded = 0;
    if (temp_int_y <= FP(0, 0x10, 0x00)) {
      temp_int_y = player_y;
    } else if (player_bg_collide(PLAYER_X1, PLAYER_Y1) || player_bg_collide(PLAYER_X2, PLAYER_Y1)) {
      player_dy = 0;
      temp_int_y = player_y;
    }
  }
  player_y = temp_int_y;
}

void update_player_x (void) {
  // update player x
  temp_int_y = player_y;
  temp_int_x = player_x + player_dx;
  if (player_dx > 0) {
    if (player_bg_collide(PLAYER_X2, PLAYER_Y1) || player_bg_collide(PLAYER_X2, PLAYER_Y2)) {
      player_dx = 0;
      temp_int_x = player_x;
    }
  } else if (player_dx < 0) {
    if (player_bg_collide(PLAYER_X1, PLAYER_Y1) || player_bg_collide(PLAYER_X1, PLAYER_Y2)) {
      player_dx = 0;
      temp_int_x = player_x;
    }
  }
  player_x = temp_int_x;

  if (!(pad1 & (PAD_LEFT | PAD_RIGHT))) {
    if (player_dx > 0) {
      player_dx -= FRICTION;
      if (player_dx < 0) player_dx = 0;
    }
    if (player_dx < 0) {
      player_dx += FRICTION;
      if (player_dx > 0) player_dx = 0;
    }
  }
}

void update_camera (void) {
  set_scroll_x(0);

  // update camera
  if (player_x - camera_x >= CAM_R_LIMIT) {
    camera_x = player_x - CAM_R_LIMIT;
  } else if (player_x - camera_x >= CAM_R_SUB_LIMIT && player_dx > 0) {
    camera_x += (player_dx >> 1) + (player_dx >> 2);
  }

  if (player_x - camera_x <= CAM_L_LIMIT) {
    player_x = camera_x + CAM_L_LIMIT;
  }

  // check for column loading
  temp_int_x = INT(camera_x);
  temp_x = ((temp_int_x + 0x100) & 0x1ff) >> 4;

  if (next_metatile_column == temp_x) {
    if (current_level_columns == 0) {
      temp = 1;
      while(temp < num_levels) temp <<= 1;
      temp--;
      i = 0;
      while(i == 0 || i >= num_levels) {
        i = rand8() & temp;
      }
      current_level_ptr = (unsigned char *) levels[i];
      select_level();
    }
    load_next_column();
  }
}

void update_entities (void) {
  player_trunc_x1 = (unsigned char) (TRUNC(player_x - FP(0, 3, 0)));
  player_trunc_x2 = (unsigned char) (TRUNC(player_x + FP(0, 3, 0)));
  player_trunc_y1 = (unsigned char) (TRUNC(player_y - FP(0, 13, 0)));
  player_trunc_y2 = (unsigned char) (TRUNC(player_y + FP(0, 0, 0)));

  for(i = 0; i < MAX_ENTITIES; i++) {
    if (entity_state[i] != Inactive) {
      temp_int_x = entity_x[i] - camera_x;
      if (temp_int_x >= FP(1, 0xd0, 0x00)) {
        entity_state[i] = Inactive;
        continue;
      }

      entity_update[i]();
    }
  }
}

void update_death (void) {
  if (player_direction == Up) {
    player_y -= FP(0, 0x4, 0x00);
    death_counter--;
    if (death_counter == 0 || player_y <= FP(0, 0x20, 0x00)) {
      death_counter = 0x40;
      player_direction = Down;
    }
  }
  if (player_direction == Down) {
    if (death_counter == 0 || player_y > FP(0, 0xf0, 0x00)) {
      player_y = FP(0, 0xff, 0x00);
      if (((TRUNC(camera_x)) & 0x7f) == 0) {
        set_scroll_x((unsigned int)INT(camera_x));
        game_over_start();
      } else {
        camera_x -= FP(0, 0x01, 0x00);
      }
    } else {
      if (player_y < FP(0, 0xf0, 0x00)) player_y += FP(0, 0x4, 0x00);
      if (death_counter > 0) death_counter--;
    }
  }
}

void update_victory_lap (void) {
  if (((TRUNC(camera_x)) & 0x7f) == 0) {
    set_scroll_x((unsigned int)INT(camera_x));
    game_over_start();
  } else {
    camera_x += FP(0, 0x01, 0x00);
  }
}

#define DIALOGUE_END_DELAY (3 * 60)
#define DIALOGUE_NEXT_SENTENCE_DELAY (2 * 60)

void dialogue_update(void) {
  temp_char = *dialogue_ptr;

  if (temp_char == '\xff') {
    if ((pad1_new & (PAD_A | PAD_B | PAD_START)) || dialogue_sleep++ >= DIALOGUE_END_DELAY) {
      multi_vram_buffer_horz(erase_dialogue, 18, NTADR_A(3, 2));
      dialogue_ptr = 0;
      dialogue_sleep = 0;
    }
  } else if (temp_char == '\xfe') {
    if ((pad1_new & (PAD_A | PAD_B | PAD_START)) || dialogue_sleep++ >= DIALOGUE_END_DELAY) {
      multi_vram_buffer_horz(erase_dialogue, 18, NTADR_A(3, 2));
      dialogue_ptr = 0;
      dialogue_sleep = 0;
      start_victory();
    } else return;
  } else if (temp_char == '\xfd') {
    if ((pad1_new & (PAD_A | PAD_B | PAD_START)) || dialogue_sleep++ >= DIALOGUE_NEXT_SENTENCE_DELAY) {
      multi_vram_buffer_horz(erase_dialogue, 18, NTADR_A(3, 2));
      dialogue_ptr++;
      dialogue_column = 3;
      dialogue_sleep = 0;
    } else return;
  } else {
    if ((get_frame_count() & 0b11) == 0 || (pad1 & (PAD_A | PAD_B))) {
      one_vram_buffer(temp_char, NTADR_A(dialogue_column, 2));
      dialogue_column++;
      dialogue_ptr++;
      dialogue_sleep = 0;
      sfx_play(SFXText, 3);
    }
  }
}

void main_upkeep (void) {
  // irq scroll
  double_buffer[double_buffer_index++] = 0x1f - 1;
  temp_int = INT(camera_x);
  double_buffer[double_buffer_index++] = 0xf0;
  double_buffer[double_buffer_index++] = 0b10001000 | (TRUNC(camera_x) >= 0x80);
  double_buffer[double_buffer_index++] = 0xf5;
  double_buffer[double_buffer_index++] = temp_int & 0xff;

  update_load_column_state();

  if (dialogue_ptr) {
    dialogue_update();
    update_player_y();
    update_camera();
  } else if (victory_lap) {
    update_victory_lap();
  } else if (player_direction == Left || player_direction == Right) {
    player_input();
    update_player_y();
    update_player_x();
    update_entities();
    update_camera();
  } else {
    update_death();
  }
}

void main_sprites (void) {
  temp_int_x = player_x - camera_x;
  if (temp_int_x >= SPRITE_RIGHT_BORDER) return;
  temp_x = INT(temp_int_x);
  temp_y = INT(player_y);
  if (player_direction == Left) {
    temp_char = MSPlayer + 2;
  } else {
    temp_char = MSPlayer;
  }
  if (INT(player_x) & 0b100) temp_char++;
  oam_meta_spr(temp_x, temp_y, metasprite_list[temp_char]);

  for(i = 0; i < MAX_ENTITIES; i++) {
    if (entity_state[i] != Inactive) {
      entity_render[i]();
    }
  }
}

void select_level (void) {
  current_level_columns = *current_level_ptr++;
  // TODO: load level configs, like entities and stuff - stop at start of metatile data
}

void load_next_column (void) {
  temp_int = 0;

  for(j = 0; j < 30; j += 2) {
    temp_char = *current_level_ptr++;

    // update collision info
    temp_int <<= 1;
    if (temp_char == 1 || temp_char == 3) {
      // TODO: pull from metatile metadata
      temp_int |= 1;
    }

    // prepare buffers for vram updates
    first_column_stripe[j] = metatiles_ul[temp_char];
    first_column_stripe[j+1] = metatiles_dl[temp_char];
    second_column_stripe[j] = metatiles_ur[temp_char];
    second_column_stripe[j+1] = metatiles_dr[temp_char];

    temp_attr = metatiles_attr[temp_char];

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

  collision_mask[next_metatile_column] = temp_int;

  if (next_metatile_column < 16) {
    temp_int = NTADR_A((next_metatile_column * 2), 4);
  } else {
    temp_int = NTADR_B(((next_metatile_column & 15) * 2), 4);
  }

  multi_vram_buffer_vert((const char *)first_column_stripe + 4, 26, temp_int);
  multi_vram_buffer_vert((const char *)second_column_stripe + 4, 26, temp_int + 1);

  load_column_state = next_metatile_column;

  --current_level_columns;
  next_metatile_column++;
  if (next_metatile_column == 32) next_metatile_column = 0;
}

void entity_star_update() {
  temp_x = TRUNC(entity_x[i]);
  temp_y = TRUNC(entity_y[i]);

  if (player_trunc_x2 >= (unsigned char) (temp_x - TRUNC(FP(0,3,0))) &&
      player_trunc_x1 <= (unsigned char) (temp_x + TRUNC(FP(0,3,0))) &&
      player_trunc_y2 >= (unsigned char) (temp_y - TRUNC(FP(0,10,0))) &&
      player_trunc_y1 <= (unsigned char) (temp_y + TRUNC(FP(0,3,0)))) {
    entity_state[i] = Inactive;

    // increase score
    score[3]++;
    if (score[3] > '9') {
      score[3] = '0';
      score[2]++;
      if (score[2] > '9') {
        score[2] = '0';
        score[1]++;
        if (score[1] > '9') {
          score[1] = '0';
          score[0]++;
          if (score[0] > '9') {
            score[0] = 'X';
          }
        }
      }
    }

    // refresh score display
    for(j = 0; j < 3; j++) {
      if (score[j] != '0') {
        break;
      }
    }
    multi_vram_buffer_horz((const char *) score + j, 4 - j, NTADR_A(18 + j, 3));
    sfx_play(SFXBling, 1);
  }
}

void entity_extrastar_update() {
  entity_star_update();
  if (entity_state[i] == Inactive) {
    dialogue_column = 3;
    dialogue_ptr = (char *) pseudo_victory_dialogue;
  }
}

#define ENEMY_SPEED FP(0, 0x1, 0x00)
void entity_movable_update() {
  if (entity_state_value[i] == 0) {
    entity_state_value[i] = 16 * entity_arg[i];
    switch(entity_state[i]) {
    case MoveLeft: entity_state[i] = MoveRight; break;
    case MoveRight: entity_state[i] = MoveLeft; break;
    }
  } else {
    entity_state_value[i]--;
    switch(entity_state[i]) {
    case MoveLeft:
      entity_x[i] -= ENEMY_SPEED;
      break;
    case MoveRight:
      entity_x[i] += ENEMY_SPEED;
      break;
    }
  }
}

void entity_blob_update() {
  entity_movable_update();

  temp_x = TRUNC(entity_x[i]);
  temp_y = TRUNC(entity_y[i]);

  if (player_dy > 0) {
    // upper blob collision
    if (player_trunc_x2 >= (unsigned char) (temp_x - TRUNC(FP(0,8,0))) &&
        player_trunc_x1 <= (unsigned char) (temp_x + TRUNC(FP(0,8,0))) &&
        player_trunc_y2 >= (unsigned char) (temp_y - TRUNC(FP(0,8,0))) &&
        player_trunc_y1 <= (unsigned char) (temp_y - TRUNC(FP(0,3,0)))) {
      entity_state[i] = Inactive; // TODO: maybe dying?
      player_dy = JUMP_IMPULSE;
      sfx_play(SFXJump, 0);
      return;
    }
  }
  if (player_trunc_x2 >= (unsigned char) (temp_x - TRUNC(FP(0,8,0))) &&
      player_trunc_x1 <= (unsigned char) (temp_x + TRUNC(FP(0,8,0))) &&
      player_trunc_y2 >= (unsigned char) (temp_y - TRUNC(FP(0,8,0))) &&
      player_trunc_y1 <= (unsigned char) (temp_y + TRUNC(FP(0,0,0)))) {
    sfx_play(SFXHit, 0);
    start_dying();
  }
}

void entity_spike_update() {
  entity_movable_update();
  temp_x = TRUNC(entity_x[i]);
  temp_y = TRUNC(entity_y[i]);
  if (player_trunc_x2 >= (unsigned char) (temp_x - TRUNC(FP(0,7,0))) &&
      player_trunc_x1 <= (unsigned char) (temp_x + TRUNC(FP(0,7,0))) &&
      player_trunc_y2 >= (unsigned char) (temp_y - TRUNC(FP(0,8,0))) &&
      player_trunc_y1 <= (unsigned char) (temp_y + TRUNC(FP(0,0,0)))) {
    sfx_play(SFXHit, 0);
    start_dying();
  }
}

void entity_mapgoal_update() {
  temp_x = TRUNC(entity_x[i]);
  temp_y = TRUNC(entity_y[i]);
  if (player_trunc_x2 >= (unsigned char) (temp_x - TRUNC(FP(0,8,0))) &&
      player_trunc_x1 <= (unsigned char) (temp_x + TRUNC(FP(0,8,0))) &&
      player_trunc_y2 >= (unsigned char) (temp_y - TRUNC(FP(0,30,0))) &&
      player_trunc_y1 <= (unsigned char) (temp_y + TRUNC(FP(0,0,0)))) {
    dialogue_ptr = (char *) victory_dialogue;
    dialogue_column = 3;
    sfx_play(SFXAchieved, 2);
  }
}

void entity_star_render() {
  temp_int_x = entity_x[i] - camera_x;
  if (temp_int_x >= SPRITE_RIGHT_BORDER || temp_int_x <= SPRITE_LEFT_BORDER) return;
  temp_x = INT(temp_int_x);
  temp_y = INT(entity_y[i]);
  oam_meta_spr(temp_x, temp_y, metasprite_list[4]);
}

void entity_blob_render() {
  temp_int_x = entity_x[i] - camera_x;
  if (temp_int_x >= SPRITE_RIGHT_BORDER || temp_int_x <= SPRITE_LEFT_BORDER) return;
  temp_x = INT(temp_int_x);
  temp_y = INT(entity_y[i]);
  if (entity_state[i] == MoveLeft) {
    temp_char = MSBlob + 2;
  } else {
    temp_char = MSBlob;
  }
  if (INT(entity_x[i]) & 0b1000) temp_char++;

  oam_meta_spr(temp_x, temp_y, metasprite_list[temp_char]);
}

void entity_spike_render() {
  temp_int_x = entity_x[i] - camera_x;
  if (temp_int_x >= SPRITE_RIGHT_BORDER || temp_int_x <= SPRITE_LEFT_BORDER) return;
  temp_x = INT(temp_int_x);
  temp_y = INT(entity_y[i]);
  if (entity_state[i] == MoveLeft) {
    temp_char = MSSpike + 2;
  } else {
    temp_char = MSSpike;
  }
  if (INT(entity_x[i]) & 0b1000) temp_char++;

  oam_meta_spr(temp_x, temp_y, metasprite_list[temp_char]);
}

void entity_mapgoal_render() {
  return;
}

void update_load_column_state (void) {
  if (load_column_state < 32) {
    for(j = 0; j < 8; j++) {
      temp = j * 8 + ((load_column_state & 15) >> 1);
      if (load_column_state < 16) {
        one_vram_buffer(attributes[temp], 0x23c0 + temp);
      } else {
        one_vram_buffer(attributes[temp + 64], 0x27c0 + temp);
      }
    }
    load_column_state ^= 0x80;
  } else if (load_column_state < 0xff) {
    load_column_state ^= 0x80;
    j = *current_level_ptr++;
    temp_x = FP(0, INT(camera_x), 0);
    while(j > 0) {
      for(k = 0; k < MAX_ENTITIES; k++) {
        if (entity_state[next_inactive_entity] == Inactive) break;
        next_inactive_entity++;
        if (next_inactive_entity >= MAX_ENTITIES) next_inactive_entity = 0;
      }
      if (entity_state[next_inactive_entity] == Inactive) {
        temp = *current_level_ptr++;
        entity_x[next_inactive_entity] = temp_x + FP(0, (load_column_state << 4) + 8, 0);
        temp_y = *current_level_ptr++;
        entity_y[next_inactive_entity] = FP(0, ((temp_y) << 4) + 15, 0);
        entity_state_value[next_inactive_entity] = 0;
        switch(temp) {
        case Star:
          entity_update[next_inactive_entity] = entity_star_update;
          entity_render[next_inactive_entity] = entity_star_render;
          entity_state[next_inactive_entity] = Fixed;
          break;
        case Extrastar:
          entity_update[next_inactive_entity] = entity_extrastar_update;
          entity_render[next_inactive_entity] = entity_star_render;
          entity_state[next_inactive_entity] = Fixed;
          break;
        case Blob:
          entity_update[next_inactive_entity] = entity_blob_update;
          entity_render[next_inactive_entity] = entity_blob_render;
          entity_arg[next_inactive_entity] = *current_level_ptr++;
          entity_state[next_inactive_entity] = MoveRight;
          break;
        case Spike:
          entity_update[next_inactive_entity] = entity_spike_update;
          entity_render[next_inactive_entity] = entity_spike_render;
          entity_arg[next_inactive_entity] = *current_level_ptr++;
          entity_state[next_inactive_entity] = MoveRight;
          break;
        case Mapgoal:
          entity_update[next_inactive_entity] = entity_mapgoal_update;
          entity_render[next_inactive_entity] = entity_mapgoal_render;
          entity_state[next_inactive_entity] = Fixed;
          break;
        }
      } else {
        // entity overflow, just drop (shouldn't happen usually)
        temp = *current_level_ptr++;
        *current_level_ptr++; // drop row
        switch(temp) {
        case Star:
        case Extrastar:
        case Mapgoal:
          break;
        case Blob:
        case Spike:
          *current_level_ptr++; // drop arg
          break;
        }
      }
      j--;
    }
    load_column_state = 0xff;
  }
}

const unsigned int collision_row_mask[] =
  {
   0b0100000000000000,
   0b0010000000000000,
   0b0001000000000000,
   0b0000100000000000,
   0b0000010000000000,
   0b0000001000000000,
   0b0000000100000000,
   0b0000000010000000,
   0b0000000001000000,
   0b0000000000100000,
   0b0000000000010000,
   0b0000000000001000,
   0b0000000000000100,
   0b0000000000000010,
   0b0000000000000001
  };

unsigned char __fastcall__ player_bg_collide(signed char dx, signed char dy) {
  temp_x = ((((unsigned int) INT(temp_int_x)) + dx) & 0x1ff) >> 4;
  temp_y = (((unsigned int) INT(temp_int_y) + dy) & 0xff) >> 4;

  return (collision_mask[temp_x] & collision_row_mask[temp_y]) != 0;
}

void start_dying (void) {
  player_direction = Up;
  death_counter = 0x20;
}

void start_victory (void) {
  victory_lap = 1;
}
