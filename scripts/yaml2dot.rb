# Takes a path to a yamlized clustering (one suitable for scoring
# by clusterval) and creates input for graphviz (sp. neato) to
# visualize the clustering.

# Input/output paths
input = ARGV.shift
clusters = YAML.load_file(input)
colors = YAML.load_file("colormap.yaml")
gold = YAML.load_file("gold.yaml")
output = input.split(".")
output.pop
output = output.join(".") + ".dot"
fscore = input.split(".")
fscore.pop
fscore = fscore.join(".") + ".fscore"
if File.exists?(fscore)
	fscore = `cat #{fscore}`.to_f
else
	fscore = "F-Score not found"
end

fout = File.open(output,"w")
fout.puts "digraph Clustering {"
fout.puts "\tlabel=\"#{fscore}\""
fout.puts "\tsize=\"12,12\""
clusters.each_pair do |cluster,list|
	list.each do |word|
		word.gsub!(/\-/,"_")
		color = colors[word] || "white"
		fout.puts "\t#{word} [shape=box,style=filled,color=\"#{color}\"];"
		fout.puts "\t#{word} -> #{cluster} [arrowhead=none];"
	end if list.size > 1
end
fout.puts "}"
