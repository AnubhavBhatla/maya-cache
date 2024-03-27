#!/bin/bash

if [ "$#" -lt 14 ]; then
    echo "Illegal number of parameters"
    echo "Usage: ./build_champsim.sh [branch_pred] $1  [l1d_pref] $2 [l2c_pref] $3 [llc_pref] $4 [llc_repl] $5 [num_core]  $6 [is_huge_page] $7 [is_divided_queues] $8 [ceaser_s_llc] ${9} [partitions] ${10} [pipelined_encryption_engine] ${11} [ceaser_latency] ${12}  [Remap] ${13} {remapping_on_eviction] ${14} [LLC_SLICES] ${15}"
        
exit 1
fi

# ChampSim configuration
BRANCH=$1           # branch/*.bpred
L1D_PREFETCHER=$2   # prefetcher/*.l1d_pref
L2C_PREFETCHER=$3   # prefetcher/*.l2c_pref
LLC_PREFETCHER=$4   # prefetcher/*.llc_pref
LLC_REPLACEMENT=$5  # replacement/*.llc_repl
NUM_CORE=$6         # tested up to 8-core system
IS_HUGE_PAGE=${7}
IS_DIVIDED_QUEUES=${8} 				#-1: Single core no SMT , -2: Multicore no SMT
CEASER_S_LLC=${9}     				# 0 : Non-randomization , 1: CEASER-S 
partitions=${10}       				# Number of partitions 1 : CEASER , 2 : CEASER-S , 16 : SCATTERCACHE 
pipelined_encryption_engine=${11}        	# 0 : Non-pipelined Encryption Engine , 1 : Pipelined Encryption Engine  
CEASER_LATENCY=${12}   				# Encryption Engine Latency
remapping=${13}  				# 0 : Remapping Off , 1 : Remapping On
remapping_on_eviction=${14}			# 0 : Remapping is based on LLC Accesses , 1 : Remapping is based on LLC evictions
LLC_SLICES=${15}  				# Number of LLC SLICE  Default : 1 LLC_SLICE/CORE
CEASER_L1I=0
CEASER_L1D=0
CEASER_L2C=0
CEASER_rq_wq_ratio=21 # After 21 Load requests 1 write request will get the encryption engine
############## Some useful macros ###############
BOLD=$(tput bold)
NORMAL=$(tput sgr0)
#################################################

#####################Set CEASER Parameters##############################
rm inc/ceaser.h
touch inc/ceaser.h
echo -e "#define CEASER_L1I ${CEASER_L1I}"  >> inc/ceaser.h
echo -e "#define CEASER_L1D ${CEASER_L1D}"  >> inc/ceaser.h
echo -e "#define CEASER_L2C ${CEASER_L2C}"  >> inc/ceaser.h
echo -e "#define CEASER_S_LLC ${CEASER_S_LLC}"  >> inc/ceaser.h
echo -e "#define CEASER_LATENCY ${CEASER_LATENCY}"  >> inc/ceaser.h
echo -e "#define CEASER_rq_wq_ratio ${CEASER_rq_wq_ratio}"  >> inc/ceaser.h
echo -e "#define Pipelined_Encryption_Engine ${pipelined_encryption_engine}" >> inc/ceaser.h
echo -e "#define remap_on_evictions ${remapping_on_eviction}" >> inc/ceaser.h

if [ $remapping -eq 0 ]; then 
	echo -e "#define No_Remapping"  >> inc/ceaser.h
fi
#echo -e "using namespace std;" >> inc/ceaser.h
#echo -e "int CEASER_LATENCY= ${CEASER_LATENCY};"  >> inc/ceaser.h
#########################################################################


# Sanity check
if [[ $NUM_CORE -eq 1 ]] && [[ ! $LLC_SLICES ]]; then
	echo "Specify LLC_SLICES for single-core simulations"
	exit 1
fi

if [ ! -f ./branch/${BRANCH}.bpred ]; then
    echo "[ERROR] Cannot find branch predictor"
	echo "[ERROR] Possible branch predictors from branch/*.bpred "
    find branch -name "*.bpred"
    exit 1
fi

if [ ! -f ./prefetcher/${L1D_PREFETCHER}.l1d_pref ]; then
    echo "[ERROR] Cannot find L1D prefetcher"
	echo "[ERROR] Possible L1D prefetchers from prefetcher/*.l1d_pref "
    find prefetcher -name "*.l1d_pref"
    exit 1
fi

if [ ! -f ./prefetcher/${L2C_PREFETCHER}.l2c_pref ]; then
    echo "[ERROR] Cannot find L2C prefetcher"
	echo "[ERROR] Possible L2C prefetchers from prefetcher/*.l2c_pref "
    find prefetcher -name "*.l2c_pref"
    exit 1
fi

if [ ! -f ./prefetcher/${LLC_PREFETCHER}.llc_pref ]; then
    echo "[ERROR] Cannot find LLC prefetcher"
	echo "[ERROR] Possible LLC prefetchers from prefetcher/*.llc_pref "
    find prefetcher -name "*.llc_pref"
    exit 1
fi

if [ ! -f ./replacement/${LLC_REPLACEMENT}.llc_repl ]; then
    echo "[ERROR] Cannot find LLC replacement policy"
	echo "[ERROR] Possible LLC replacement policy from replacement/*.llc_repl"
    find replacement -name "*.llc_repl"
    exit 1
fi



# Check num_core
re='^[0-9]+$'
if ! [[ $NUM_CORE =~ $re ]] ; then
    echo "[ERROR]: num_core is NOT a number" >&2;
    exit 1
fi

DRAM_CHANNELS=`expr $NUM_CORE / 4`
if [ "$DRAM_CHANNELS" -eq "0" ];then
        DRAM_CHANNELS=1
fi

LOG2_DRAM_CHANNELS=$(echo "x = l($DRAM_CHANNELS) / l(2); scale = 0; x / 1" | bc -l)


# Check for multi-core
if [ "$NUM_CORE" -gt "1" ]; then
    echo "Building multi-core ChampSim..."
    sed -i.bak 's/\<NUM_CPUS 1\>/NUM_CPUS '${NUM_CORE}'/g' inc/champsim.h
	sed -i.bak 's/\<DRAM_CHANNELS 1\>/DRAM_CHANNELS '${DRAM_CHANNELS}'/g' inc/champsim.h
	sed -i.bak 's/\<LOG2_DRAM_CHANNELS 0\>/LOG2_DRAM_CHANNELS '${LOG2_DRAM_CHANNELS}'/g' inc/champsim.h
else
    if [ "$NUM_CORE" -lt "1" ]; then
        echo "Number of core: $NUM_CORE must be greater or equal than 1"
        exit 1
    else
        DRAM_CHANNELS=`expr $LLC_SLICES / 4`
        if [ "$DRAM_CHANNELS" -eq "0" ];then
		DRAM_CHANNELS=1
	fi	
	LOG2_DRAM_CHANNELS=$(echo "x = l($DRAM_CHANNELS) / l(2); scale = 0; x / 1" | bc -l)
	sed -i.bak 's/\<DRAM_CHANNELS 1\>/DRAM_CHANNELS '${DRAM_CHANNELS}'/g' inc/champsim.h
	sed -i.bak 's/\<LOG2_DRAM_CHANNELS 0\>/LOG2_DRAM_CHANNELS '${LOG2_DRAM_CHANNELS}'/g' inc/champsim.h
	sed -i.bak 's/\<NUM_SLICES 1\>/NUM_SLICES '${LLC_SLICES}'/g' inc/no_smt_single_core
	echo "Building single-core ChampSim..."
    fi
fi
echo

if [ $IS_HUGE_PAGE == 1 ]; then
        sed -i.bak 's/\/\/#define HUGE_PAGE/#define HUGE_PAGE/g' inc/champsim.h
fi
#### Partition ####
sed -i.bak 's/int partitions;/int partitions = '${partitions}';/g' inc/cache.h

# Change prefetchers and replacement policy
cp branch/${BRANCH}.bpred branch/branch_predictor.cc
cp prefetcher/${L1D_PREFETCHER}.l1d_pref prefetcher/l1d_prefetcher.cc
cp prefetcher/${L2C_PREFETCHER}.l2c_pref prefetcher/l2c_prefetcher.cc
cp prefetcher/${LLC_PREFETCHER}.llc_pref prefetcher/llc_prefetcher.cc
cp replacement/${LLC_REPLACEMENT}.llc_repl replacement/llc_replacement.cc

if [ ${IS_DIVIDED_QUEUES} == -1 ]; then
        cp inc/no_smt_single_core inc/smt.h
elif [ ${IS_DIVIDED_QUEUES} == -2 ]; then
        cp inc/no_smt_multi_core inc/smt.h

fi




# Build
mkdir -p bin
rm -f bin/champsim
make clean
make

# Sanity check
echo ""
if [ ! -f bin/champsim ]; then
        echo "${BOLD}ChampSim build FAILED!"
        echo ""
        # Restore to the default configuration
	sed -i.bak 's/\<NUM_CPUS '${NUM_CORE}'\>/NUM_CPUS 1/g' inc/champsim.h
	sed -i.bak 's/\<DRAM_CHANNELS '${DRAM_CHANNELS}'\>/DRAM_CHANNELS 1/g' inc/champsim.h
	sed -i.bak 's/\<LOG2_DRAM_CHANNELS '${LOG2_DRAM_CHANNELS}'\>/LOG2_DRAM_CHANNELS 0/g' inc/champsim.h

	if [[ "$NUM_CORE" -eq "1" ]]; then
	    sed -i.bak 's/\<NUM_SLICES '${LLC_SLICES}'\>/NUM_SLICES 1/g' inc/no_smt_single_core
	fi

	if [ $IS_HUGE_PAGE == 1 ]; then
		sed -i.bak 's/#define HUGE_PAGE/\/\/#define HUGE_PAGE/g' inc/champsim.h
	fi

    exit 1
fi

echo "${BOLD}ChampSim is successfully built"
echo "Branch Predictor: ${BRANCH}"
echo "L1D Prefetcher: ${L1D_PREFETCHER}"
echo "L2C Prefetcher: ${L2C_PREFETCHER}"
echo "LLC Prefetcher: ${LLC_PREFETCHER}"
echo "LLC Replacement: ${LLC_REPLACEMENT}"
echo "Cores: ${NUM_CORE}"
echo "CEASER_rq_wq_ratio : ${CEASER_rq_wq_ratio}"
echo "Remapping : ${remapping}"
 if [ "$NUM_CORE" -gt "1" ]; then
	BINARY_NAME="${BRANCH}-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${LLC_REPLACEMENT}-${CEASER_S_LLC}-${partitions}-${pipelined_encryption_engine}-${CEASER_LATENCY}-${remapping}-${remapping_on_eviction}-${NUM_CORE}core_mirage"
else
        BINARY_NAME="${BRANCH}-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${LLC_REPLACEMENT}-${CEASER_S_LLC}-${partitions}-${pipelined_encryption_engine}-${CEASER_LATENCY}-${remapping}-${remapping_on_eviction}-${NUM_CORE}core-${LLC_SLICES}core_llc"
 fi

echo "Binary: bin/${BINARY_NAME}"
echo ""
mv bin/champsim bin/${BINARY_NAME}

# Restore to the default configuration
sed -i.bak 's/\<NUM_CPUS '${NUM_CORE}'\>/NUM_CPUS 1/g' inc/champsim.h
sed -i.bak 's/\<DRAM_CHANNELS '${DRAM_CHANNELS}'\>/DRAM_CHANNELS 1/g' inc/champsim.h
sed -i.bak 's/\<LOG2_DRAM_CHANNELS '${LOG2_DRAM_CHANNELS}'\>/LOG2_DRAM_CHANNELS 0/g' inc/champsim.h
sed -i.bak 's/int partitions = '${partitions}';/int partitions;/g' inc/cache.h

if [[ "$NUM_CORE" -eq "1" ]]; then
    sed -i.bak 's/\<NUM_SLICES '${LLC_SLICES}'\>/NUM_SLICES 1/g' inc/no_smt_single_core
fi

if [ $IS_HUGE_PAGE == 1 ]; then
        sed -i.bak 's/#define HUGE_PAGE/\/\/#define HUGE_PAGE/g' inc/champsim.h
fi


cp branch/bimodal.bpred branch/branch_predictor.cc
cp prefetcher/no.l1d_pref prefetcher/l1d_prefetcher.cc
cp prefetcher/no.l2c_pref prefetcher/l2c_prefetcher.cc
cp prefetcher/no.llc_pref prefetcher/llc_prefetcher.cc
cp replacement/lru.llc_repl replacement/llc_replacement.cc
