lines = []

STDIN.each_line do |line|
  lines.push line.split(" ")[1]
end
  
lines.reverse!

lines.each { |line| puts line }
