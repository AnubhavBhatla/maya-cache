#!/usr/bin/env bash

## Temp Files
rm -f bucket_occ_temp;
touch bucket_occ_temp;

## Get Total number of experiments
tot_exp=`ls raw_results/maya6ways.ReuseWays7.out | wc -l`;

## Get Line Number for Distribution of Bucket-Occupancy
line_num=`grep -n "Distribution of Bucket-Occupancy (Averaged across Ball Throws) => Used for P(Bucket = k balls) calculation" raw_results/maya6ways.ReuseWays7.out | awk -F':' '{print $1}'`
start_line_num=$(($line_num+3))

## Get the Bucket-Occupancy for an experiment & add it to bucket_occ_temp counts
tail -n+$start_line_num raw_results/maya6ways.ReuseWays7.out | head -n33 | awk -F':' '{print $2}' | awk '{print $1}' >bucket_occ_curr
paste bucket_occ_temp bucket_occ_curr > bucket_occ_next;
awk '{print $1+$2}' bucket_occ_next > bucket_occ_temp

## Total-Ball-Count (2x Ball-thrown, as statistic counts one ball twice)
total_ball_count=`awk '{sum+=$1}END{print sum}' bucket_occ_temp`;

## Output Bucket-Probabilities
echo "BallsInBucket-N Pr_obs(N)" > maya6ways.Reuse7.BucketProb.stat;
awk -v count=$total_ball_count '{print NR-1, $1/count}' bucket_occ_temp >> maya6ways.Reuse7.BucketProb.stat
column -t maya6ways.Reuse7.BucketProb.stat > temp
mv temp maya6ways.Reuse7.BucketProb.stat

## Remove Temp Files
rm -f bucket_occ_curr bucket_occ_next bucket_occ_temp;
