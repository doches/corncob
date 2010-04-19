counts = ARGV[0].to_i

`cat bnc.wordmap | sort -r | ruby scripts/sort_wordmap.rb > bnc.wordmap.final`
`cat bnc.#{counts}.counts | ruby scripts/sort_counts.rb > bnc.#{counts}.counts.final`
puts `ruby scripts/get_words.rb bnc.wordmap.final bnc.#{counts}.counts.final 10`
