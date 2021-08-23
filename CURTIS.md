1. Prove that fast context switching is better than idling out short stalls (and slow context switching)
2. Prove to Chris that the progress made by the switched-in process justifies the "pollution"

Trace merging (with parameterized randomization) should provide some
insight into these questions.

Chris' question is a specific `zoomed in' aspect of the overall inquiry, which places
value on the stalled thread's inertia and contextualizes the transient activity during
the stall as a harming, withering away, or otherwise deleterious effect.

It seems the two primary variables to be swept as are follows:
1. The length (in accesses? instructions?) of the interruptions.
2. The frequency of the interruptions.

Equivalently, the following:
1. The distribution of the duration of the interruptions.
2. The distribution of the durations between interruptions.

It may be tempting to consider only the ratio of these distributions, but the
durations themselves, when longer, should demonstrate that the value of
caching increases over time (to an asymptote, of course).

I plan to model both of these as exponential (memoryless) distributions.
>> This implementation is now complete.

It probably makes sense to use the following instruction count parameters:
 [0] { 100M, 10M, 1M }
 [1] { 100K, 10K, 1K }

This gives a 3x3 = 9 input matrix, sweeping from as much as 100M/1K ratio down to
a 1M/100K ratio.  (If the ranges overlapped, we would just end up testing the same
ratio again in reverse.)

However, our traces may not be this long; I think the main idea is that there
are some reasonable "long" periods with interruptions of comparatively "short"
periods that should hopefully help tell a story with the data.

