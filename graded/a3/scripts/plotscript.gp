set term png
set output "out/graph.png"
set xlabel "Number of threads"
set ylabel "Time in microseconds"
plot "out/stats" using 1:2 title 'pthreads' with lines,\
     "out/stats" using 1:3 title 'mythreads' with lines,\
     "out/stats" using 1:4 title 'sequential' with lines
pause -1
