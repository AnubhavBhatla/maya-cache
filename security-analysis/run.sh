#!/usr/bin/env bash

cd results;

NUM_ITERATIONS=1000

echo "Running Security Simulation for 6-Ways"
mkdir -p raw_results;
rm -rf raw_results/*;

for extra_ways_per_skew in 1 2 3 4 5 6
do
	stdbuf -oL ../bin/maya6Ways.o $extra_ways_per_skew $NUM_ITERATIONS 1 \
               > raw_results/maya6ways.${extra_ways_per_skew}extraways.out &
done

exp_count=`ps aux | grep -i "maya6Ways.o" | grep -v "grep" | wc -l`
while [ $exp_count -gt 0 ]
do
    sleep 30
    exp_count=`ps aux | grep -i "maya6Ways.o" | grep -v "grep" | wc -l`    
done


## Generate the Spills Result (Fig-7)
./get_spills.sh

## Generate the Bucket Probabilities Result (Fig-9, Fig-10)
./get_bucket_prob.sh


