# Takes a filename handle (e.g. data/subjobj.target_corpus.1k) and applies the wordmap
# (<handle>.wordmap) to the focw output (<handle.focw>), printing the result to stdout.

input = ARGV.shift

output = IO.readlines("#{input}.focw").map { |x| x.strip.split(" ").map { |y| y.to_i } }
wordmap = {}
IO.readlines("#{input}.wordmap").map { |x| x.strip.split(" ") }.each do |pair|
    wordmap[pair[0].to_i] = pair[1]
end

output.each do |pair|
    puts "#{wordmap[pair[0]]}\t#{pair[1]}"
end
