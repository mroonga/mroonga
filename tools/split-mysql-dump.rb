#!/usr/bin/env ruby

require "optparse"
require "ostruct"

options = OpenStruct.new
options.base_name = "dump"
option_parser = OptionParser.new
option_parser.on("--base-name=NAME",
                 "The base name of output paths",
                 "(#{options.base_name})") do |name|
  options.base_name = name
end
option_parser.parse!

preamble = ""
postamble = ""
contents = {}
current_content = ""
state = :in_preamble
ARGF.each_line do |line|
  case state
  when :in_preamble
    case line
    when /\A\/\*!/
      state = :in_preamble_set
      preamble << line
    else
      preamble << line
    end
  when :in_preamble_set
    case line
    when /\A\s*$/
      preamble << line
      state = :table
    else
      preamble << line
    end
  when :in_postamble
    postamble << line
  else
    target_line = line
    target_line = target_line.scrub unless target_line.valid_encoding?
    case target_line
    when /\A-- Table structure for table `(.+?)`$/
      table_name = $1
      contents[table_name] = current_content
      current_content << line
    when /\AUNLOCK TABLES;$/
      current_content << line
      current_content = ""
    when /\A\/\*!\d+ SET TIME_ZONE=/ # Heuristic. TODO: Improve this.
      state = :in_postamble
      postamble << line
    else
      current_content << line
    end
  end
end

contents.each do |table, content|
  File.open("#{options.base_name}-#{table}.sql", "w") do |output|
    output.puts(preamble)
    output.puts(content)
    output.puts(postamble)
  end
end
