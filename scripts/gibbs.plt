f(x) = m*x + b
fit f(x) 'gibbs.dat' via m,b
baseline(x) = 0.141825
plot baseline(x), f(x), 'gibbs.dat' with linespoints
