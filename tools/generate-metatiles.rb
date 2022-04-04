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
    .export _metatiles
    _metatiles:
  PREAMBLE
  (0..50).each do |metatile_index|
    meta_row = metatile_index / 8
    meta_col = metatile_index % 8
    attribute = bytes[256 + (meta_row / 2) * 4 + (meta_col / 2)]
    attribute >>= 4 if meta_row % 2 == 1
    attribute >>= 2 if meta_col % 2 == 1
    attribute &= 0b11

    metabytes = [
      bytes[0x20 * meta_row + 0x2 * meta_col],
      bytes[0x20 * meta_row + 0x2 * meta_col + 0x1],
      bytes[0x20 * meta_row + 0x10 + 0x2 * meta_col],
      bytes[0x20 * meta_row + 0x10 + 0x2 * meta_col + 0x1],
      attribute
    ].map { |b| '$%02x' % b }.join(', ')
    f.puts ".byte #{metabytes}"
  end
end
