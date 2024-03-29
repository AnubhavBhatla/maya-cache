mkdir -p mirage/bin
mkdir -p maya/bin

echo "Building Baseline Single-Core-2MB"

cd mirage
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 1 > build.log
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-1core_llc bin/baseline_1core_2MB
cd ../

echo "Building Baseline Single-Core-2MB-8slices"

cd mirage
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 8 > build.log
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-8core_llc bin/baseline_1core_16MB_8slices
cd ../

echo "Building Baseline Eight-Core-16MB"

cd mirage
./build_champsim.sh bimodal ipcp ipcp no hawkeye 8 0 -2 0 1 1 3 0 1 1 > build.log
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-8core_mirage bin/baseline_8core_16MB
cd ../

echo "Building Mirage Single-Core-2MB"

cd mirage
sed -i.bak 's/\<MIRAGE 0\>/MIRAGE 1/g' inc/cache.h
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 1 > build.log
sed -i.bak 's/\<MIRAGE 1\>/MIRAGE 0/g' inc/cache.h
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-1core_llc bin/mirage_1core_2MB
cd ../

echo "Building Mirage Single-Core-2MB-8slices"

cd mirage
sed -i.bak 's/\<MIRAGE 0\>/MIRAGE 1/g' inc/cache.h
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 8 > build.log
sed -i.bak 's/\<MIRAGE 1\>/MIRAGE 0/g' inc/cache.h
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-8core_llc bin/mirage_1core_16MB_8slices
cd ../

echo "Building Mirage Eight-Core-16MB"

cd mirage
sed -i.bak 's/\<MIRAGE 0\>/MIRAGE 1/g' inc/cache.h
./build_champsim.sh bimodal ipcp ipcp no hawkeye 8 0 -2 0 1 1 3 0 1 1 > build.log
sed -i.bak 's/\<MIRAGE 1\>/MIRAGE 0/g' inc/cache.h
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-8core_mirage bin/mirage_8core_16MB
cd ../

echo "Building Maya Single-Core-2MB-8slices"

cd maya
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 8 > build.log
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-8core_maya bin/maya_1core_12MB_3way_8slices
cd ../

echo "Building Maya Eight-Core-16MB"

cd maya
./build_champsim.sh bimodal ipcp ipcp no hawkeye 8 0 -2 0 1 1 3 0 1 1 > build.log
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-8core_maya bin/maya_8core_12MB_3way
cd ../

