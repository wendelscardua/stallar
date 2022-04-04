#ifndef _ASSETS_SPRITES_H_
#define _ASSETS_SPRITES_H_

typedef enum
  {
   MSPlayer = 0,
   MSStar = 4,
   MSSpike = 5,
   MSBlob = 9,
   MSGoaly = 13
  } metasprite_index_t;

extern const unsigned char* const metasprite_list[];

#endif
