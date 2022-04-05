.enum entity_type
  Star
  Blob
  Spike
  Mapgoal
  Extrastar
.endenum

.include "maps/level-00.inc"
.include "maps/level-01.inc"
.include "maps/level-02.inc"
.include "maps/level-03.inc"
.include "maps/level-04.inc"
.include "maps/level-05.inc"
.include "maps/level-06.inc"

.segment "RODATA"

.export _levels, _num_levels
_levels:
.word _level_00, _level_01, _level_02, _level_03, _level_04, _level_05, _level_06
_num_levels: .byte (* - _levels) / 2
