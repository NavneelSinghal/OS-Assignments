set term png
set output "out/context.png"
set xlabel "Number of threads"
set ylabel "Time in microseconds"
plot "out/contextstats" using 1:2 title 'pthreads' with lines,\
     "out/contextstats" using 1:3 title 'mythreads' with lines
pause -1
