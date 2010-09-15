# Searches <input>/* for unscored focw runs (*.focw files for which
# no corresponding *.fscore exists) and, well, scores them.
#
# <input> defaults to 'corpora/'

input = ARGV.shift || "corpora"

Dir.foreach("#{input}") do |file|
	if file =~ /^(.*)\.focw$/
		if not File.exists?(File.join("#{input}","#{$1}.fscore"))
			puts $1
			puts "Scoring #{$1}:"
			`ruby scripts/apply_wordmap.rb #{input}/#{$1} | ruby scripts/evaluate.rb > #{input}/#{$1}.fscore`
			`mv temp.yaml #{input}/#{$1}.yaml`
			print "\t"
			puts `cat #{input}/#{$1}.fscore`
		end
	end
end