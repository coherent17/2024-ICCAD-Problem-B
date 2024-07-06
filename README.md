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

*   Add Boost Package
```
$ make boost
```

*   Compile
```
$ make or make -j
```

*   Run
```
$ make run1
$ make run2
$ make run3
$ make run4
```

*   Valgrind
```
$ make check
```

*   Cppcheck
```
$ make cppcheck
```

*   Discussion Log:
0515:
    *   Net structure? Should be convenient & efficient for calculating the slack
    *   Negative slack elimination??


* update log:
    * Finish debanking, but need logic FF naming rule(?). -> solved
    * Get name function. (in util using counter, prefix = "FF_"). -> solved
    * prev FF pointer currently fixed point to largest initial HPWL. -> solved
    * Parser check whether to run preprocessing. (gradient to get all slack positive).

* log 06/17:
    * Output floating problem.
    * FF input doesn't come from IO or other FF, ie come from std cell and this std cell is floating input.

* log 0625 init cluster and add script to download boost, run $ make boost first before compile the code...
    * Cheng help to check if the newest testcase still have output floating problem...
    * MeanShift: buildRtree > KNN > shiftFF
    * compile flags, all program should be static-link, otherwise, the program can't be run on TSRI server, see detailed in makefile


* log 0706
    * still have floating input/output in testcase1_0614
    * the result after gradient to find optimal location is worse, should consider slack as weight on gradient.
    
Assign c119cheng:
*   Debanking, finish
*   Update Instance::coor for OptimalLocation.