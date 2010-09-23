# Searches <input>/* for unscored focw runs (*.focw files for which
# no corresponding *.fscore exists) and, well, scores them.
#
# <input> defaults to 'corpora/'

input = ARGV.shift || "corpora"
pattern = ARGV.empty? ? /.*/ : /#{ARGV.shift}/

Dir.foreach("#{input}") do |file|
	if file =~ pattern and file =~ /^(.*)\.focw$/
		if not File.exists?(File.join("#{input}","#{$1}.fscore"))
			puts $1
			STDERR.print "Scoring #{$1}: "
			`ruby scripts/apply_wordmap.rb #{input}/#{$1} | ruby scripts/evaluate.rb > #{input}/#{$1}.fscore`
			`mv temp.yaml #{input}/#{$1}.yaml`
			STDERR.print "\t"
			STDERR.puts `cat #{input}/#{$1}.fscore`
		end
	end
end
