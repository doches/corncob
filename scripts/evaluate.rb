# Reads focw output with applied wordmap from stdin, and prints a clusterval score
# against 'gold.yaml'

clusters = {}
STDIN.readlines().map { |x| x.strip.split("\t") }.each do |pair|
    clusters[pair[1]] ||= []
    clusters[pair[1]].push pair[0]
end

fout = File.open("temp.yaml","w")
fout.puts clusters.to_yaml
fout.close

puts `clusterval -s -g gold.yaml -c temp.yaml`
