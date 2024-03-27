IS_HUGE_PAGE=0
IS_DIVIDED_QUEUES=-2
traces=(605.mcf_s-1554B.champsimtrace.xz)
traces1=(600.perlbench_s-210B.champsimtrace.xz	644.nab_s-12459B.champsimtrace.xz	657.xz_s-3167B.champsimtrace.xz	621.wrf_s-8065B.champsimtrace.xz
	654.roms_s-293B.champsimtrace.xz
	607.cactuBSSN_s-2421B.champsimtrace.xz
	649.fotonik3d_s-1176B.champsimtrace.xz
	603.bwaves_s-891B.champsimtrace.xz
	649.fotonik3d_s-7084B.champsimtrace.xz 	623.xalancbmk_s-202B.champsimtrace.xz
	654.roms_s-1390B.champsimtrace.xz
	619.lbm_s-2676B.champsimtrace.xz
	605.mcf_s-782B.champsimtrace.xz	605.mcf_s-1554B.champsimtrace.xz
	602.gcc_s-2226B.champsimtrace.xz )
traces2=(600.perlbench_s-1273B.champsimtrace.xz  620.omnetpp_s-874B.champsimtrace.xz    644.nab_s-5853B.champsimtrace.xz
600.perlbench_s-210B.champsimtrace.xz   621.wrf_s-575B.champsimtrace.xz        644.nab_s-7928B.champsimtrace.xz
600.perlbench_s-570B.champsimtrace.xz   621.wrf_s-6673B.champsimtrace.xz       644.nab_s-9322B.champsimtrace.xz
602.gcc_s-1850B.champsimtrace.xz        621.wrf_s-8065B.champsimtrace.xz       648.exchange2_s-1227B.champsimtrace.xz
602.gcc_s-2226B.champsimtrace.xz        621.wrf_s-8100B.champsimtrace.xz       648.exchange2_s-1247B.champsimtrace.xz
602.gcc_s-2375B.champsimtrace.xz        623.xalancbmk_s-10B.champsimtrace.xz   648.exchange2_s-1511B.champsimtrace.xz
602.gcc_s-734B.champsimtrace.xz         623.xalancbmk_s-165B.champsimtrace.xz  648.exchange2_s-1699B.champsimtrace.xz
603.bwaves_s-1080B.champsimtrace.xz     623.xalancbmk_s-202B.champsimtrace.xz  648.exchange2_s-1712B.champsimtrace.xz
603.bwaves_s-1740B.champsimtrace.xz     623.xalancbmk_s-325B.champsimtrace.xz  648.exchange2_s-210B.champsimtrace.xz
603.bwaves_s-2609B.champsimtrace.xz     623.xalancbmk_s-592B.champsimtrace.xz  648.exchange2_s-353B.champsimtrace.xz
603.bwaves_s-2931B.champsimtrace.xz     623.xalancbmk_s-700B.champsimtrace.xz  648.exchange2_s-387B.champsimtrace.xz
603.bwaves_s-3699B.champsimtrace.xz     625.x264_s-12B.champsimtrace.xz        648.exchange2_s-584B.champsimtrace.xz
603.bwaves_s-5359B.champsimtrace.xz     625.x264_s-18B.champsimtrace.xz        648.exchange2_s-72B.champsimtrace.xz
603.bwaves_s-891B.champsimtrace.xz      625.x264_s-20B.champsimtrace.xz        649.fotonik3d_s-10881B.champsimtrace.xz
605.mcf_s-1152B.champsimtrace.xz        625.x264_s-33B.champsimtrace.xz        649.fotonik3d_s-1176B.champsimtrace.xz
605.mcf_s-1536B.champsimtrace.xz        625.x264_s-39B.champsimtrace.xz        649.fotonik3d_s-1B.champsimtrace.xz
605.mcf_s-1554B.champsimtrace.xz        627.cam4_s-490B.champsimtrace.xz       649.fotonik3d_s-7084B.champsimtrace.xz
605.mcf_s-1644B.champsimtrace.xz        627.cam4_s-573B.champsimtrace.xz       649.fotonik3d_s-8225B.champsimtrace.xz
605.mcf_s-472B.champsimtrace.xz         628.pop2_s-17B.champsimtrace.xz        654.roms_s-1007B.champsimtrace.xz
605.mcf_s-484B.champsimtrace.xz         631.deepsjeng_s-928B.champsimtrace.xz  654.roms_s-1021B.champsimtrace.xz
605.mcf_s-665B.champsimtrace.xz         638.imagick_s-10316B.champsimtrace.xz  654.roms_s-1070B.champsimtrace.xz
605.mcf_s-782B.champsimtrace.xz         638.imagick_s-4128B.champsimtrace.xz   654.roms_s-1390B.champsimtrace.xz
605.mcf_s-994B.champsimtrace.xz         638.imagick_s-824B.champsimtrace.xz    654.roms_s-1613B.champsimtrace.xz
607.cactuBSSN_s-2421B.champsimtrace.xz  641.leela_s-1052B.champsimtrace.xz     654.roms_s-293B.champsimtrace.xz
607.cactuBSSN_s-3477B.champsimtrace.xz  641.leela_s-1083B.champsimtrace.xz     654.roms_s-294B.champsimtrace.xz
607.cactuBSSN_s-4004B.champsimtrace.xz  641.leela_s-149B.champsimtrace.xz      654.roms_s-523B.champsimtrace.xz
607.cactuBSSN_s-4248B.champsimtrace.xz  641.leela_s-334B.champsimtrace.xz      654.roms_s-842B.champsimtrace.xz
619.lbm_s-2676B.champsimtrace.xz        641.leela_s-602B.champsimtrace.xz      657.xz_s-2302B.champsimtrace.xz
619.lbm_s-2677B.champsimtrace.xz        641.leela_s-800B.champsimtrace.xz      657.xz_s-3167B.champsimtrace.xz
619.lbm_s-3766B.champsimtrace.xz        641.leela_s-862B.champsimtrace.xz      657.xz_s-4994B.champsimtrace.xz
619.lbm_s-4268B.champsimtrace.xz        644.nab_s-12459B.champsimtrace.xz      657.xz_s-56B.champsimtrace.xz
620.omnetpp_s-141B.champsimtrace.xz     644.nab_s-12521B.champsimtrace.xz
)
traces3=( 620.omnetpp_s-874B.champsimtrace.xz
621.wrf_s-6673B.champsimtrace.xz
621.wrf_s-8065B.champsimtrace.xz
602.gcc_s-2226B.champsimtrace.xz
602.gcc_s-734B.champsimtrace.xz
623.xalancbmk_s-165B.champsimtrace.xz
623.xalancbmk_s-202B.champsimtrace.xz
603.bwaves_s-2931B.champsimtrace.xz
603.bwaves_s-891B.champsimtrace.xz
649.fotonik3d_s-10881B.champsimtrace.xz
605.mcf_s-1554B.champsimtrace.xz
627.cam4_s-490B.champsimtrace.xz
649.fotonik3d_s-7084B.champsimtrace.xz
627.cam4_s-573B.champsimtrace.xz
628.pop2_s-17B.champsimtrace.xz
605.mcf_s-782B.champsimtrace.xz
607.cactuBSSN_s-2421B.champsimtrace.xz
654.roms_s-293B.champsimtrace.xz
654.roms_s-294B.champsimtrace.xz
607.cactuBSSN_s-4248B.champsimtrace.xz
619.lbm_s-2677B.champsimtrace.xz
619.lbm_s-4268B.champsimtrace.xz
620.omnetpp_s-141B.champsimtrace.xz )

CEASER_LATENCY=0 
CEASER_rq_wq_ratio=21
L1D_PREFETCHER=(no)  #(ipcp)      #(ip_stride)
L2C_PREFETCHER=(no) #(ipcp) #(bingo_dpc3)
LLC_PREFETCHER=(no)
policy=(srrip)  #(srrip)
policies=(ship) # ship ship_drishyam ship++ srrip lru) #   (ship ship_drishyam)    #  ( hawkeye  ship ship_drishyam srrip lru drrip)
num_cpus=16
remap=1
LLC_SIZE=16
CEASER_S_LLC=0
pipelined_encryption_engine=1
remapping_on_eviction=1
#Trace_Dir=/home/sujeet/SPEC_2017/tracer
Trace_Dir=/home/sujeet/spec2017_traces
traces5=(tc_70B.trace.gz
canneal_44B.trace.gz
bf_94B.trace.gz
pr_1B.trace.gz
Components_70B.trace.gz
Radii_100B.trace.gz
MIS_85B.trace.gz
BC_74350M.champsim.gz
BFS_61B.trace.gz
sssp-5.trace.gz)

OPTIONS=(with_MIRAGE) #(with_remap_find_victim_with_remap_update_with_multi_step)   #(with_multi_step)
count=0
for trace in ${traces[*]}          
do
for policy in ${policies[*]}
do 
for partitions in 1     #4 6 8 10 12 14 16 18 20 21  #100 50 20 10 5 2 
do
   for CEASER_S_LLC in 0     #1
   do
	for remap in 1 
	do
	war=1
	sim=1
	if [ $count -ne 15 ];
        then
	 ./build_champsim.sh hashed_perceptron ${L1D_PREFETCHER} ${L2C_PREFETCHER} ${LLC_PREFETCHER} $policy ${num_cpus} ${IS_HUGE_PAGE} ${IS_DIVIDED_QUEUES}  ${CEASER_S_LLC} ${partitions} ${pipelined_encryption_engine} ${CEASER_LATENCY} ${remap} ${remapping_on_eviction} ${LLC_SIZE}	
	
	#./run_champsim.sh hashed_perceptron-${L1D_PREFETCHER}-${L2C_PREFETCHER}-no-${policy}-${CEASER_S_LLC}-${partitions}-${pipelined_encryption_engine}-${CEASER_LATENCY}-${remap}-${remapping_on_eviction}-1core-${LLC_SIZE}core_llc $war $sim ${Trace_Dir} ${trace} ${OPTIONS}  & 

       	./build_champsim.sh hashed_perceptron ipcp ipcp no $policy ${num_cpus} ${IS_HUGE_PAGE} ${IS_DIVIDED_QUEUES}  ${CEASER_S_LLC} ${partitions} ${pipelined_encryption_engine} ${CEASER_LATENCY} ${remap} ${remapping_on_eviction} ${LLC_SIZE}  

         #./run_champsim.sh hashed_perceptron-ipcp-ipcp-no-${policy}-${CEASER_S_LLC}-${partitions}-${pipelined_encryption_engine}-${CEASER_LATENCY}-${remap}-${remapping_on_eviction}-1core-${LLC_SIZE}core_llc $war $sim ${Trace_Dir} ${trace} ${OPTIONS}  &
	 
	 count=`expr $count + 1`
	  else
		  ./build_champsim.sh hashed_perceptron ${L1D_PREFETCHER} ${L2C_PREFETCHER} ${LLC_PREFETCHER} $policy ${num_cpus} ${IS_HUGE_PAGE} ${IS_DIVIDED_QUEUES}  ${CEASER_S_LLC} ${partitions} ${pipelined_encryption_engine} ${CEASER_LATENCY} ${remap} ${remapping_on_eviction} ${LLC_SIZE}

        #	./run_champsim.sh hashed_perceptron-${L1D_PREFETCHER}-${L2C_PREFETCHER}-no-${policy}-${CEASER_S_LLC}-${partitions}-${pipelined_encryption_engine}-${CEASER_LATENCY}-${remap}-${remapping_on_eviction}-1core-${LLC_SIZE}core_llc $war $sim ${Trace_Dir} ${trace} ${OPTIONS}  
	 count=0
	 ./build_champsim.sh hashed_perceptron ipcp ipcp no $policy ${num_cpus} ${IS_HUGE_PAGE} ${IS_DIVIDED_QUEUES}  ${CEASER_S_LLC} ${partitions} ${pipelined_encryption_engine} ${CEASER_LATENCY} ${remap} ${remapping_on_eviction} ${LLC_SIZE}

         #./run_champsim.sh hashed_perceptron-ipcp-ipcp-no-${policy}-${CEASER_S_LLC}-${partitions}-${pipelined_encryption_engine}-${CEASER_LATENCY}-${remap}-${remapping_on_eviction}-1core-${LLC_SIZE}core_llc $war $sim ${Trace_Dir} ${trace} ${OPTIONS}  &

        fi


 done
done
done
done
done
