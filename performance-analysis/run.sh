#!/bin/bash

mkdir -p maya/results
mkdir -p mirage/results

######--------------- MIRAGE Configuration -----------------####


cd mirage/

####-----------------Deadblocks-----------------------------###
echo "Running Baseline Single-Core-2MB SPEC_2017"
./run_1core_spec.sh bin/baseline_1core_2MB 200 200 baseline_1core_2MB
echo "Running Baseline Single-Core-2MB GAP"
./run_1core_gap.sh bin/baseline_1core_2MB 200 200 baseline_1core_2MB

echo "Running Mirage Single-Core-2MB SPEC_2017"
./run_1core_spec.sh bin/mirage_1core_2MB 200 200 mirage_1core_2MB
echo "Running Mirage Single-Core-2MB GAP"
./run_1core_gap.sh bin/mirage_1core_2MB 200 200 mirage_1core_2MB
cd ../

cd mirage/
echo "Running Baseline Single-Core-16MB-8slices SPEC_2017"
./run_1core_spec.sh bin/baseline_1core_2MB_8slices 200 200 baseline_1core_16MB_8slices
echo "Running Baseline Eight-Core-16MB SPEC_2017"
./run_8core_spec.sh bin/baseline_8core_2MB 200 200 baseline_8core_16MB

cd ../maya/
echo "Running Maya Single-Core-12MB-1way-8slices SPEC_2017"
./run_1core_spec.sh bin/maya_1core_12MB_1way_8slices 200 200 maya_1core_12MB_1way_8slices
echo "Running Maya Eight-Core-12MB-1way SPEC_2017"
./run_8core_spec.sh bin/maya_8core_12MB_1way 200 200 maya_8core_12MB_1way

echo "Running Maya Single-Core-12MB-3way-8slices SPEC_2017"
./run_1core_spec.sh bin/maya_1core_12MB_3way_8slices 200 200 maya_1core_12MB_3way_8slices
echo "Running Maya Eight-Core-12MB-3way SPEC_2017"
./run_8core_spec.sh bin/maya_8core_12MB_3way 200 200 maya_8core_12MB_3way

echo "Running Maya Single-Core-12MB-5way-8slices SPEC_2017"
./run_1core_spec.sh bin/maya_1core_12MB_5way_8slices 200 200 maya_1core_12MB_5way_8slices
echo "Running Maya Eight-Core-12MB-5way SPEC_2017"
./run_8core_spec.sh bin/maya_8core_12MB_5way 200 200 maya_8core_12MB_5way

echo "Running Maya Single-Core-12MB-7way-8slices SPEC_2017"
./run_1core_spec.sh bin/maya_1core_12MB_7way_8slices 200 200 maya_1core_12MB_7way_8slices
echo "Running Maya Eight-Core-12MB-7way SPEC_2017"
./run_8core_spec.sh bin/maya_8core_12MB_7way 200 200 maya_8core_12MB_7way

echo "Running Maya Single-Core-12MB-3way-8slices GAP"
./run_1core_gap.sh bin/maya_1core_12MB_3way_8slices 200 200 maya_1core_12MB_3way_8slices
echo "Running Maya Eight-Core-12MB-3way GAP"
./run_8core_gap.sh bin/maya_8core_12MB_3way 200 200 maya_8core_12MB_3way
