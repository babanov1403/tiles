# How to run


```bash
# Build tools:
bazel build //tools:all

# Run binary:
./bazel-bin/tools/comp_metrics 

# Generate compile_commands.json for your IDE:
bazel run @hedron_compile_commands//:refresh_all
```

# Results

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Running strategy for RandomStrategy
Total tiles included is 46323081
Total bytes included is 17179869184
Total visits among tiles is 300361160
Paged metric is: 1811.39 Mb/s
Unpaged metric is: 898.144 Mb/s
Ideal metric is: 1748.73 Mb/s

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Running strategy for SectorStrategy
percentile_pos: 54896397
starting..
middle is: 267517813
min_visits: 0
0<---------
Parts of array after splitting by visits:
boring: 0.974628 | interesting: 0.0253721
boring: 267517813 | interesting: 6964174
Tiles in RAM: 751600 Not in RAM: 6212574
middle is: 3929422
small: 3929422 | big: 2283152

Total tiles used for padding: 3025621
We have 903801 left from filling empty spaces
this will cause 732615 more pages
751600
We used additional 1812.37 megabytes :)
Total tiles included is 751599
Total bytes included is 17179869184
Total visits among tiles is 1413107750
Paged metric is: 78.5456 Mb/s
Unpaged metric is: 39.7774 Mb/s
Ideal metric is: 75.3283 Mb/s

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Running strategy for AlignStategy
We used additional 13013.8mb
Total tiles included is 751615
Total bytes included is 17179869184
Total visits among tiles is 1413112406
Paged metric is: 75.3279 Mb/s
Unpaged metric is: 39.7772 Mb/s
Ideal metric is: 75.3279 Mb/s

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Running strategy for GreedyStrategy
Total tiles included is 751615
Total bytes included is 17179869184
Total visits among tiles is 1413112406
Paged metric is: 88.8567 Mb/s
Unpaged metric is: 39.7772 Mb/s
Ideal metric is: 75.3279 Mb/s

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Running strategy for GreedyScaledStrategy
Total tiles included is 3177239
Total bytes included is 17179869184
Total visits among tiles is 1547313203
Paged metric is: 141.951 Mb/s
Unpaged metric is: 70.0461 Mb/s
Ideal metric is: 132.735 Mb/s

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Running strategy for GreedyNoRearrangeStrategy
Total tiles included is 722142
Total bytes included is 17179869184
Total visits among tiles is 1404336579
Paged metric is: 86.7329 Mb/s
Unpaged metric is: 39.5551 Mb/s
Ideal metric is: 69.9072 Mb/s

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
