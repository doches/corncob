require 'yaml'

fake = ARGV.include?("--fake")
ARGV.reject! { |x| x == "--fake" }

results = ARGV.shift


assignments = IO.readlines(results).map { |x| x.strip.split(" ").map { |y| y.to_i } }

targets = IO.readlines(fake ? "scripts/fake.target_words" : "scripts/target_words").map { |x| x.strip }

wordmap = IO.readlines("#{results}.wordmap")
wordmap.each_with_index do |x,i|
	begin
		wordmap[i] = x.strip.split(" ")
	rescue ArgumentError
#		STDERR.puts $!
#		STDERR.puts "\t[#{x.strip}]"
	end
end

#wordmap = IO.readlines("#{results}.wordmap").map { |x| x.strip.split(" ") }
wordmap_t = wordmap.map { |x| [x[0].to_i,x[1]] }.reject { |x| not targets.include?(x[1]) }
wordmap = {}
wordmap_t.each { |x| wordmap[x[0]] = x[1] }

assignments.reject! { |x| not wordmap.keys.include?(x[0]) }
assignments.map! { |x| [wordmap[x[0]],x[1]] }

categories = {}
assignments.each do |word,category|
	categories[category] ||= []
	categories[category].push word
end

fout = File.open("#{results}.yaml","w")
fout.puts categories.to_yaml
fout.close

score = `clusterval -s -g scripts/#{fake ? 'fake.yaml' : 'mcrae.cluster'} -c #{results}.yaml`

puts score
