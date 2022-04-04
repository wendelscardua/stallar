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
      metatiles.each_slice(columns).to_a.transpose.each do |column|
        bytes = column.map { |byte| fmt(byte) }.join(', ')
        f.puts '; column data'
        f.puts ".byte #{bytes}"
        f.puts '; entities on column'
        f.puts '.byte $00'
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
