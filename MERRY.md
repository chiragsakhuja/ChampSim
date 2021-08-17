# Merging traces
I've added the file tracer/merge_traces.cpp, which models a scheduler
(currently just random scheduling) to select instructions from each of the
source traces.

## Building
g++ -std=c++14 -O3 merge_traces.cpp -omerge_traces

## Running
merge_traces merged source [source ...]

The source traces must be in uncompressed form, and the output trace will be
produced in uncompressed form.

# Running ChampSim
For some reason ChampSim crashes on uncompressed traces, even though it's
supposed to support them.  All you need to do is gzip the trace before running.

I've modified src/main.cc to dump stats every 100000 cycles (see the commit
diff).  You can build and run like normal, and the stats are dumped in the text
file alongside other output.  You can extract the stats into a csv like so:

```
awk '/^stat,/ { print($0); }' $result.txt | cut -d','  -f2- > $result.csv
```

I've also included a Jupyter notebook, analyze.ipynb, to visualize the temporal
stats.
