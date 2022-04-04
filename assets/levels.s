.enum entity_type
  Star
  Blob
  Spike
.endenum

.include "maps/level-00.inc"
.include "maps/level-01.inc"
.include "maps/level-02.inc"

.segment "RODATA"

.export _levels, _num_levels
_levels:
.word _level_00, _level_01, _level_02
_num_levels: .byte (* - _levels) / 2
