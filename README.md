# 2024 ICCAD Problem B

## Flow Brief
*   1. Cell Library Scoring
*   2. Debanking
*   3. GlobalPlacement with slack-driven conjugate-gradient placer
*   4. Legalize Single bit flops
*   5. Bank flops with graceful meanshift algorithm
*   6. GlobalPlacement with slack-driven conjugate-gradient placer
*   7. Legalize all MBFF
*   8. DetailPlacement

## Flow Chart
Should install Markdown Preview Mermaid Support in vscode

```mermaid
graph TD;
    Start-->Parser;
    Parser-->Cell_library_Scoring;
    Cell_library_Scoring-->Debanking_All_MBFF;
    Debanking_All_MBFF-->Pre-GlobalPlacement;
    Pre-GlobalPlacement-->Legalize_All_single-bit_FF;
    Legalize_All_single-bit_FF-->MeanShift_Clustering;
    MeanShift_Clustering-->Post-GlobalPlacement;
    Post-GlobalPlacement-->Legalize;
    Legalize-->DetailPlacement;
```

## Usage
Install Boost Package
```
$ sudo apt-get install libboost-all-dev
$ make boost
```

Compile
```
$ make or make -j
```

Run testcase
```
$ make run1
$ make run2
$ make run3
$ make run4
$ make run5
```

Valgrind
```
$ make check
```

Cppcheck
```
$ make cppcheck
```

## Doxygen
```
$ make view_doxygen
```