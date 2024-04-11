#!/bin/bash

mkdir -p maya/results
mkdir -p mirage/results


echo "Building Baseline Single-Core-2MB"

cd mirage
./build_champsim.sh bimodal no no no srrip 1 0 -1 0 1 1 3 0 1 1 > build.log
mv bin/bimodal-no-no-no-srrip-0-1-1-3-0-1-1core-1core_llc bin/baseline_1core_2MB

echo "Building Mirage Single-Core-2MB"

sed -i.bak 's/\<MIRAGE 0\>/MIRAGE 1/g' inc/cache.h
./build_champsim.sh bimodal no no no srrip 1 0 -1 0 1 1 3 0 1 1 > build.log
sed -i.bak 's/\<MIRAGE 1\>/MIRAGE 0/g' inc/cache.h
mv bin/bimodal-no-no-no-srrip-0-1-1-3-0-1-1core-1core_llc bin/mirage_1core_2MB

####-----------------------------------------Dead blocks------------------------------------####

echo "Running Baseline Single-Core-2MB SPEC_2017"
./run_1core_spec.sh bin/baseline_1core_2MB 200 200 baseline_1core_2MB_spec_dead
echo "Running Baseline Single-Core-2MB GAP"
./run_1core_gap.sh bin/baseline_1core_2MB 200 200 baseline_1core_2MB_gap_dead

echo "Running Mirage Single-Core-2MB SPEC_2017"
./run_1core_spec.sh bin/mirage_1core_2MB 200 200 mirage_1core_2MB_spec_dead
echo "Running Mirage Single-Core-2MB GAP"
./run_1core_gap.sh bin/mirage_1core_2MB 200 200 mirage_1core_2MB_gap_dead
