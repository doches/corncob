# Do genetic algorithms exploration of parameter space for nlda2

require '../../clusterfuck/lib/clusterfuck'

class Instance
	MaxWindow = 5
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
		end
		@beta += rand * 0.2
		if @beta < 0.1
			@beta = 0.1+rand*0.2
		elsif @beta > 1.0
			@beta = 1.0 - rand*0.2
		end
		@gamma += rand * 0.2
		if @gamma < 0.0
			@gamma = rand*0.2
		elsif @gamma > 1.0
			@gamma = 1.0-rand*0.2
		end
=begin
		gene = rand
		if gene < 0.33
			old_alpha = @alpha
			@alpha += (rand) * (rand < 0.5 ? -1 : 1)
			@alpha = old_alpha+rand if @alpha < 0.0
		elsif gene < 0.66
			@beta += (rand - 0.5)*0.1
			if @beta < 0.1
				@beta = 0.1+rand*0.2
			elsif @beta > 1.0
				@beta = 1.0-rand*0.2
			end
		elsif gene < 0.1
			@gamma += (rand) * (rand < 0.5 ? -1 : 1)
			@gamma = 0.001 if @gamma < 0.0
		else
			@window += (1+rand*5).to_i * (rand < 0.5 ? -1 : 1)
			@window = 0 if @window < 0.0
		end
=end
	end
	
	def breed_with(b)
		Instance.breed(self,b)
	end

	def outfile
		"#{@@input}.results/#{@alpha}_#{@beta}_#{@gamma}_#{@window}"
	end
	
	def job
		"cd models/corncob; nice ./nlda2 #{@alpha} #{@beta} #{@gamma} #{@@input} #{self.outfile} #{@window};  ruby scripts/nlda2_eval.rb #{self.outfile}"
	end
	
	def score
		start = Time.now
		`./nlda2 #{@alpha} #{@beta} #{@gamma} #{@@input} #{self.outfile} #{@window}`
		@elapsed = Time.now - start
		return `ruby scripts/nlda2_eval.rb #{self.outfile}`.to_f 
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
<<<<<<< HEAD
		beta = rand*0.8+0.2
=======
		beta = rand*0.95+0.5
>>>>>>> .
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
		@hosts = IO.readlines("scripts/hosts.#{@input}").map { |x| x.strip }
	end
	
	def simulate_generation
		start_time = Time.now
=begin		
		# Compute scores
		jobs = @population.map { |instance| instance.job }
		clusterfile = File.open("nlda2_params.clusterfile","w")
#	task.jobs = #{@population.map { |i| Clusterfuck::Job.new(i.outfile.split("/").pop,i.job) }.inspect}
#	task.jobnames = #{@population.map { |i| i.outfile.split("/").pop}.inspect}
		clusterfile.puts <<CF
Clusterfuck::Task.new do |task|
	task.hosts = @hosts
	task.jobs = [#{@population.map { |i| "Clusterfuck::Job.new('#{i.outfile.split('/').pop}','#{i.job} 2> /dev/null')" }.join(", ")}]
	task.temp = "fragments"
	task.username = "s0897549"
	task.password = "d()ches42"
	task.showreport = false
	task.verbose=Clusterfuck::VERBOSE_ALL
end
CF
		clusterfile.close
		`clusterfuck nlda2_params.clusterfile`

		# Rename fragment files
		cmds = []
		cwd = "fragments"
		Dir.foreach(cwd) do |f|
			if f =~ /^(\d\.\d+)_(\d\.\d+)_(\d\.\d+)_(\d+)/
				cmds.push "mv #{cwd}/#{f} #{cwd}/#{$1}_#{$2}_#{$3}_#{$4}.score 2> /dev/null"
			end
		end
		cmds.each { |cmd| `#{cmd}` }
		# Load scores
		i = 0
		@population.map do |instance|
			begin
				instance.score = IO.readlines("fragments/#{instance.outfile.split("/").pop}.score").join("").strip.to_f
			rescue
				instance.score = 0.0
				begin
					@hosts.delete_at i
				rescue
					STDERR.puts "Could not delete host"
				end
				STDERR.puts $!
			end
			i += 1
		end
		`rm fragments/*`
=end
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
	pool = World.new(ARGV.shift,6,3)
	100.times { pool.simulate_generation }
	puts "-----------------------------------"
	puts "Best result after #{pool.generation-1} generations:"
	puts "Parameters: #{pool.best[0].to_s}"
	puts "Score:      #{pool.best[1]}"
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
