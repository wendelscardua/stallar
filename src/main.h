#ifndef _MAIN_H_
#define _MAIN_H_

typedef enum {
              Title,
              Main,
              GameOver
} game_state_t;

#pragma bss-name(push, "ZEROPAGE")
extern game_state_t current_game_state;
#pragma zpsym("current_game_state")
#pragma bss-name(pop)

#define BG_MAIN_0 0
#define BG_MAIN_1 1
#define BG_MAIN_2 2
#define BG_MAIN_3 3
#define SPRITE_0 4
#define SPRITE_1 6

#include "title-game-state.h"
#include "main-game-state.h"
#include "game-over-game-state.h"

/* fedc ba98 7654 3210
 * f: 0 for left nametable, 1 for right nametable
 * e..7: integer pixel
 * 6..0: subpixel
 */

// converts 3 uint8 into a single uint16
#define FP(screen,integer,fraction) (((screen) << 15)|((integer)<<7)|((fraction)>>2))

// extract the integer part (including screen)
// TODO: round insted of truncate?
#define INT(unsigned_fixed_point) ((unsigned_fixed_point>>7)&0xff)

#endif
