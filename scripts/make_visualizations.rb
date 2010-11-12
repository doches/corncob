# Looks for all *.yaml files in <input> and produces graph visualizations
# of them in <output>
#
# Usage> ruby __FILE__ input/ output/

input = ARGV.shift
output = ARGV.shift
pattern = ARGV.empty? ? false : ARGV.shift
STDERR.puts "Rendering #{File.join(input,pattern || '*')} into #{output}"
pattern = /#{pattern}/ if pattern
`mkdir #{output}` if not File.exists?(output)
Dir.foreach(input) do |file|
	if file =~ /^(.*)\.yaml$/
		key = $1
		if not File.exists?("#{output}/#{key}.png") and ((pattern and file =~ pattern) or not pattern)
			`ruby scripts/yaml2dot.rb #{input}/#{file} && neato -Tpng #{input}/#{key}.dot -o #{output}/#{key}.png`
		end
	end
end
