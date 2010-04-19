wordmap = ARGV[0]
counts = ARGV[1]
target = ARGV[2].to_i

words = IO.readlines(wordmap).map { |x| x.strip }
counts = IO.readlines(counts).map { |x| x.strip.split(" ").map { |i| i.to_i } }

counts.each do |topic|
  best_words = []
  topic.each_with_index do |count,w_i|
    if best_words.size < target or count > best_words[target-1][1]
      best_words.push [w_i,count]
      best_words.sort! { |a,b| b[1] <=> a[1] }
      best_words.pop if best_words.size > target
    end
  end
#  p best_words
  puts best_words.map { |x| words[x[0]] }.join(" ")
end
