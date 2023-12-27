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

CEASER_S_LLC=1
CEASER_LATENCY=5  #8
#mixes=(0-8 4-4 7-1)
mixes=(7-1)
num_cpus=8
policy=srrip
war=1
sim=1
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
traces=(605.mcf_s-1152B.champsimtrace.xz  605.mcf_s-1644B.champsimtrace.xz  605.mcf_s-665B.champsimtrace.xz 605.mcf_s-1536B.champsimtrace.xz 605.mcf_s-472B.champsimtrace.xz   605.mcf_s-782B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz  605.mcf_s-484B.champsimtrace.xz   605.mcf_s-994B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 627.cam4_s-573B.champsimtrace.xz 621.wrf_s-8065B.champsimtrace.xz 619.lbm_s-4268B.champsimtrace.xz 619.lbm_s-2677B.champsimtrace.xz 619.lbm_s-2676B.champsimtrace.xz 623.xalancbmk_s-10B.champsimtrace.xz 603.bwaves_s-891B.champsimtrace.xz 654.roms_s-1390B.champsimtrace.xz 600.perlbench_s-570B.champsimtrace.xz 657.xz_s-2302B.champsimtrace.xz)
#traces=(605.mcf_s-1152B.champsimtrace.xz  605.mcf_s-1644B.champsimtrace.xz  605.mcf_s-665B.champsimtrace.xz 605.mcf_s-1536B.champsimtrace.xz 605.mcf_s-472B.champsimtrace.xz   605.mcf_s-782B.champsimtrace.xz 605.mcf_s-1554B.champsimtrace.xz  605.mcf_s-484B.champsimtrace.xz   605.mcf_s-994B.champsimtrace.xz 602.gcc_s-2226B.champsimtrace.xz 627.cam4_s-573B.champsimtrace.xz 621.wrf_s-8065B.champsimtrace.xz 619.lbm_s-4268B.champsimtrace.xz 619.lbm_s-2677B.champsimtrace.xz 619.lbm_s-2676B.champsimtrace.xz 623.xalancbmk_s-10B.champsimtrace.xz 603.bwaves_s-891B.champsimtrace.xz 654.roms_s-1390B.champsimtrace.xz)
#traces=(605.mcf_s-1644B.champsimtrace.xz)
#traces=(605.mcf_s-1554B.champsimtrace.xz 603.bwaves_s-1080B.champsimtrace.xz 607.cactuBSSN_s-4248B.champsimtrace.xz)
pref_combinations=(ipcp-ipcp-no) # next_line-ip_stride-no ipcp-ipcp-no next_line-no-no)
CEASER_LLC=0
for NUM_SET in 1
do
for trace in ${traces[*]}
do
for mix in ${mixes[*]}
do
   for core in 1 2 3 4 5 6 7 8
   do
	for pref_combination in ${pref_combinations[*]}
	do
		
		 for choice in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15  #16 
 		 do
		  for CEASER_S_LLC in 0 1
		  do
			
                IFS='-' # hyphen (-) is set as delimiter
                read -ra ADDR <<< "${pref_combination}" # string is read into an array as tokens separated by IFS
                L1D_PREFETCHER=${ADDR[0]}
                L2C_PREFETCHER=${ADDR[1]}
                LLC_PREFETCHER=${ADDR[2]}
                IFS=""
		if [ $is_divided_queues == -2 ]; then
				if [ $is_huge_page == 1 ]; then
					results_folder=Scatter_cache_Wih_pref_8_core_without_randomization_with_remap_single_port_5latency_25_llc_latency_/results_2MB_page/results_${num_cpus}core_${trace}/${NUM_SET}_Sets  #Experiment2_16_core_read_intensive Without_remap_8_core_read_intensive
				
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



				file_name=$results_folder/mix${mix}-bimodal-${L1D_PREFETCHER}-${L2C_PREFETCHER}-${LLC_PREFETCHER}-${policy}-${CEASER_LLC}-${CEASER_S_LLC}-${CEASER_LATENCY}-21-${num_cpus}core.txt
				
				#mul=1000

				#IPC=$(grep IPC $file_name | tail -n $num_cpus | awk '{print $5}' | head -n ${core} | tail -n 1)
				#inst=$(grep instructions $file_name | tail -n $num_cpus | awk '{print $7}' | head -n ${core} | tail -n 1)
				#miss=$(grep MISS $file_name | grep LLC | grep LOAD | tail -n $num_cpus | awk '{print $8}' | head -n ${core} | tail -n 1 )
				#miss=$(echo "scale=2; $miss*$mul" | bc -l )
				#MPKI=$(echo "scale=2; $miss/$inst" | bc -l )
			

			  	
				if [ $choice == 1 ]; then
	                                IPC=$(grep IPC $file_name | grep -v "Simulation" | tail -n $num_cpus | awk '{print $5}' | head -n ${core} | tail -n 1)
					if [[ -n $IPC ]]; then
						echo $IPC | tr -s '\n' ';'
					else
						echo "-" | tr -s '\n' ';'
					fi
					
					#echo $IPC | tr -s '\n' ';'
		
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
					AVG_MISS_PENALTY=$(grep -i "L1D AVERAGE MISS LATENCY:" $file_name | grep -v "Simulation" | tail -n $num_cpus | head -n $core | awk '{print $5}' | tail -n 1)
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
                                        AVG_MISS_PENALTY=$(grep -i "L2C AVERAGE MISS LATENCY:" $file_name | grep -v "Simulation" | tail -n $num_cpus | head -n $core | awk '{print $5}' | tail -n 1)
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

                                        AVG_MISS_PENALTY=$(grep -i "TOTAL     ACCESS:" $file_name | grep LLC | grep -v "Simulation" | tail -n $llc_lines | head -n $rel_llc_lines |  awk '{print $18}' | tail -n ${num_cpus} | awk '{ SUM += $1} END { print SUM }')

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

				
                                elif [ $choice == 11 ]; then
                                        rob_stall_cycles=$(grep -i "ROI Average ROB Stall Cycles:" $file_name | awk '{print $6}' | tail -n $num_cpus | head -n $core | tail -n 1)
					if [[ -n $rob_stall_cycles ]]; then
                                        	echo $rob_stall_cycles | tr -s '\n' ';'
                                        else
                                                echo "-" | tr -s '\n' ';'
                                        fi

				elif [ $choice == 12 ]; then
                                        avg_load_rob_stall_cycles=$(grep -i "ROI Average Load ROB Stall Cycles:" $file_name | awk '{print $7}' | tail -n $num_cpus | head -n $core | tail -n 1)
                                        if [[ -n $avg_load_rob_stall_cycles ]]; then
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
                        echo "" | tr -s '\n' ';'

		done
	echo ";"
done
		
	done
done
done

