#!/usr/bin/env bash

echo "ExtraWaysPerSkew Trials Spills Trials/Spill" > maya6ways.Spills.stat;

for extra_ways_per_skew in 1 2 3 4 5 6 ; do
    iterations=`sed -n 's/.*BALL_THROWS:\(.*\),.*/\1/p' raw_results/maya6ways.${extra_ways_per_skew}extraways.out` ;

    grep -h "Spill Count:"  raw_results/maya6ways.${extra_ways_per_skew}extraways.* | awk -F ':' '{print $2}' | \
    awk -v iter=$iterations -v extra=$extra_ways_per_skew \
        '{sum+=$1; iters+=iter/2}END{if(sum){iters_per_sum=iters/sum}else{iters_per_sum=0};print extra, iters,sum,iters_per_sum}' \
        >> maya6ways.Spills.stat;
done

column -t maya6ways.Spills.stat > temp ; mv temp maya6ways.Spills.stat ;
