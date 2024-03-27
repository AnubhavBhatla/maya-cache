sim_instr=50000000
#if [ $3 == 0 ]; then
#name=${1}_${2}_Slices_4KB_page
#else
#name=${1}_${2}_Slices_2MB_page
#fi
name=${1}
gcc -O0 ${1}.c -g -o $name

pin -ifeellucky -t tracer/obj-intel64/champsim_tracer.so -s 50000000 -t ${sim_instr} -o ${name}.trace -- ./"$name" ${2} ${3}
gzip "${name}".trace
rm ${name}.trace
mv "${name}".trace.gz /home/sujeet/Study/Study/Thesis/EnclaveSim_for_multicore/EnclaveSim/tracer/
echo ${name}
#rm -f ${name}

