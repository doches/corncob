prefix = ARGV[0]

words_f = IO.readlines("#{prefix}.wordmap").map { |x| x.strip.split(" ") }
words = {}
words_f.each { |pair| words[pair[0].to_i] = pair[1] }

counts = IO.readlines("#{prefix}.counts").map { |x| x.strip.split(" ").map { |x| x.to_i } }
counts.each_with_index { |x,x_i| x.each_with_index { |w_c, w_i| counts[x_i][w_i] = [w_c,w_i] } }
counts.map! { |x| x.sort { |a,b| b[0] <=> a[0] } }
counts.each do |category|
	p category[0..9].map { |x| words[x[1]] }
end
