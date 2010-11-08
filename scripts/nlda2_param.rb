# Do genetic algorithms exploration of parameter space for nlda2

class Instance
	MaxWindow = 10
	MaxAlpha = 10
	MaxBeta = 10
	attr_accessor :alpha, :beta, :gamma, :window, :score
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
		@window = rand < 0.5 ? 0 : 1
		@alpha += rand * 0.2
		if @alpha < 0.1
			@alpha = 0.1+rand*0.2
		elsif @alpha > MaxAlpha
			@alpha = MaxAlpha - rand*0.2
		end
		@beta += rand * 0.2
		if @beta < 0.1
			@beta = 0.1+rand*0.2
		elsif @beta > MaxBeta
			@beta = MaxBeta - rand*0.2
		end
		@gamma += rand * 0.2
		if @gamma < 0.0
			@gamma = rand*0.2
		elsif @gamma > 1.0
			@gamma = 1.0-rand*0.2
		end
	end
	
	def breed_with(b)
		Instance.breed(self,b)
	end

	def outfile
		"#{@@input.split('/').pop}.results/#{@alpha}_#{@beta}_#{@gamma}_#{@window}"
	end
	
	def score
		start = Time.now
		cmd = "./nlda2 #{@alpha} #{@beta} #{@gamma} #{@@input} #{self.outfile} #{@window}"
		`#{cmd}`
		@elapsed = Time.now - start
		s = `ruby scripts/nlda2_eval.rb #{self.outfile}`.to_f 
		puts "[#{@alpha} #{@beta} #{@gamma} #{@window}]:\t#{s}"
		return s
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
		alpha = rand*MaxAlpha
		beta = rand*MaxBeta
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
		@input = input
		
		Instance.input = input
		begin
			`mkdir #{Instance.input.split('/').pop}.results`
			report = `head -n 5 #{Instance.input.split('/').pop}.results/report`.split("\n").reject { |l| not l =~ /<([^>]+)>/ }.pop
			report =~ /<([^>]+)>/
			params = $1.split(" ").map { |x| x.to_f }
			@population.clear
			@population = (1..popsize).map { |i| Instance.new(*params) }
			@population.each_with_index { |i,dex| i.mutate!(1.0) if dex > 1 }
			STDERR.puts "Resuming from previous trial..."
		rescue
			STDERR.puts $!
		ensure
			`rm #{Instance.input.split('/').pop}.results/*_*`
		end
		
		`mkdir #{Instance.input.split('/').pop}.results/` if not File.exists?("#{Instance.input.split('/').pop}.results/")
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
		@population.clear
		[0,1].each { |i| @population.push scores[i][0] }
		(@popsize-2-@mutants).times { @population.push Instance.breed(@population[0],@population[1]) }
		@population.each_with_index { |i,c| i.mutate!(0.33) if c > 1 }
		@mutants.times { @population.push Instance.random }
		
		# Report
		STDERR.puts "#{scores[0][0].to_s} / #{scores[0][1]}"
		STDERR.puts ""
		fout = File.open("#{Instance.input.split('/').pop}.results/report","a+")
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

