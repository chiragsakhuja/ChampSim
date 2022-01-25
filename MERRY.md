# Trace merger
The tracer merger creates a merged trace and models a scheduler to select
instructions from each of the source traces.

## Building
From `tracer` directory:
`g++ -std=c++14 -O3 merge_traces.cpp -omerge_traces`

## Running directly (unadvised)
`merge_traces output-trace input-trace avg-inst-length [[input-trace
avg-inst-length] ...]`

# ChampSim
I've modified Champsim to include an "SMT Count", which defines how many threads
can be interleaved on a single core.  To do so, I added a parameter to the
default build script.

## Building
To build a version for the N-thread stat comparison script (run_interleave.sh),
run the following commands, which builds 1-to-N-thread versions of Champsim.
```
./build_champsim.sh perceptron no no no no lru 1 1
./build_champsim.sh perceptron no no no no lru 1 N
```

## Running an experiment with run_interleave.sh
I've provided a script, `run_interleave.sh` that runs a comparison experiment
with N traces (where Champsim must be built for each value 1..N).  The script
runs each source trace independently on the 1-thread build of Champsim and then
runs an interleaved trace on the N-thread build.  It also extracts the temporal
stats into a CSV file for each of the runs.  An example run is as follows:

`$PWD/run_interleave.sh example 1000000 100000000 100000 mcf_250B 50000 libquantum_1210B 10000`

The results of this run will be produced in `results/example`.

# Jupyter Notebook
I've provided a Jupyter notebook, analyze.ipynb, to visualize the comparison.
