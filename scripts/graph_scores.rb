# Make a gnuplot graph showing the score over time for *.fscore in input/ 
# matching /pattern/
#
# Usage: ruby scripts/graph_scores.rb input/ pattern output.png

input = ARGV.shift
pattern = /#{ARGV.shift}/
output = nil
#format = "eps"
#term = "postscript enhanced color"
format = "png"
term = "png"
if not ARGV.empty?
	output = ARGV.shift
	output = "#{output}.#{format}" if not output =~ /\.#{format}$/
end

scores = {}
corpus = nil
threshold = nil
Dir.foreach(input) do |file|
	if file =~ /fscore$/ and file =~ pattern
		components = file.split(".")
		if components.size == 4
			STDERR.puts file
			_corpus = components.shift
			if not corpus.nil? and corpus != _corpus
				STDERR.puts "[warn] Multiple corpora in plot (#{corpus} and #{_corpus})"
			end
			corpus = _corpus
			docindex = components.shift.to_i
			_threshold = components.shift.gsub("_",".").to_f
			if not threshold.nil? and threshold != _threshold
				STDERR.puts "[warn] Multiple thresholds in plot (#{threshold} and #{_threshold})"
			end
			threshold = _threshold
			fscore = `cat #{File.join(input,file)}`.to_f
			scores[docindex] = fscore
		end
	end
end

output = "#{corpus}.#{threshold.to_s.gsub('.','_')}.#{format}" if output.nil?
datfile = output.gsub(".#{format}",".dat")

fout = File.open("plot.plt","w")
fout.puts <<GNU
set terminal #{term} font "scripts/Arial.ttf" 14
set output \"#{output}\"
set xtics rotate by -45
set yrange [0:1.0]
set xrange [0:#{scores.size-1}]
plot '#{datfile}' using 2:xticlabels(1) with linespoints title '#{corpus} #{threshold}'
fout.close

fout = File.open(datfile,"w")
scores.map { |l,s| [l,s] }.sort { |a,b| a[0] <=> b[0] }.each do |lines,score|
	fout.puts "#{lines}\t#{score}"
end
fout.close

viewer = 'gthumb'
viewer = 'open' if `uname`.strip == "Darwin"

`gnuplot plot.plt && #{viewer} #{output} &`
