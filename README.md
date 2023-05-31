[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.7986572.svg)](https://doi.org/10.5281/zenodo.7986572)

# HeiTwin-Heuristic
HeiTwin-Heuristic is a heuristic solver for the twin-width problem. It takes part in the [PACE Challenge 2023](https://pacechallenge.org/2023/).

## Description
HeiTwin-Heuristic implements a greedy sliding window algorithm. First the `k` first nodes of the graph are considered. From these `k` nodes those two are contracted, whose contraction leads to the least red degree at any node in the neighborhood. Then, the next `k` nodes are considered. If the last nodes of the graph are reached, the algorithm starts over with the first nodes again, until the graph is contracted.

This strategy can use different node orderings, which will lead to very different results. The natural order, as well as a BFS ordering, turned out to lead to a low twin width.

To make use of all available time, `k` is incremented iteratively. A higher `k` usually leads to a lower twin width, although this is not the case for all graphs. As it is additionally difficult to predict the running time of a specific choice of `k` (it depends on the graph, including size, density, twin width, and also the quality of the node ordering), increasing `k` iteratively turned out to be efficient.

For small graphs a different strategy is used first. Instead of `k`, a maximum allowed red degree (and therefore twin-width) is incremented periodically. After each step, all contractions that do not lead to a red degree higher than this limit are done. This more global strategy yields better results The more local first approach, but as one iteration has quadratic complexity, this can only be used for small (or very sparse) graphs.

## Build and run
The project can be build using `cmake`, e.g.

```cmake -Bbuild . && cd build && make```

A binary `HeiTWin` will be created, that reads a graph from stdin and writes the solution to stdout. For more details on the input and output format look at the [PACE Challenge definition](https://pacechallenge.org/2023/io/).
