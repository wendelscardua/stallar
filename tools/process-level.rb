#!/usr/bin/ruby
# frozen_string_literal: true

# converts a .tmx map into a .s
# inputs:
# - .tmx filename
# - .s filename

require 'bundler/inline'

gemfile do
  source 'https://rubygems.org'

  gem 'nokogiri'
  gem 'pry'
end

require 'json'

class TmxReader
  attr_reader :document, :tmx_file, :s_file

  def initialize(tmx_file, s_file)
    @tmx_file = tmx_file
    @s_file = s_file
    @document = Nokogiri::XML(File.read(tmx_file))
  end

  def process
    level_label = labelify(tmx_file.gsub('.tmx', '').gsub(/.*\//, ''))

    objects = document.xpath('//objectgroup/object')

    entities = objects.map do |object|
      {
        type: object['type'].capitalize,
        meta_column: numberify(object['x']) / 16,
        meta_row: numberify(object['y']) / 16
      }.tap do |entity|
        case object['type']
        when 'star'
          entity[:arg] = 0
        when 'blob', 'spike'
          entity[:arg] = (numberify(object['x']) + numberify(object['width'])) / 16 - entity[:meta_column]
          entity[:meta_column] += entity[:arg]
        end
      end
    end

    metatiles = document.xpath('//layer/data')
                        .text
                        .scan(/\d+/)
                        .map { |t| t.to_i - 1 }
    columns = metatiles.count / 15
    File.open(s_file, 'w') do |f|
      f.puts <<~"PREAMBLE"
        .segment "RODATA"
        .export _#{level_label}
        _#{level_label}:
        .byte #{fmt(columns)} ; num columns
      PREAMBLE
      metatiles.each_slice(columns).to_a.transpose.each.with_index do |column, column_index|
        bytes = column.map { |byte| fmt(byte) }.join(', ')
        f.puts '; column data'
        f.puts ".byte #{bytes}"

        f.puts '; entities on column'
        column_entities = entities.select { |entity| entity[:meta_column] == column_index }
        f.puts ".byte #{fmt(column_entities.count)}"
        column_entities.each.with_index do |entity, index|
          f.puts "; entity #{index + 1}"
          f.puts ".byte entity_type::#{entity[:type]}"
          f.puts ".byte #{fmt(entity[:meta_row])}, #{fmt(entity[:arg])}"
        end
        f.puts '; end of entities on column'
      end
    end
  end

  private

  def labelify(tmx_name)
    tmx_name.tr('-.', '__')
  end

  def numberify(coordinate)
    coordinate.to_f.round.to_i
  end

  def coalesce(coordinate)
    (coordinate + 4) & ~0b111
  end

  def fmt(byte)
    format('$%02x', byte)
  end
end

tmx_file, s_file = ARGV

tmx_reader = TmxReader.new(tmx_file, s_file)
tmx_reader.process
