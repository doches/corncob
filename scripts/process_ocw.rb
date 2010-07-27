# Build yaml cluster from ocw/target_corpus.wordmap
require 'yaml'

input = ARGV.shift
wordmap = "#{input.gsub('.ocw','')}.target_corpus.wordmap"

words = IO.readlines(wordmap).map { |x| x.strip.split(" ") }.map { |pair| [pair[0].to_i,pair[1]] }
wordmap = {}
words.each { |pair| wordmap[pair[0]] = pair[1] }

lines = IO.readlines(input).map { |x| x.strip.split(" ").map { |y| y.to_i }}
clusters = {}
lines.each do |pair|
  clusters[pair[1]] ||= []
  clusters[pair[1]].push wordmap[pair[0]]
end
fout = File.open("#{input.gsub('.ocw','.yaml')}","w")
fout.puts clusters.to_yaml
fout.close
puts `clusterval -s -g gold.yaml -c #{input.gsub('.ocw','.yaml')}`
