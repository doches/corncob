# Do genetic algorithms exploration of parameter space for nlda2

class Instance
	MaxWindow = 30
	attr_accessor :alpha, :beta, :gamma, :window
	attr_reader :elapsed
	
	@@input=nil
	
	def Instance.input=(path)
		@@input = path
	end
	
	def Instance.input
		return @@input
	end
	
	def initialize(a,b,g,w)
		@alpha = a
		@beta = b
		@gamma = g
		@window = w
	end
	
	def mutate!(rate)
		gene = rand
		if gene < 0.25
			@alpha += (rand) * (rand < 0.5 ? -1 : 1)
			@alpha += rand if @alpha < 0.0
		elsif gene < 0.5
			@beta += (rand) * (rand < 0.5 ? -1 : 1)
			@beta = rand if (@beta < 0.0 or @beta > 1.0)
		elsif gene < 0.75
			@gamma += (rand) * (rand < 0.5 ? -1 : 1)
			@gamma = 0.001 if @gamma < 0.0
		else
			@window += (1+rand*5).to_i * (rand < 0.5 ? -1 : 1)
			@window = 0 if @window < 0.0
		end
	end
	
	def breed_with(b)
		Instance.breed(self,b)
	end
	
	def score
		outfile = "#{@@input}.results/#{@alpha}_#{@beta}_#{@gamma}_#{@window}"
		start = Time.now
		`./nlda2 #{@alpha} #{@beta} #{@gamma} #{@@input} #{outfile} #{@window}`
		@elapsed = Time.now - start
		return `ruby scripts/nlda2_eval.rb #{outfile}`.to_f 
	end
	
	def to_s
		"<#{@alpha} #{@beta} #{@gamma} #{@window}>"
	end
	
	def Instance.breed(a,b)
		Instance.new(rand < 0.5 ? a.alpha : b.alpha, 
		             rand < 0.5 ? a.beta : b.beta,
		             rand < 0.5 ? a.gamma : b.gamma,
		             rand < 0.5 ? a.window : b.window)
	end
	
	def Instance.random
		alpha = rand*10
		beta = rand
		gamma = rand
		window = (rand*MaxWindow).to_i
		
		return Instance.new(alpha, beta, gamma, window)
	end
end

class World
	attr_reader :generation, :best
	
	def initialize(input,popsize,mutants)
		@population = (1..popsize).map { |i| Instance.random }
		@generation = 1
		@mutants = mutants
		@popsize = popsize
		@best = [nil,0]
		
		Instance.input = input
		begin
			report = `head -n 5 #{Instance.input}.results/report`.split("\n").reject { |l| not l =~ /<([^>]+)>/ }.pop
			report =~ /<([^>]+)>/
			params = $1.split(" ").map { |x| x.to_f }
			@population.clear
			@population = (1..popsize).map { |i| Instance.new(*params) }
			@population.each_with_index { |i,dex| i.mutate!(1.0) if dex > 1 }
			STDERR.puts "Resuming from previous trial..."
		rescue
			STDERR.puts $!
		ensure
			`rm #{Instance.input}.results/*`
		end
		
		`mkdir #{Instance.input}.results/` if not File.exists?("#{Instance.input}.results/")
	end
	
	def simulate_generation
		start_time = Time.now
		scores = @population.map { |instance| [instance, instance.score] }.sort { |a,b| b[1] <=> a[1] }	
		elapsed = Time.now - start_time
		STDERR.puts ""
		STDERR.puts "#### Generation #{@generation} ####"
		STDERR.puts "Elapsed: #{elapsed}"
		scores.each { |i| STDERR.puts "#{i[0].to_s} \t#{i[1]}\t(#{i[0].elapsed}s)" }
		STDERR.puts "-------------------------------"
		@population.clear
		if scores[0][1] > @best[1]
			@best = scores[0].dup
		else
			@population.push @best[0]
		end
		[0,1].each { |i| @population.push scores[i][0] }
		(@popsize-2-@mutants).times { @population.push Instance.breed(@population[0],@population[1]) }
		@population.each_with_index { |i,c| i.mutate!(0.33) if c > 1 }
		@mutants.times { @population.push Instance.random }
		
		# Report
		STDERR.puts "#{scores[0][0].to_s} / #{scores[0][1]}"
		STDERR.puts ""
		fout = File.open("#{Instance.input}.results/report","a+")
		fout.puts "##### Generation #{@generation} #####"
		fout.puts "Elapsed: #{elapsed}"
		fout.puts "Params:  #{@best[0].to_s}"
		fout.puts "Score:   #{@best[1]}"
		fout.puts ""
		fout.close
		
		@generation += 1
	end
end

if __FILE__ == $0
	pool = World.new(ARGV.shift,10,3)
	100.times { pool.simulate_generation }
	puts "-----------------------------------"
	puts "Best result after #{pool.generation-1} generations:"
	puts "Parameters: #{pool.best[0].to_s}"
	puts "Score:      #{pool.best[1]}"
end
