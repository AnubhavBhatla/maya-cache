mkdir mirage/bin
mkdir maya/bin

echo "Building Baseline Single-Core-2MB"

cd mirage
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 1 > build.log
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-1core_llc bin/baseline_1core_2MB
cd ../

echo "Building Baseline Single-Core-2MB-8slices"

cd mirage
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 8 > build.log
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-8core_llc bin/baseline_1core_2MB_8slices
cd ../

echo "Building Baseline Eight-Core-2MB"

cd mirage
./build_champsim.sh bimodal ipcp ipcp no hawkeye 8 0 -2 0 1 1 3 0 1 1 > build.log
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-8core_mirage bin/baseline_8core_2MB
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
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-8core_llc bin/mirage_1core_2MB_8slices
cd ../

echo "Building Mirage Eight-Core-2MB"

cd mirage
sed -i.bak 's/\<MIRAGE 0\>/MIRAGE 1/g' inc/cache.h
./build_champsim.sh bimodal ipcp ipcp no hawkeye 8 0 -2 0 1 1 3 0 1 1 > build.log
sed -i.bak 's/\<MIRAGE 1\>/MIRAGE 0/g' inc/cache.h
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-8core_mirage bin/mirage_8core_2MB
cd ../

echo "Building Maya Eight-Core-2MB-1way"

cd maya
sed -i.bak 's/\<extra_tag_ways = 15\>/extra_tag_ways = 13/g' inc/cache.h
sed -i.bak 's/\<threshold_pr0 = 2048*6\>/threshold_pr0 = 2048*2/g' inc/cache.h
sed -i.bak 's/\<tag0[LLC_SET][15], tag1[LLC_SET][15]\>/tag0[LLC_SET][13], tag1[LLC_SET][13]/g' inc/cache.h
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 8 > build.log

echo "Building Maya Single-Core-2MB-1way-8slices"
./build_champsim.sh bimodal ipcp ipcp no hawkeye 8 0 -2 0 1 1 3 0 1 1 > build.log
sed -i.bak 's/\<tag0[LLC_SET][13], tag1[LLC_SET][13]\>/tag0[LLC_SET][15], tag1[LLC_SET][15]/g' inc/cache.h
sed -i.bak 's/\<threshold_pr0 = 2048*2\>/threshold_pr0 = 2048*6/g' inc/cache.h
sed -i.bak 's/\<extra_tag_ways = 13\>/extra_tag_ways = 15/g' inc/cache.h
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-8core_maya bin/maya_1core_1_5MB_1way_8slices
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-8core_maya bin/maya_8core_1_5MB_1way
cd ../

echo "Building Maya Eight-Core-2MB-3way"

cd maya
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 8 > build.log

echo "Building Maya Single-Core-2MB-3way-8slices"
./build_champsim.sh bimodal ipcp ipcp no hawkeye 8 0 -2 0 1 1 3 0 1 1 > build.log
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-8core_maya bin/maya_1core_1_5MB_3way_8slices
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-8core_maya bin/maya_8core_1_5MB_3way
cd ../

echo "Building Maya Eight-Core-2MB-5way"

cd maya
sed -i.bak 's/\<extra_tag_ways = 15\>/extra_tag_ways = 17/g' inc/cache.h
sed -i.bak 's/\<threshold_pr0 = 2048*6\>/threshold_pr0 = 2048*10/g' inc/cache.h
sed -i.bak 's/\<tag0[LLC_SET][15], tag1[LLC_SET][15]\>/tag0[LLC_SET][17], tag1[LLC_SET][17]/g' inc/cache.h
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 8 > build.log

echo "Building Maya Single-Core-2MB-5way-8slices"
./build_champsim.sh bimodal ipcp ipcp no hawkeye 8 0 -2 0 1 1 3 0 1 1 > build.log
sed -i.bak 's/\<tag0[LLC_SET][17], tag1[LLC_SET][17]\>/tag0[LLC_SET][15], tag1[LLC_SET][15]/g' inc/cache.h
sed -i.bak 's/\<threshold_pr0 = 2048*10\>/threshold_pr0 = 2048*6/g' inc/cache.h
sed -i.bak 's/\<extra_tag_ways = 17\>/extra_tag_ways = 15/g' inc/cache.h
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-8core_maya bin/maya_1core_1_5MB_5way_8slices
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-8core_maya bin/maya_8core_1_5MB_5way
cd ../

echo "Building Maya Eight-Core-2MB-5way"

cd maya
sed -i.bak 's/\<extra_tag_ways = 15\>/extra_tag_ways = 19/g' inc/cache.h
sed -i.bak 's/\<threshold_pr0 = 2048*6\>/threshold_pr0 = 2048*14/g' inc/cache.h
sed -i.bak 's/\<tag0[LLC_SET][15], tag1[LLC_SET][15]\>/tag0[LLC_SET][19], tag1[LLC_SET][19]/g' inc/cache.h
./build_champsim.sh bimodal ipcp ipcp no hawkeye 1 0 -1 0 1 1 3 0 1 8 > build.log

echo "Building Maya Single-Core-2MB-7way-8slices"
./build_champsim.sh bimodal ipcp ipcp no hawkeye 8 0 -2 0 1 1 3 0 1 1 > build.log
sed -i.bak 's/\<tag0[LLC_SET][19], tag1[LLC_SET][19]\>/tag0[LLC_SET][15], tag1[LLC_SET][15]/g' inc/cache.h
sed -i.bak 's/\<threshold_pr0 = 2048*14\>/threshold_pr0 = 2048*6/g' inc/cache.h
sed -i.bak 's/\<extra_tag_ways = 19\>/extra_tag_ways = 15/g' inc/cache.h
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-1core-8core_maya bin/maya_1core_1_5MB_7way_8slices
mv bin/bimodal-ipcp-ipcp-no-hawkeye-0-1-1-3-0-1-8core_maya bin/maya_8core_1_5MB_7way
cd ../