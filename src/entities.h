#ifndef _ENTITIES_H_
#define _ENTITIES_H_

// enum value = first metasprite
typedef enum
  {
   Star,
   Blob,
   Spike,
  } entity_t;

typedef enum
  {
   Inactive,
   Fixed,
   MoveLeft,
   MoveRight
  } entity_state_t;

#endif
