words = IO.readlines("scripts/target_words").map { |x| x.strip }

IO.foreach(ARGV[0]) do |line|
	puts line.strip.split(" ").map { |x| words[x.to_i] }.join(" ")
end
