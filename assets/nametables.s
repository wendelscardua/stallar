.segment "RODATA"

.export _empty_nametable
_empty_nametable: .incbin "nametables/empty.rle"

.export _title_nametable
_title_nametable: .incbin "nametables/title.rle"

.export _game_over_nametable
_game_over_nametable: .incbin "nametables/game-over.rle"
