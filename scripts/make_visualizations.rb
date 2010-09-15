# Looks for all *.yaml files in <input> and produces graph visualizations
# of them in <output>
#
# Usage> ruby __FILE__ input/ output/

input = ARGV.shift
output = ARGV.shift
Dir.foreach(input) do |file|
	if file =~ /^(.*)\.yaml$/
		if not File.exists?("#{output}/#{$1}.png")
			STDERR.puts file
			`ruby scripts/yaml2dot.rb #{input}/#{file} && neato -Tpng #{input}/#{$1}.dot -o #{output}/#{$1}.png`
		end
	end
end
