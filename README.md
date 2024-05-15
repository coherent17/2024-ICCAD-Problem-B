# 2024 ICCAD Problem B

*   think how to setup netlist structure
*   need to refactor net class(also create a pin class?)
*   input preplace FF may also be a multi-bit ff(not single-bit only), appropriate debanking algorithm is needed...


Current Process:

1. Eliminate possible negative slack
The preplace FF(MBFF) may also have negative slack, however, most of the algorithm on FF clustering are mainly focus on min timing degradation(min displacement). Should we first model the timing path(2 HPWL with 1 Qpin delay) for the FF as the differential expression(eg: ntuplace wirelength log sum exponential model), and run gradient method to move the FF to eliminate the negative slack?

2. Run any kind of cluster algorithm(min displacement)
For graceful cluster:
    *   For each FF, construct a gaussain kernal function
    *   For each comb cell, construct a quick down-stream function -> avoid FF move to there
    *    Run gradient-ascent method to cluster the FF (with displacement small)

3. Legalizer + Placer to determine the FF or debanking the FF

為啥class FF跟class Gate不要繼承class Cell?
因為預期instance的數量會遠比cell library中的cell數量多，如果用繼承的會有很多一樣的資訊，且compiler會需要一直查表，所以用指標指向cell library的位置。若是merge到更大的multi-bit FF則僅需要更改指標指的位址，不需要更新一大堆資訊。

*   Compile
```
$ make or make -j
```

*   Run
```
$ make run1
$ make run2
```

*   Valgrind
(need to fix the make pair issue...)
```
$ make check
```

*   Discussion Log:
0515:
    *   Net structure? Should be convenient & efficient for calculating the slack
    *   Negative slack elimination??