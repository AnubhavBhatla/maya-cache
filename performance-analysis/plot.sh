#!/bin/bash

OLD=$1

cd scripts/

python3 get_data.py ${OLD}
python3 get_plots.py ${OLD}

mv Fig1.pdf ../Fig1.pdf
mv Fig10.pdf ../Fig10.pdf

cd ..

rm scripts/*.csv
