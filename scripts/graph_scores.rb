# Make a gnuplot graph showing the score over time for *.fscore in input/ 
# matching /pattern/
#
# Usage: ruby scripts/graph_scores.rb input/ pattern output.png

input = ARGV.shift
pattern = /#{ARGV.shift}/
output = ARGV.shift
output = "#{output}.png" if not output =~ /\.png$/

scores = {}
Dir.foreach(input) do |file|
	if file =~ /fscore$/ and file =~ pattern
		components = file.split(".")
		if components.size == 4
			STDERR.puts file
			corpus = components.shift
			docindex = components.shift
			threshold = components.shift
			fscore = `cat #{File.join(input,file)}`.to_f
			scores[docindex] = fscore
		end
	end
end

fout = File.open("plot.plt","w")
fout.puts <<GNU
set terminal png
set output \"#{output}\"
set yrange [0:0.5]
set xrange [0:#{scores.size+1}]
plot 'plot.dat' using 2:xticlabels(1) with linespoints
GNU
fout.close

fout = File.open("plot.dat","w")
scores.each_pair do |lines,score|
	fout.puts "#{lines}\t#{score}"
end
fout.close

`gnuplot plot.plt`
