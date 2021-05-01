set term png
set output "out/graph.png"
set xlabel "Number of threads"
set ylabel "Time in microseconds"
plot "out/stats" using 1:2 title 'pthreads' with lines,\
     "out/stats" using 1:3 title 'mythreads' with lines,\
     "out/stats" using 1:4 title 'sequential' with lines
set term png
set output "out/matrix_mult_context.png"
set xlabel "Number of threads"
set ylabel "Context switch time in microseconds"
plot "out/stats" using 1:4 title 'mythreads' with lines,\
     "out/stats" using 1:5 title 'pthreads' with lines,\
