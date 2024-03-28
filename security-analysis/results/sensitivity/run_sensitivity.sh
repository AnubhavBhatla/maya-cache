## Fire short runs for generating Bucket-Probability Distribution.
echo "Running Security Simulation for 3-ways and 12-ways"
NUM_ITERATIONS=${1:-100}
mkdir -p raw_results;
rm -rf raw_results/*;
rm -rf Base*.BucketProb.stat;

#LLC-Associativity=36
stdbuf -oL ../../bin/maya12Ways.o 6 ${NUM_ITERATIONS} 42 \
       > raw_results/maya12ways.ExtraWays6.out &
#LLC-Associativity=8
stdbuf -oL ../../bin/maya3Ways.o 6 ${NUM_ITERATIONS} 42 \
       > raw_results/maya3ways.ExtraWays6.out &

echo "Running Security Simulation for 1-way, 5-way and 7-way Reuse"

## Run Empirical Results for Reuse-Ways = 1,5,7
for reuse_ways_per_skew in 1 5 7
do
    stdbuf -oL ../../bin/maya6Ways_${reuse_ways_per_skew}Reuse.o 6 ${NUM_ITERATIONS} 42 \
               > raw_results/maya6ways.ReuseWays${reuse_ways_per_skew}.out &
done

## Wait for completion of all experiments.
exp_count=`ps aux | grep -i "maya12Ways.o" | grep -v "grep" | wc -l`
while [ $exp_count -gt 0 ]
do
    sleep 30
    exp_count=`ps aux | grep -i "maya12Ways.o" | grep -v "grep" | wc -l`    
done
exp_count=`ps aux | grep -i "maya3Ways.o" | grep -v "grep" | wc -l`
while [ $exp_count -gt 0 ]
do
    sleep 30
    exp_count=`ps aux | grep -i "maya3Ways.o" | grep -v "grep" | wc -l`    
done
exp_count=`ps aux | grep -i "maya6Ways_1Reuse.o" | grep -v "grep" | wc -l`
while [ $exp_count -gt 0 ]
do
    sleep 30
    exp_count=`ps aux | grep -i "maya6Ways_1Reuse.o" | grep -v "grep" | wc -l`    
done
exp_count=`ps aux | grep -i "maya6Ways_5Reuse.o" | grep -v "grep" | wc -l`
while [ $exp_count -gt 0 ]
do
    sleep 30
    exp_count=`ps aux | grep -i "maya6Ways_5Reuse.o" | grep -v "grep" | wc -l`    
done
exp_count=`ps aux | grep -i "maya6Ways_7Reuse.o" | grep -v "grep" | wc -l`
while [ $exp_count -gt 0 ]
do
    sleep 30
    exp_count=`ps aux | grep -i "maya6Ways_7Reuse.o" | grep -v "grep" | wc -l`    
done

##Generate Bucket_Probabilities
./get_bucket_prob_12wps.sh
./get_bucket_prob_3wps.sh
./get_bucket_prob_1reuse.sh
./get_bucket_prob_5reuse.sh
./get_bucket_prob_7reuse.sh
