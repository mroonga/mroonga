#!/usr/bin/env ruby

if ARGV.size < 1
  puts("Usage: #{$0} TARGET_SQL TRACE_LOG_PATH")
  exit(false)
end

target_sql = ARGV.shift
start_pattern = /\AT@\d+ *: \| \| query: #{Regexp.escape(target_sql)}/
in_target = false
ARGF.each_line do |line|
  next unless line.valid_encoding?
  if in_target
    puts(line)
    case line
    when /\AT@\d+ *: \| <dispatch_command$/
      in_target = false
    end
  else
    case line
    when start_pattern
      in_target = true
      puts(line)
    end
  end
end
