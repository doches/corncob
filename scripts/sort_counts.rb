STDIN.each_line do |line|
  cols = line.strip.split(" ").map { |x| x.split("_") }.sort { |a,b| a[0] <=> b[0] }
  puts cols.map { |x| x[1] }.join(" ")
end
