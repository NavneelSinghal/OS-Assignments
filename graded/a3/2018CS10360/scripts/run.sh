rm -f out/stats
rm -f out/contextstats
touch out/stats
touch out/contextstats
for i in {1..40}; do
    bin/matrixmult $i >> out/stats
done
gnuplot scripts/plotscript.gp
for i in {1..40}; do
    bin/measurecontext $i >> out/contextstats
done
gnuplot scripts/plotscript_context.gp
