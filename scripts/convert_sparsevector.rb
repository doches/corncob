input = ARGV[0]

IO.foreach(input) do |line|
	cols = line.strip.split(" ")
	puts "#{cols[0]} #{cols.size/2}\n"
	i = 1
	while i < cols.size
		puts "#{cols[i]} #{cols[i+1]}"
		i += 2
	end
end
