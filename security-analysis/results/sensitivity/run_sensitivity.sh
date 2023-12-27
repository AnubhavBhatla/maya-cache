## Fire short runs for generating Bucket-Probability Distribution.
echo "Running Sensitivity: Buckets & Balls Simulation for WaysPerSkew=3,12"
NUM_BILLIONS=10
echo "(Throwing ${NUM_BILLIONS} Billion Balls)"
mkdir -p raw_results;
rm -rf raw_results/*;
rm -rf Base*.BucketProb.stat;

#LLC-Associativity=36
stdbuf -oL ../../bin/maya12Ways.o 6 ${NUM_BILLIONS} 42 \
       > raw_results/Base12.ExtraWays6.out &
#LLC-Associativity=8
stdbuf -oL ../../bin/maya3Ways.o 6 ${NUM_BILLIONS} 42 \
       > raw_results/Base3.ExtraWays6.out &

## Run Empirical Results for Reuse-Ways = 1,5,7
for reuse_ways_per_skew in 1 5 7
do
    stdbuf -oL ../../bin/maya6Ways_${reuse_ways_per_skew}Reuse.o 6 ${NUM_BILLIONS} 42 \
               > raw_results/Base6.ReuseWays${reuse_ways_per_skew}.out &
done

## Wait for completion of all experiments.
exp_count=`ps aux | grep -i "maya" | grep -v "grep" | wc -l`
while [ $exp_count -gt 0 ]
do
    sleep 30
    exp_count=`ps aux | grep -i "maya" | grep -v "grep" | wc -l`    
done

##Generate Bucket_Probabilities
./get_bucket_prob_12wps.sh
./get_bucket_prob_3wps.sh
./get_bucket_prob_1reuse.sh
./get_bucket_prob_5reuse.sh
./get_bucket_prob_7reuse.sh
