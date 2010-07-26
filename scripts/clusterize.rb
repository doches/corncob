require 'yaml'

wordmap = ARGV.size > 1 ? ARGV.shift : "ocw.wordmap"

words = IO.readlines(wordmap).map { |x| x.strip.split(" ") }.map { |pair| [pair[0].to_i,pair[1]] }
wordmap = {}
words.each { |pair| wordmap[pair[0]] = pair[1] }

lines = STDIN.readlines.map { |x| x.strip.split(" ").map { |y| y.to_i }}
clusters = {}
lines.each do |pair|
  clusters[pair[1]] ||= []
  clusters[pair[1]].push wordmap[pair[0]]
end
puts clusters.to_yaml
