for i in {1..40}; do
    bin/matrixmult $i >> out/stats
done
gnuplot scripts/plotscript.gp
