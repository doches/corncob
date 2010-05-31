require 'yaml'
assignments = IO.readlines(ARGV.shift).map { |x| x.strip.split(" ").map { |y| y.to_i } }

targets = IO.readlines("scripts/target_words").map { |x| x.strip }

wordmap = IO.readlines("nlda2_wordmap").map { |x| x.strip.split(" ") }
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

puts categories.to_yaml