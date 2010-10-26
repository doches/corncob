# Archive an focw run in focw_runs
#
# Usage: ruby #{__FILE__} corpus method

corpus = ARGV.shift
method = ARGV.shift
output= "focw_runs/#{corpus}.#{method}.#{Time.now.day}#{Time.now.month}#{Time.now.year}"
STDERR.puts "Archiving #{corpus} #{method} in\n\t#{output}"

`mkdir #{output}`
`mkdir #{output}/corpora/`
`mv corpora/#{corpus}.*.#{method}.* #{output}/corpora/`
`mv #{corpus}.#{method}.* #{output}`
