#rm bin/*
#!/bin/bash

if [ "$#" -ne 9 ]; then
    echo "Illegal number of parameters"
    echo "Usage: ./run_multicore.sh [l1d_pref] [l2c_pref] [llc_pref] [trace] [num_core] [num_set] [is_huge_page] [is_divided_queues] [is_run_background] "
    exit 1
fi

L1D_PREFETCHER=${1}
L2C_PREFETCHER=${2}
LLC_PREFETCHER=${3}
trace1=${4}
num_cpus=${5}
NUM_SET=${6}
is_huge_page=${7}
is_divided_queues=${8}
is_run_background=${9}
#traces1=(654.roms_s-1070B.champsimtrace.xz 605.mcf_s-1644B.champsimtrace.xz 621.wrf_s-8065B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz 603.bwaves_s-891B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz)
#traces1=(605.mcf_s-1554B.champsimtrace.xz)
#traces2=(605.mcf_s-1554B.champsimtrace.xz)
#traces2=(llc_fitting.trace.gz)

if [ $is_huge_page == 1 ]; then
	#traces2=(llc_wq_choke_e_Slices_2MB_page.trace.gz)  
	traces2=(llc_fitting_4_Slices_2MB_page.trace.gz)
else
        traces2=(llc_fitting_4_Slices_4KB_page.trace.gz)
fi

if [ $is_divided_queues == -2 ]; then
	if [ $is_huge_page == 1 ]; then
		traces2=(attacker_${num_cpus}_core_4KB.trace.gz)      #(600.perlbench_s-570B.champsimtrace.xz)    #(attacker_${num_cpus}_core_4KB.trace.gz)   # (llc_fitting_for_16_cores.trace.gz)           #(attacker_16_core_4KB.trace.gz) #(attacker_16_core_4KB.trace.gz)   #(attacker_8_core_4KB.trace.gz) #(llc_attacker_with_more_mpki.trace.gz)       #(llc_fitting_parallelism_using_goto.trace.gz)     # (llc_fitting_for_16_cores.trace.gz)              #(llc_fitting_parallelism_using_goto.trace.gz)
	else
		traces2=(attacker_8_core_4KB.trace.gz)
		#traces2=(llc_fitting_${num_cpus}_Slices_4KB_page.trace.gz)
	fi
fi


#traces2=(llc_fitting_per_set_${num_cpus}_Slices_${NUM_SET}_Sets.trace.gz)


if [ $NUM_SET != 1 ]; then
	traces2=(llc_fitting_per_set_${num_cpus}_Slices_${NUM_SET}_Sets.trace.gz)
fi
trace3=(llc_fitting_per_set_${num_cpus}_Slices_${NUM_SET}_Sets.trace.gz)
policy=srrip
policies=(hawkeye) # srrip ship lru drrip)
runfile=run_${num_cpus}core.sh
war=1
sim=1
#trace1=605.mcf_s-1644B.champsimtrace.xz
parallel=0
CEASER_L1I=0 
CEASER_L1D=0 
CEASER_L2C=0
CEASER_LLC=0
CEASER_S_LLC=0
CEASER_LATENCY=5 #8 #5	 Remember you have changed LLC_LATENCY in cache.h
CEASER_rq_wq_ratio=21
remap=1
#for trace1 in ${traces1[*]}
#do
for trace2 in ${traces2[*]} 
do
for CEASER_rq_wq_ratio in  21 #50 20 15 10 5 2
do
for CEASER_LLC in 1 0  #0 1 #CEASER_S
do
for remap in 0    #1 
do
for CEASER_LATENCY in  8 5     #8
do
for policy in ${policies[*]}
do
#   if [ $CEASER_LLC == 1 ]; then
#	   echo $CEASER_LLC
#	   CEASER_S_LLC=0
 # else
#	 CEASER_S_LLC=1 
 # fi
   if [ $is_divided_queues == 0 ]; then
	if [ $is_huge_page == 1 ]; then
		results_folder=new_latency_/results_2MB_page_smtNormalQs/results_${num_cpus}core_${trace1}/${NUM_SET}_Sets
	else
                results_folder=new_latency_/results_4KB_page_smtNormalQs/results_${num_cpus}core_${trace1}/${NUM_SET}_Sets
	fi
   elif [ $is_divided_queues == 1 ]; then
        if [ $is_huge_page == 1 ]; then
                results_folder=new_latency_/results_2MB_page_smtDividedQs/results_${num_cpus}core_${trace1}/${NUM_SET}_Sets
        else
                results_folder=new_latency_/results_4KB_page_smtDividedQs/results_${num_cpus}core_${trace1}/${NUM_SET}_Sets
        fi
  
   elif [ $is_divided_queues == -2 ]; then
        if [ $is_huge_page == 1 ]; then
                 results_folder=Replacement_policy_effect/CEASER_8_core_4KB_attacker_With_4_victim_0EL_pipelined/results_2MB_page/results_${num_cpus}core_${trace1}/${NUM_SET}_Sets/EL_Of_5_pipelined      #${CEASER_LATENCY}
#CEASER_8_core_pipelined_4KB_attacker_With_ROB_0EL/results_2MB_page/results_${num_cpus}core_${trace1}/${NUM_SET}_Sets/EL_Of_${CEASER_LATENCY}
# CEASER_16_core_4KB_attacker_With_ROB                results_folder=8_core_with_randomization_with_remap_single_port_8latency_28_llc_latency_without_pref/results_2MB_page/results_${num_cpus}core_${trace1}/${NUM_SET}_Sets/${CEASER_LATENCY}
		#Experiment2_16_core_read_intensive CEASER_16_core_4KB_attacker_With_ROB  SCATTER_CACHE_8_core_4KB_attacker_With_ROB
        else
                results_folder=EXP_8_core_4KB_attacker/results_4KB_page/results_${num_cpus}core_${trace1}/EL_Of_${CEASER_LATENCY}
        fi
   fi

	./build_champsim.sh bimodal ${L1D_PREFETCHER} ${L2C_PREFETCHER} ${LLC_PREFETCHER} $policy ${num_cpus} ${is_huge_page} ${is_divided_queues} ${CEASER_LLC} ${CEASER_S_LLC} ${CEASER_LATENCY} ${CEASER_rq_wq_ratio} ${remap}
	

	if [ $num_cpus == 2 ]; then
		 bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-$remap-${CEASER_rq_wq_ratio}-${num_cpus}core ${war} ${sim} 0-2 ${trace1} ${trace1} ${results_folder} &
#	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${num_cpus}core ${war} ${sim} ${trace2} ${trace1} ${results_folder} &
 #       bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${num_cpus}core ${war} ${sim} ${trace1} ${trace1} ${results_folder} &

	fi
	if [ $num_cpus == 4 ]; then

	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${num_cpus}core ${war} ${sim} 3-1 ${trace2} ${trace2} ${trace2} ${trace1} ${results_folder} &
	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${num_cpus}core ${war} ${sim} 2-2 ${trace2} ${trace2} ${trace1} ${trace1} ${results_folder} &
	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${num_cpus}core ${war} ${sim} 1-3 ${trace2} ${trace1} ${trace1} ${trace1} ${results_folder} &
        bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${num_cpus}core ${war} ${sim} 0-4 ${trace1} ${trace1} ${trace1} ${trace1} ${results_folder} &

	fi
        if [ $num_cpus == 8 ]; then
	#echo $trace2	
	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${remap}-${CEASER_rq_wq_ratio}-${num_cpus}core ${war} ${sim} 4-4 ${trace2} ${trace2} ${trace2} ${trace2} ${trace1} ${trace1} ${trace1} ${trace1} ${results_folder} &
#	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${CEASER_rq_wq_ratio}-${num_cpus}core ${war} ${sim} 0-8 ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${results_folder} &
	#if [ $is_run_background == 0 ] && [ $CEASER_S_LLC == 1 ]; then
        #bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${num_cpus}core ${war} ${sim} 7-1 ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace1} ${results_folder}  
	#else
#	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${remap}-${CEASER_rq_wq_ratio}-${num_cpus}core ${war} ${sim} 1-7 ${trace2} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${results_folder}  &
#	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${remap}-${CEASER_rq_wq_ratio}-${num_cpus}core ${war} ${sim} 8-0 ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${results_folder}  &
#        bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${remap}-${CEASER_rq_wq_ratio}-${num_cpus}core ${war} ${sim} 7-1 ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace1} ${results_folder}  &
	#fi

	fi

        if [ $num_cpus == 16 ]; then
       # bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${CEASER_rq_wq_ratio}-${num_cpus}core ${war} ${sim} 1V-15A ${trace1} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${results_folder} &

	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${remap}-${CEASER_rq_wq_ratio}-${num_cpus}core ${war} ${sim} 15-1 ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace2} ${trace1} ${results_folder} &
	
#	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${num_cpus}core ${war} ${sim} 8-8 ${trace2} ${trace2} ${trace2} ${trace2}  ${trace2} ${trace2} ${trace2} ${trace2} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${results_folder} &
#	bash ${runfile} bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${CEASER_rq_wq_ratio}-${num_cpus}core ${war} ${sim} 0-16 ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${trace1} ${results_folder} &

     fi


	#rm inc/ceaser.h
	#vim results_${sim}M/${trace}-bimodal-no-no-no-${policy}-${num_cpus}core.txt
done
done
done
done
done
done
echo $results_folder
