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
#define FP(screen,integer,fraction) ((signed long)((((unsigned long)screen) << 16)|(((unsigned long)integer)<<8)|((fraction))))

// extract the integer part (including screen)
// TODO: round insted of truncate?
#define INT(unsigned_fixed_point) ((unsigned int)(((unsigned_fixed_point)>>8))&0x1ff)

// take just the msb, enough for some comparisons
#define TRUNC(unsigned_fixed_point) ((unsigned char)((unsigned_fixed_point) >> 8))

#define SCREEN(unsigned_fixed_point) ((unsigned char)(((unsigned_fixed_point) >> 16) & 0x1))
#endif
