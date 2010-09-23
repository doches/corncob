# Takes a filename handle (e.g. data/subjobj.target_corpus.1k) and applies the wordmap
# (<handle>.wordmap) to the focw output (<handle.focw>), printing the result to stdout.

require 'iconv'

input = ARGV.shift
wordmap_file = "#{input}.wordmap"
wordmap_file_com = input.split(".")
while not File.exists?(wordmap_file) and wordmap_file.size > 0
#	STDERR.puts "#{wordmap_file} not found"
	len = wordmap_file.size
	wordmap_file_com.pop
	wordmap_file = wordmap_file_com.join(".") + ".wordmap"
	break if wordmap_file.size == len
end

if wordmap_file.size <= 1 or not File.exists?(wordmap_file)
	STDERR.puts "No reasonable wordmap found, dying"
	exit(1)
end
STDERR.puts "Using #{wordmap_file}"

output = IO.readlines("#{input}.focw").map { |x| x.strip.split(" ").map { |y| y.to_i } }
wordmap = {}
encoder = Iconv.new('US-ASCII//IGNORE', 'US-ASCII')
IO.readlines(wordmap_file).map { |x| encoder.iconv(x.strip).split(" ") }.each do |pair|
    wordmap[pair[0].to_i] = pair[1]
end

output.each do |pair|
    puts "#{wordmap[pair[0]]}\t#{pair[1]}"
end
