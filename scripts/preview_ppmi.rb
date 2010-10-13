# Read a .ppmi file output by focw and sort each word's vector according to importance

IO.foreach(ARGV.shift) do |line|
	line.strip!
	# harpsichord <set (4.712248), ian (7.084677), that (4.875701), here (5.523178), >
	word,line = *line.split(" ",2)
	line = line.slice(1,line.size-4)
	features = line.split(", ")
	features.map! { |x| 
		x =~ /(\w+)\s\(([0-9\.]+)\)/
		[$1,($2.include?(".") ? $2.to_f : $2.to_i)]
	}
	features.sort! { |a,b| b[1] <=> a[1] }
	features = features[0..19] if features.size > 20
	
	print "#{word} <"
	print features.map { |x| "#{x[0]} (#{x[1]})" }.join(", ")
	puts ">"
end
