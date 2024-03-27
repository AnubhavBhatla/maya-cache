#!/bin/bash

if [ "$#" -lt 4 ]; then 
    echo "Illegal number of parameters"
    echo "Usage: ./run_1core_spec_.sh [BINARY] [N_WARM] [N_SIM] [TRACE_DIR] [OPTION]"
    exit 1
fi

BINARY=${1}
N_WARM=${2}
N_SIM=${3}
TRACE_DIR=$(pwd)/../traces/spec
OPTION=${4}

if [ -z $TRACE_DIR ]; then
    echo "[ERROR] Cannot find the trace directory: $TRACE_DIR"
    exit 0
fi

if [ ! -f "$BINARY" ]; then
    echo "[ERROR] Cannot find the ChampSim binary: $BINARY"
    exit 1
fi

re='^[0-9]+$'
if ! [[ $N_WARM =~ $re ]] || [ -z $N_WARM ]; then
    echo "[ERROR] Number of warmup instructions is NOT a number" >&2
    exit 1
fi

re='^[0-9]+$'
if ! [[ $N_SIM =~ $re ]] || [ -z $N_SIM ]; then
    echo "[ERROR] Number of simulation instructions is NOT a number" >&2
    exit 1
fi

cd $TRACE_DIR

mkdir -p $(pwd)/../results_$OPTION

for TRACE in *;
do
    if [ "$TRACE" != "wget-log" ]
    then
	name=${TRACE%.champsimtrace.xz}
	(./../${BINARY} -warmup_instructions ${N_WARM}000000 -simulation_instructions ${N_SIM}000000 -traces ${TRACE_DIR}/${TRACE}) > $(pwd)/../results_${OPTION}/${name}_${N_WARM}M_${N_SIM}M.txt &
    else
	continue
    fi
done
