# 2024 ICCAD Problem B

*   refactor die class and FF class
*   think how to parse netlist structure

Current Process:

For each FF, construct a gaussain kernal function
For each comb cell, construct a quick down-stream function -> avoid FF move to there

Run gradient-ascent method to cluster the FF (with displacement small)

Legalizer + Placer to determine the FF or debanking the FF

為啥class FF跟class Gate不要繼承class Cell?
因為預期instance的數量會遠比cell library中的cell數量多，如果用繼承的會有很多一樣的資訊，且compiler會需要一直查表，所以用指標指向cell library的位置。若是merge到更大的multi-bit FF則僅需要更改指標指的位址，不需要更新一大堆資訊。

*   Compile
```
$ make or make -j
```

*   Run
```
$ make run
```

*   Valgrind
(need to fix the make pair issue...)
```
$ make check
```