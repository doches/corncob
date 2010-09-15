# Looks for fscores in corpora/* and outputs a LaTeX table of scores for
# each found corpus

def shorten(f)
	return nil if f.nil?
	(f * 1000).to_i / 1000.0
end

puts <<TEX
\\documentclass{article}

\\begin{document}
TEX

tables = {}
Dir.foreach("corpora") do |file|
	if file =~ /^(.*)\.fscore$/
		corpus,size,threshold = *$1.split(".")
		if not threshold.nil? and size != "all"
			tables[corpus] ||= {}
			tables[corpus][size] ||= {}
			tables[corpus][size][threshold.gsub("_",".").to_f] = `cat corpora/#{file}`.to_f
		end
	end
end

tables.each_pair do |corpus,sizes|
	headers = sizes.values.map { |x| x.keys }.flatten.uniq.sort
	puts "\\begin{table}[h!]"
	puts "\\begin{centering}"
	puts "\\begin{tabular}{l | #{headers.map { |x| 'c' }.join(" ")}}"
	puts " & " + headers.join(" & ") + "\\\\"
	sizes.each_pair do |size,scores|
		print size + " & "
		puts headers.map { |x| shorten(scores[x]) }.join(" & ") + "\\\\"
	end
	puts "\\end{tabular}"
	puts "\\caption{#{corpus}}"
	puts "\\end{centering}"
	puts "\\end{table}"
	puts ""
end

puts <<TEX
\\end{document}
TEX
