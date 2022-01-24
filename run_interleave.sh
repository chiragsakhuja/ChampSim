#!/bin/bash

# Example usage:
# $PWD/run.sh dist-samples 50000000 250000000 1000000 mcf_250B 1000000 libquantum_1210B 100000
# 
# Results will be in results/dist-samples/mcf_250B_1000000_libquantum_1210B_100000
#   - mcf_250B.txt : ChampSim output for individual run of mcf
#   - mcf_250B.csv : Temporal stat dump of mcf
#   - libquantum_1210B.txt : ChampSim output for individual run of libquantum
#   - libquantum_1210B.csv : Temporal stat dump of libquantum
#   - merged.txt : ChampSim output for interleaved threads
#   - merged.csv : Temporal stat dump of interleaved threads

if [[ $# -lt 5 ]]; then
    echo "Usage: $0 base-dir warmup-inst sim-inst stat-period [[trace duation] ...]"
    exit 0
fi

BASEDIR=$1
WARMUP_INSTR=$2
SIM_INSTR=$3
TEMPORAL_STAT_PERIOD=$4
shift 4

TRACES=()
TRACE_INSTR=()
if [ $(expr $# % 2) != 0 ]; then
    echo "Usage: $0 base-dir warmup-inst sim-inst stat-period [[trace duation] ...]"
    exit 0
fi

while [[ $# != 0 ]]; do
    # Use traces from Akanksha's directory
    TRACES+=("/scratch/cluster/akanksha/CRCRealTraces/$1.trace.gz")
    TRACE_INSTR+=($2)
    shift 2
done

NUM_THREADS=${#TRACES[@]}

MERGE_TRACES_CMD="./tracer/merge_traces /dev/stdout"
# Build merge_traces command and result directory name
for i in $(seq 0 $((NUM_THREADS-1))); do
    # Get trace base name from trace path
		TRACENAME="${TRACES[i]##*/}"
		TRACENAME="${TRACENAME%.*}"
		TRACENAME="${TRACENAME%.*}"
		MERGE_TRACES_CMD="$MERGE_TRACES_CMD <(gunzip -cd ${TRACES[i]}) ${TRACE_INSTR[i]}"
		RESULT_DIR="$RESULT_DIR""_$TRACENAME""_${TRACE_INSTR[i]}"
done

# Remove first character of RESULT_DIR because it will be a _
RESULT_DIR="results/$BASEDIR/${RESULT_DIR:1}"
mkdir -p $RESULT_DIR

CHAMPSIM_BASE="./bin/perceptron-no-no-no-no-lru-1core"
CHAMPSIM_ARGS="-temporal_stat_period $TEMPORAL_STAT_PERIOD -warmup_instructions $WARMUP_INSTR -simulation_instructions $SIM_INSTR"

# Run each trace individually and extract stats
for TRACE in "${TRACES[@]}"; do
		TRACENAME="${TRACE##*/}"
		TRACENAME="${TRACENAME%.*}"
		TRACENAME="${TRACENAME%.*}"

		COMMAND="$CHAMPSIM_BASE-1thread $CHAMPSIM_ARGS -traces $TRACE > $RESULT_DIR/$TRACENAME.txt"
    echo "$COMMAND"
		eval "$COMMAND"
    grep '^stat' "$RESULT_DIR/$TRACENAME.txt" > "$RESULT_DIR/$TRACENAME.csv"
done

# Run merged trace and extract stats and PID order
COMMAND="$MERGE_TRACES_CMD | $CHAMPSIM_BASE-$NUM_THREADS""thread $CHAMPSIM_ARGS -stdin > $RESULT_DIR/merged.txt"
echo "$COMMAND"
eval "$COMMAND"
grep '^stat' "$RESULT_DIR/merged.txt" > "$RESULT_DIR/merged.csv"
awk 'BEGIN { print("cycle,pid"); first=1; }; /Switching/ { if(first != 1) { printf("%d,%d\n", $9, $7); } else { first=0; } }' "$RESULT_DIR/merged.txt" > "$RESULT_DIR/pid.txt"
