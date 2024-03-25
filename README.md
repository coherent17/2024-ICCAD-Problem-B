# 2024 ICCAD Problem B

## 1. Introduction
Using multibit flip-flops (FFs) to replace multiple single-bit FFs offers several advantages:

* Reduction in area
* Decrease in power consumption
* Potential timing issues (when converting multibit to single bit, known as debanking)

## 2. Contest Objective
For the ICCAD 2024 problem B, we need to consider the following objectives:

1. Timing
2. Power
3. Area
4. No cell overlapping

Cost function:

$$ 
\begin{gather*}
    \sum_{\forall i \in FF} (\alpha \cdot TNS(i)+\beta\cdot Power(i)+\gamma \cdot Area(i)) + \delta \cdot D
\end{gather*}
$$

where
$$
\left\{
    \begin{matrix}
        TNS(i) &=& total \ negative \ slack\\
        D &=& \#bin \ violate \ util \\
        \alpha &=& TNS \ weight \\
        \beta &=& Power \ weight \\
        \gamma &=& Area \ weight \\
        \delta &=& Violate \ weight
    \end{matrix}
\right.
$$

For each bin, can not exceed density constraint. (BinMaxUtil). Or might lose some score due to penality.

![image](https://hackmd.io/_uploads/HJk1u5A0p.png)







## 4. Solution Flow?
Prototype of the algorithm flow:
* Partitioning (constraint with hmetis)
* Running tasks in parallel using Taskflow with analytical placement (modified force-directed or merge ntuplace k-means?)
* Sub-gradient optimization for the undifferentiable cost function

We need to discuss optimization techniques for merging communication between different partitions.

## 5. Reference Paper

1. Graceful Register Clustering by Effective Mean Shift Algorithm for Power and Timing Balancing.

    [Paper] (https://waynelin567.github.io/files/meanshift.pdf)
    
    [Github] (https://github.com/waynelin567/Register_Clustering)

2.  Flip-flop clustering by weighted K-means algorithm
    [Paper] https://home.engineering.iastate.edu/~cnchu/pubs/c88.pdf