#!/usr/bin/ruby
# frozen_string_literal: true

# converts a .map metatile into a .s
# inputs:
# - .map filename
# - .s filename

map_file, s_file = ARGV

bytes = File.open(map_file, 'rb', &:read).unpack('C*')

File.open(s_file, 'w') do |f|
  f.puts <<~PREAMBLE
    .segment "RODATA"
    .export _metatiles_ul, _metatiles_ur, _metatiles_dl, _metatiles_dr, _metatiles_attr
    _metatiles:
  PREAMBLE

  (0..50).map do |metatile_index|
    meta_row = metatile_index / 8
    meta_col = metatile_index % 8
    attribute = bytes[256 + (meta_row / 2) * 4 + (meta_col / 2)]
    attribute >>= 4 if meta_row.odd?
    attribute >>= 2 if meta_col.odd?
    attribute &= 0b11

    [
      bytes[0x20 * meta_row + 0x2 * meta_col],
      bytes[0x20 * meta_row + 0x2 * meta_col + 0x1],
      bytes[0x20 * meta_row + 0x10 + 0x2 * meta_col],
      bytes[0x20 * meta_row + 0x10 + 0x2 * meta_col + 0x1],
      attribute
    ]
  end.transpose
     .map { |metabytes| metabytes.map { |b| '$%02x' % b }.join(', ') }
     .zip(%w[ul ur dl dr attr])
     .each { |metabytes, type| f.puts "_metatiles_#{type}: .byte #{metabytes}" }
end
