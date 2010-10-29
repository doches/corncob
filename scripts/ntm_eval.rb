# Takes a filename handle (e.g. data/subjobj.target_corpus.1k) and applies the wordmap
# (<handle>.wordmap) to the ntm output (<handle.ntm>), printing the result to stdout.

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

targets = IO.readlines("scripts/target_words").map { |x| x.strip }

output = IO.readlines("#{input}.ntm").map { |x| x.strip.split("\t").map { |y| y.to_i } }
wordmap = {}
encoder = Iconv.new('US-ASCII//IGNORE', 'US-ASCII')
IO.readlines(wordmap_file).map { |x| encoder.iconv(x.strip).split(" ") }.each do |pair|
    wordmap[pair[0].to_i] = pair[1]
end

clusters = {}

output.each do |pair|
	if targets.include?(wordmap[pair[0]])
		clusters[pair[1]] ||= []
		clusters[pair[1]].push wordmap[pair[0]]
	end
end

fout = File.open("#{input}.ntm.yaml","w")
fout.puts clusters.to_yaml
fout.close

puts `clusterval -s -c #{input}.ntm.yaml -g gold.yaml`
