gold = YAML.load_file("gold.yaml")
x = %w{coral1 coral4 lavender ivory1 ivory4 lightgray sienna1 palegoldenrod lawngreen mintcream orangered pink beige chocolate orange gold yellow yellowgreen green limegreen aquamarine cyan2 turquoise2 aliceblue blue1 blue4 indigo lightblue2 mediumblue navy slateblue blueviolet magenta1 orchid1 orchid4 violetred1 violetred4}
colors = {}
gold.keys.each { |c| colors[c] = x.pop }

words = {}
gold.each_pair do |key,wordlist|
	wordlist.each do |word|
		words[word] = colors[key]
	end
end

puts words.to_yaml	
