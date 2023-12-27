#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    echo "Usage: ./search_8core.sh [is_divided_queues (0: Normal Queue for SMT, 1: Divided Queues for SMT, -2: No SMT)]" #[is_huge_page] [NUM_SET]"
    exit 1
fi

is_divided_queues=${1}

is_huge_page=1
#is_huge_page=${1}
#NUM_SET=${2}
#L1D_PREFETCHER=$1
#L2C_PREFETCHER=$2
#LLC_PREFETCHER=$3
CEASER_S_LLC=0
CEASER_LLC=0
CEASER_LATENCY=3  #8
#mixes=(0-8 4-4 7-1)
mixes=(7-1)    #(4-4)      #(1-7)
num_cpus=1
policy=srrip
war=10 #50
sim=10 #200
#results_folder=results_${num_cpus}core_${sim}M
parallel=0
CEASER_L1I=0 
CEASER_L1D=0 
CEASER_L2C=0 
#traces=(600.perlbench_s-570B.champsimtrace.xz 605.mcf_s-484B.champsimtrace.xz 654.roms_s-1070B.champsimtrace.xz 603.bwaves_s-891B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz)
#traces=(605.mcf_s-484B 600.perlbench_s-570B) # 654.roms_s-1070B 621.wrf_s-8065B 605.mcf_s-1554B 603.bwaves_s-891B 602.gcc_s-2226B)
#traces=(654.roms_s-1070B.champsimtrace.xz 605.mcf_s-1644B.champsimtrace.xz 621.wrf_s-8065B.champsimtrace.xz 603.bwaves_s-891B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz)
#traces=(605.mcf_s-1554B.champsimtrace.xz)
#traces=(600.perlbench_s-570B.champsimtrace.xz 620.omnetpp_s-141B.champsimtrace.xz 603.bwaves_s-2931B.champsimtrace.xz 649.fotonik3d_s-10881B.champsimtrace.xz 623.xalancbmk_s-10B.champsimtrace.xz 654.roms_s-1390B.champsimtrace.xz 619.lbm_s-4268B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz)
#traces=(619.lbm_s-4268B.champsimtrace.xz)
#traces=(llc_fitting.trace.gz)
#traces=(600.perlbench_s-570B.champsimtrace.xz 620.omnetpp_s-141B.champsimtrace.xz 603.bwaves_s-2931B.champsimtrace.xz 649.fotonik3d_s-10881B.champsimtrace.xz 623.xalancbmk_s-10B.champsimtrace.xz 654.roms_s-1390B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz)
#traces=(600.perlbench_s-570B.champsimtrace.xz 649.fotonik3d_s-10881B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 603.bwaves_s-2931B.champsimtrace.xz 620.omnetpp_s-141B.champsimtrace.xz 654.roms_s-1390B.champsimtrace.xz 623.xalancbmk_s-10B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz)
#traces=(605.mcf_s-1152B.champsimtrace.xz  605.mcf_s-1644B.champsimtrace.xz  605.mcf_s-665B.champsimtrace.xz 605.mcf_s-1536B.champsimtrace.xz 605.mcf_s-472B.champsimtrace.xz   605.mcf_s-782B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz  605.mcf_s-484B.champsimtrace.xz   605.mcf_s-994B.champsimtrace.xz
 #)
#traces=(600.perlbench_s-570B.champsimtrace.xz 657.xz_s-2302B.champsimtrace.xz)
#traces=(605.mcf_s-1152B.champsimtrace.xz  605.mcf_s-1644B.champsimtrace.xz  605.mcf_s-665B.champsimtrace.xz 605.mcf_s-1536B.champsimtrace.xz 605.mcf_s-472B.champsimtrace.xz   605.mcf_s-782B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz  605.mcf_s-484B.champsimtrace.xz   605.mcf_s-994B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 627.cam4_s-573B.champsimtrace.xz 621.wrf_s-8065B.champsimtrace.xz 619.lbm_s-4268B.champsimtrace.xz 619.lbm_s-2677B.champsimtrace.xz 619.lbm_s-2676B.champsimtrace.xz 623.xalancbmk_s-10B.champsimtrace.xz 603.bwaves_s-891B.champsimtrace.xz 654.roms_s-1390B.champsimtrace.xz)
#traces=(605.mcf_s-1644B.champsimtrace.xz)
#traces=(605.mcf_s-1152B.champsimtrace.xz  605.mcf_s-1644B.champsimtrace.xz  605.mcf_s-665B.champsimtrace.xz 605.mcf_s-1536B.champsimtrace.xz 605.mcf_s-472B.champsimtrace.xz   605.mcf_s-782B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz  605.mcf_s-484B.champsimtrace.xz   605.mcf_s-994B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 627.cam4_s-573B.champsimtrace.xz 621.wrf_s-8065B.champsimtrace.xz 619.lbm_s-4268B.champsimtrace.xz 619.lbm_s-2677B.champsimtrace.xz 619.lbm_s-2676B.champsimtrace.xz 623.xalancbmk_s-10B.champsimtrace.xz 603.bwaves_s-891B.champsimtrace.xz 654.roms_s-1390B.champsimtrace.xz 600.perlbench_s-570B.champsimtrace.xz  657.xz_s-2302B.champsimtrace.xz )
#traces=(605.mcf_s-1554B.champsimtrace.xz 603.bwaves_s-1080B.champsimtrace.xz 607.cactuBSSN_s-4248B.champsimtrace.xz)
 pref_combinations=(no-no-no) #(no-no-no ) # next_line-ip_stride-no ipcp-ipcp-no next_line-no-no)
#traces=(  605.mcf_s-1554B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 600.perlbench_s-570B.champsimtrace.xz 623.xalancbmk_s-700B.champsimtrace.xz)
#traces=(605.mcf_s-1554B.champsimtrace.xz  602.gcc_s-2226B.champsimtrace.xz )
traces=(600.perlbench_s-210B.champsimtrace.xz  644.nab_s-12459B.champsimtrace.xz       657.xz_s-3167B.champsimtrace.xz 621.wrf_s-8065B.champsimtrace.xz
        654.roms_s-293B.champsimtrace.xz
       607.cactuBSSN_s-2421B.champsimtrace.xz
        649.fotonik3d_s-1176B.champsimtrace.xz
       603.bwaves_s-891B.champsimtrace.xz
        649.fotonik3d_s-7084B.champsimtrace.xz  623.xalancbmk_s-202B.champsimtrace.xz
        654.roms_s-1390B.champsimtrace.xz
        619.lbm_s-2676B.champsimtrace.xz
        605.mcf_s-782B.champsimtrace.xz         605.mcf_s-1554B.champsimtrace.xz
        602.gcc_s-2226B.champsimtrace.xz
)
traces2=(605.mcf_s-1554B.champsimtrace.xz) # 605.mcf_s-782B.champsimtrace.xz)
#traces=(657.xz_s-3167B.champsimtrace.xz)
remap=1
NUM_SET=1
policies=(ship ship_drishyam)     #_drishyam) #(srrip ship lru drrip hawkeye)
partitions=1
#for remap in 1 0
for NUM_SET in 1
do
for trace in ${traces[*]}
do
	echo $trace | tr -s '\n' ';'
for mix in ${mixes[*]}
do
  
   for core in 1 # 2 3 4 5 6 7 8
   do
	for pref_combination in ${pref_combinations[*]}
	do
		
	for remap in 1 
	do
		for  policy in ${policies[*]}
		do
		 for choice in 17 18 19  #1 2 3 4 5 6 7 #8 9 10 11 12 13 14 15  
 		 do
		  for CEASER_S_LLC in  1  #CEASER_LLC in 0 1               #CEASER_S_LLC in 0 1
		  do
                IFS='-' # hyphen (-) is set as delimiter
                read -ra ADDR <<< "${pref_combination}" # string is read into an array as tokens separated by IFS
                L1D_PREFETCHER=${ADDR[0]}
                L2C_PREFETCHER=${ADDR[1]}
                LLC_PREFETCHER=${ADDR[2]}
                IFS=""
		if [ $is_divided_queues == -2 ]; then
				if [ $is_huge_page == 1 ]; then
					 results_folder=results_200M
#Scatter_Cache_8_core_4KB_attacker_With_4_victim/results_2MB_page/results_${num_cpus}core_${trace}/${NUM_SET}_Sets/EL_Of_3_pipelined
#Scatter_Cache_8_core_4KB_attacker_With_4_victim/results_2MB_page/results_${num_cpus}core_${trace}/${NUM_SET}_Sets/EL_Of_5_pipelined
				#	results_folder=CEASER_8_core_4KB_attacker_With_7_victim/results_2MB_page/results_${num_cpus}core_${trace}/${NUM_SET}_Sets/EL_Of_${CEASER_LATENCY}
#CEASER_8_core_4KB_attacker SCATTER_CACHE_8_core_4KB_attacker cEASER_8_core_4KB_attacker_with_New_ROB
			#		results_folder=New_CEASER_S_8_core_with_randomization_with_remap_single_port_8latency_28_llc_latency_without_pref/results_2MB_page/results_${num_cpus}core_${trace}/${NUM_SET}_Sets  #Experiment2_16_core_read_intensive Without_remap_8_core_read_intensive
                                elif [ $is_huge_page == 0 ]; then
                                        results_folder=results_4KB_page/results_${num_cpus}core_${trace}/${NUM_SET}_Sets
			  	fi		
		fi
		if [ $is_divided_queues == 0 ]; then
				if [ $is_huge_page == 1 ]; then
                                        results_folder=new_latency_/results_2MB_page_smtNormalQs/results_${num_cpus}core_${trace}/${NUM_SET}_Sets
                                
                                elif [ $is_huge_page == 0 ]; then
                                        results_folder=results_4KB_page_smtNormalQs/results_${num_cpus}core_${trace}/${NUM_SET}_Sets
				fi

		fi

		if [ $is_divided_queues == 1 ]; then
				if [ $is_huge_page == 1 ]; then
                                        results_folder=invalidate_Sptr_Set/results_2MB_page_smtDividedQs/results_${num_cpus}core_${trace}/${NUM_SET}_Sets
                                
				elif [ $is_huge_page == 0 ]; then
                                        results_folder=invalidate_Sptr_Set/results_4KB_page_smtDividedQs/results_${num_cpus}core_${trace}/${NUM_SET}_Sets
                                fi
		fi

				if [ $CEASER_S_LLC == 0 ] || [ $CEASER_LLC == 1 ]; then
                                file_name=$results_folder/${trace}-hashed_perceptron-no-no-no-${policy}-${CEASER_S_LLC}-${partitions}-1-${CEASER_LATENCY}-${remap}-1-1core-1core_llc.txt
                                else
                                file_name=$results_folder/${trace}-hashed_perceptron-no-no-no-${policy}-${CEASER_S_LLC}-${partitions}-1-${CEASER_LATENCY}-${remap}-1-1core-1core_llc.txt    #_bfs.txt    #with_multi_step.txt        #with_multi_step.txt
                                fi

				#if [ $CEASER_S_LLC == 0 ] || [ $CEASER_LLC == 1 ]; then
				#file_name=$results_folder/${trace}-hashed_perceptron-no-no-no-${policy}-${CEASER_S_LLC}-${partitions}-1-${CEASER_LATENCY}-${remap}-1-1core-1core_llc.txt		
				#else
				#file_name=$results_folder/${trace}-hashed_perceptron-no-no-no-${policy}-${CEASER_S_LLC}-${partitions}-1-${CEASER_LATENCY}-${remap}-1-1core-1core_llcwith_multi_step.txt        #with_multi_step.txt
				#fi
				#	echo $file_name			
	#file_name=$results_folder/mix${mix}-bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-${remap}-21-${num_cpus}core.txt
				
		#		else
		#		file_name=CEASER_8_core_4KB_attacker_With_4_victim/results_2MB_page/results_${num_cpus}core_${trace}/${NUM_SET}_Sets/EL_Of_3_pipelined/mix${mix}-bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-0-21-${num_cpus}core.txt

#$results_folder/mix${mix}-bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-0-21-${num_cpus}core.txt
		#		fi
				#mul=1000

				#IPC=$(grep IPC $file_name | tail -n $num_cpus | awk '{print $5}' | head -n ${core} | tail -n 1)
				#inst=$(grep instructions $file_name | tail -n $num_cpus | awk '{print $7}' | head -n ${core} | tail -n 1)
				#miss=$(grep MISS $file_name | grep LLC | grep LOAD | tail -n $num_cpus | awk '{print $8}' | head -n ${core} | tail -n 1 )
				#miss=$(echo "scale=2; $miss*$mul" | bc -l )
				#MPKI=$(echo "scale=2; $miss/$inst" | bc -l )
			

	#	echo $results_folder	  	
				if [ $choice == 1 ]; then
	                                IPC=$(grep IPC $file_name | grep -v "Simulation" | tail -n $num_cpus | awk '{print $5}' | head -n ${core} | tail -n 1)
					if [[ -n $IPC ]]; then
						echo $IPC | tr -s '\n' ';'
					else
						echo "-" | tr -s '\n' ';'
					fi
					
				#	echo $IPC | tr -s '\n' ';'
				elif [ $choice == 17 ]; then
					Sptr=$(grep "Total Invalid Blocks" $file_name | awk '{print $8}')
					#$(grep IPC $file_name | grep -v "Simulation" | tail -n $num_cpus | awk '{print $5}' | head -n ${core} | tail -n 1)
					if [[ -n $Sptr ]]; then
						echo $Sptr | tr -s '\n' ';'
					else
                                                echo "-" | tr -s '\n' ';'
                                        fi

				#	echo "Hi"
				
				elif [ $choice == 18 ]; then
					Invalid_blocks=$(grep "Total Blocks Remapped" $file_name | awk '{print $14}' )
					if [[ -n $Invalid_blocks ]]; then
                                                echo $Invalid_blocks | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi

				elif [ $choice == 19 ]; then
                                        Invalid_blocks=$(grep "Total Blocks Saved Per set:" $file_name | awk '{print $6}' )
                                        if [[ -n $Invalid_blocks ]]; then
                                                echo $Invalid_blocks | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi

				#	echo "Hi"
					#elif[ $choice == 17 ]; then
                                 #       Invalid_blocks=$(grep "Invalid Blocks After Remapping" $file_name | awk '{print $9}' )
                                  #      if [[ -n $Invalid_blocks ]]; then
                                   #             echo $Invalid_blocks | tr -s '\n' ';'
                                    #    else
                                     #           echo "-" | tr -s '\n' ';'
                                      #  fi
				
				#elif[ $choice == 18 ]; then
                                        Invalid_blocks=$(grep "Invalid Blocks After Remapping" $file_name | awk '{print $5}' )
                                  #      if [[ -n $Invalid_blocks ]]; then
                                   #             echo $Invalid_blocks | tr -s '\n' ';'
                                    #    else
                                     #           echo "-" | tr -s '\n' ';'
                                      #  fi


                                elif [ $choice == 2 ]; then
					mul=1000
                                	inst=$(grep instructions $file_name| grep -v "Simulation" | tail -n $num_cpus | awk '{print $7}' | head -n ${core} | tail -n 1)
					if [[ -n $inst ]]; then
                                	  l1d_load_miss=$(grep MISS $file_name | grep -v "Simulation" | grep L1D | grep LOAD | tail -n $num_cpus | awk '{print $8}' | head -n ${core} | tail -n 1 )
                                          l1d_rfo_miss=$(grep MISS $file_name | grep -v "Simulation" | grep L1D | grep RFO | tail -n $num_cpus | awk '{print $8}' | head -n ${core} | tail -n 1 )

				          miss=$(echo "scale=2; $l1d_load_miss+$l1d_rfo_miss" | bc -l )
                                	  miss=$(echo "scale=2; $miss*$mul" | bc -l )
                                	  L1D_MPKI=$(echo "scale=2; $miss/$inst" | bc -l )

                                          echo $L1D_MPKI | tr -s '\n' ';'
					else
						echo "-" | tr -s '\n' ';'
					fi
                                

				elif [ $choice == 3 ]; then
					AVG_MISS_PENALTY=$(grep -i "L1D AVERAGE" $file_name | grep -v "Simulation" | tail -n $num_cpus | head -n $core | awk '{print $5}' | tail -n 1)
					#AVG_MISS_PENALTY=$(grep -i "L1D TOTAL     ACCESS:" $file_name | grep -v "Simulation" | tail -n $num_cpus | head -n $core | awk '{print $18}' | tail -n 1)
					if [[ -n $AVG_MISS_PENALTY ]]; then
						echo $AVG_MISS_PENALTY | tr -s '\n' ';'

                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi


                                elif [ $choice == 4 ]; then
                                        mul=1000
                                        inst=$(grep instructions $file_name| grep -v "Simulation" | tail -n $num_cpus | awk '{print $7}' | head -n ${core} | tail -n 1)
                                        if [[ -n $inst ]]; then
                                          l2c_load_miss=$(grep MISS $file_name | grep -v "Simulation" | grep L2C | grep LOAD | tail -n $num_cpus | awk '{print $8}' | head -n ${core} | tail -n 1 )
					  l2c_rfo_miss=$(grep MISS $file_name | grep -v "Simulation" | grep L2C | grep RFO | tail -n $num_cpus | awk '{print $8}' | head -n ${core} | tail -n 1 )

                                          miss=$(echo "scale=2; $l2c_load_miss+$l2c_rfo_miss" | bc -l )

                                          miss=$(echo "scale=2; $miss*$mul" | bc -l )
                                          L2C_MPKI=$(echo "scale=2; $miss/$inst" | bc -l )

                                          echo $L2C_MPKI | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi


                                elif [ $choice == 5 ]; then
					AVG_MISS_PENALTY=$(grep -i "L2C AVERAGE" $file_name | grep -v "Simulation" | tail -n $num_cpus | head -n $core | awk '{print $5}' | tail -n 1)

                                        #AVG_MISS_PENALTY=$(grep -i "L2C TOTAL     ACCESS:" $file_name | grep -v "Simulation" | tail -n $num_cpus | head -n $core | awk '{print $18}' | tail -n 1)
                                        if [[ -n $AVG_MISS_PENALTY ]]; then
                                                echo $AVG_MISS_PENALTY | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi




                                elif [ $choice == 6 ]; then
                                        mul=1000
                                        inst=$(grep instructions $file_name| grep -v "Simulation" | tail -n $num_cpus | awk '{print $7}' | head -n ${core} | tail -n 1)
                                        if [[ -n $inst ]]; then
					llc_lines=$(echo "scale=2; $num_cpus*$num_cpus" | bc -l )
					rel_llc_lines=$(echo "scale=2; ${num_cpus}*${core}" | bc -l )
				
                                          llc_load_miss=$(grep MISS $file_name | grep -v "Simulation" | grep LLC | grep LOAD | tail -n $llc_lines | awk '{print $8}' | head -n $rel_llc_lines | tail -n ${num_cpus} | awk '{ SUM += $1} END { print SUM }')
                                          llc_rfo_miss=$(grep MISS $file_name | grep -v "Simulation" | grep LLC | grep RFO | tail -n $llc_lines | awk '{print $8}' | head -n $rel_llc_lines | tail -n ${num_cpus} | awk '{ SUM += $1} END { print SUM }')
                                          miss=$(echo "scale=2; $llc_load_miss+$llc_rfo_miss" | bc -l )

                                          miss=$(echo "scale=2; $miss*$mul" | bc -l )
                                          LLC_MPKI=$(echo "scale=2; $miss/$inst" | bc -l )

                                          echo $LLC_MPKI | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi


                                elif [ $choice == 7 ]; then
                                        llc_lines=$(echo "scale=2; $num_cpus*$num_cpus" | bc -l )
                                        rel_llc_lines=$(echo "scale=2; $num_cpus*$core" | bc -l )
					AVG_MISS_PENALTY=$(grep -i "AVERAGE MISS LATENCY:" $file_name | grep LLC | grep -v "Simulation" | tail -n $llc_lines | head -n $rel_llc_lines |  awk '{print $5}' | tail -n ${num_cpus} | awk '{ SUM += $1} END { print SUM }')
                                        #AVG_MISS_PENALTY=$(grep -i "TOTAL     ACCESS:" $file_name | grep LLC | grep -v "Simulation" | tail -n $llc_lines | head -n $rel_llc_lines |  awk '{print $18}' | tail -n ${num_cpus} | awk '{ SUM += $1} END { print SUM }')

                                        if [[ -n $AVG_MISS_PENALTY ]]; then
						AVG_MISS_PENALTY=$(echo "scale=2; $AVG_MISS_PENALTY/$num_cpus" | bc -l )
                                                echo $AVG_MISS_PENALTY | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi


				elif [ $choice == 8 ]; then 
					DRAM_RQ_HITS=$(grep -i "RQ ROW_BUFFER_HIT" $file_name | grep -v "Simulation" | awk '{print $3}'| awk '{ SUM += $1} END { print SUM }' )
				
                                        if [[ -n $DRAM_RQ_HITS ]]; then
                                                echo "$DRAM_RQ_HITS" | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi


                                elif [ $choice == 9 ]; then
                                        DRAM_RQ_MISS=$(grep -i "RQ ROW_BUFFER_HIT" $file_name | grep -v "Simulation" | awk '{print $5}'| awk '{ SUM += $1} END { print SUM }')
					
                                        if [[ -n $DRAM_RQ_MISS ]]; then
                                                echo "$DRAM_RQ_MISS" | tr -s '\n' ';'
				
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi



				elif [ $choice == 10 ]; then
					rob_stall_counter=$(grep -i "ROI TOTAL ROB counter      :" $file_name | awk '{print $6}' | tail -n $num_cpus | head -n $core | tail -n 1)
					if [[ -n $rob_stall_counter ]]; then
						echo $rob_stall_counter | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi

				
                                elif [ $choice == 12 ]; then
                                        #rob_stall_cycles=$(grep -i "ROI Average ROB Stall Cycles:" $file_name | awk '{print $6}' | tail -n $num_cpus | head -n $core | tail -n 1)
					
					rob_stalls=$(grep -i "ROI Total Load ROB Stalls Cycle" $file_name | awk '{print $8}' | tail -n $num_cpus | head -n $core | tail -n 1)
					#avg_rob_stalls=$rob_stalls/($sim*1000000)
					mul=1000000
					#echo " rob stalls $rob_stalls"
					instr=$(echo "scale=3; $sim*$mul" | bc -l )
					
					avg_rob_stalls=$(echo "scale=3; $rob_stalls/$instr" | bc -l )
					
					if [[ -n $rob_stalls ]]; then
                                        	echo $avg_rob_stalls | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi

				elif [ $choice == 11 ]; then
					total_rob_stalls=$(grep -i "ROI Total ROB Stalls Cycles:" $file_name | awk '{print $6}' | tail -n $num_cpus | head -n $core | tail -n 1)
                                        #avg_load_rob_stall_cycles=$(grep -i "ROI Average Load ROB Stall Cycles:" $file_name | awk '{print $7}' | tail -n $num_cpus | head -n $core | tail -n 1)
                                       #echo "total_rob_stalls $total_rob_stalls" 
					mul=1000000
                                        instr=$(echo "scale=3; $sim*$mul" | bc -l )
					#echo "instr $instr"
					avg_load_rob_stall_cycles=$(echo "scale=3; $total_rob_stalls/$instr" | bc -l )
					if [[ -n $total_rob_stalls ]]; then
                                                echo $avg_load_rob_stall_cycles | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi
				
				elif [ $choice == 13 ]; then
                                        avg_time_lapsed_in_rq=$(grep -i "Average Time Lapsed in RQ" $file_name |  awk '{print $7}' | tail -n $num_cpus | head -n ${core} | tail -n 1)
                                        if [[ -n $avg_time_lapsed_in_rq ]]; then
                                                echo $avg_time_lapsed_in_rq | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi

				elif [ $choice == 14 ]; then
                                        avg_time_lapsed_in_pq=$(grep -i "Average Time Lapsed in PQ" $file_name |  awk '{print $7}' | tail -n $num_cpus | head -n ${core} | tail -n 1)
                                        if [[ -n $avg_time_lapsed_in_pq ]]; then
                                                echo $avg_time_lapsed_in_pq | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi

                          	 elif [ $choice == 15 ]; then
                                        avg_time_lapsed_in_mshr=$(grep -i "Average Time Lapsed in MSHR" $file_name |  awk '{print $7}' | tail -n $num_cpus | head -n ${core} | tail -n 1)
					#echo $avg_time_lapsed_in_mshr | tr -s '\n' ';'
                                        if [[ -n $avg_time_lapsed_in_mshr ]]; then
                                                echo $avg_time_lapsed_in_mshr | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
					fi 
				 elif [ $choice == 16 ]; then
                                        avg_time_in_mshr=$(grep -i "Average Time Lapsed in MSHR" $file_name | grep -v "Simulation" | tail -n $num_cpus | head -n $core | awk '{print $7}' | tail -n 1)
                                        if [[ -n $avg_time_in_mshr ]]; then
                                                echo $avg_time_in_mshr | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi



                                fi
				
		done
		done
		echo "" | tr -s '\n' ';'
#		echo "" | tr -s '\n' ';'
#		echo ";"
		done
#		echo ";"
		done
#			echo "" | tr -s '\n' ';'
 #                       echo "" | tr -s '\n' ';'

		done
	echo ";"
done
		
	done
done
done

