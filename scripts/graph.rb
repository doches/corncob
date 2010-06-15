
clean = false
if ARGV.include?("-c")
	clean = true
	ARGV.reject! { |x| x == "-c" }
end
dir = ARGV[0]

rocco = "/home/s0897549/models/rocco/bin/rocco_label"
target = ARGV[1].nil? ? "/home/s0897549/models/rocco/data/target_words" : ARGV[1]
mcrae = ARGV[2].nil? ? "/home/s0897549/models/rocco/eval/mcrae.cluster" : ARGV[2]

clusters = []

Dir.foreach(dir) do |file|
	if file =~ /.*\.([0-9]+)\..*probabilities$/
		out = File.join(dir,"#{$1}.cluster")
		if not File.exists?(out) or clean
			STDERR.puts "Labeling #{$1}..."
			`#{rocco} #{File.join(dir,file)} #{target} > #{out}`
		end
		score = File.join(dir,"#{$1}.score")
		if not File.exists?(score) or clean
			STDERR.puts "Scoring #{$1}..."
			`clusterval -s -g #{mcrae} -c #{out} > #{score}`
		end
		clusters.push score
	end
end

scores = clusters.map { |f| IO.readlines(f)[0].to_f }
gibbs = []
scores.each_with_index do |s,i|
	gibbs[i] = "#{i} #{s}"
end

fout = File.open(File.join(dir,"gibbs.dat"),"w")
fout.puts gibbs.join("\n")
fout.close

if not File.exists?(File.join(dir,"gibbs.plt"))
	`cp scripts/gibbs.plt #{dir}`
end
