# 2024 ICCAD Problem B

An IU a day keeps the bad code away.
Contributed by Cheng
![iu](https://hackmd.io/_uploads/Hy8f4Ec1R.gif)
[TOC]





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

Timing slack need to consider both displacement delay & Q-Pin delay.
(Try not to move FF far away...)

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

切bin是為了估placement density，才可以去預估routing difficulty...

![image](https://hackmd.io/_uploads/HJk1u5A0p.png)


## 4. Solution Flow?
Prototype of the algorithm flow:
* Partitioning (constraint with hmetis)
* Running tasks in parallel using Taskflow with analytical placement (modified force-directed or merge ntuplace k-means?)
* Sub-gradient optimization for the undifferentiable cost function

We need to discuss optimization techniques for merging communication between different partitions.

*    Different partition banking/debanking algorithm
*    FF goes to different partition
*    MBFF #bits = 2^n?
## 5. Reference Paper

### 0. Slack Redistributed Register Clustering with Mixed-Driving Strength Multi-bit Flip-Flops

[Paper](https://dl.acm.org/doi/pdf/10.1145/3626184.3633327)
    
[PPT](https://ispd.cc/slides/2024/protected/2_3_slides_final.pdf)
    
[Video](https://www.youtube.com/watch?v=QHa1iKkk0uo&list=PL6-vor2YamECt-qVQRoyxEXguR2RmP-kF&index=3)

* 演算法流程
![image](https://hackmd.io/_uploads/HyfsYmiy0.png)
藍色部分為此篇的創新演算法

1. Window-Based Adaptive Interval Graph Construction

    為了避免某些FF的feasible region較大造成cluster後的displacement過大在最開始先設定window大小由左至右、由下至上掃過一整個chip
    ![image](https://hackmd.io/_uploads/BkYwn7oJ0.png)

2. Coordinate transform
    
    先計算在window內的FFs的feasible region，若有overlap則代表有機會cluster，可以將feaible region overlap的關聯性變成intersection graph
    ![image](https://hackmd.io/_uploads/ryCuCQsyC.png)
    此intersection graph的關係可以用X,Y的兩個sequence來表示，先將所有矩形順時針轉45度並產生新的X'Y'座標，根據矩形X'座標與Y'座標的start point(S)與end point(E)可以獲得不同FF的先後關係資訊
    ![image](https://hackmd.io/_uploads/B175yEj1A.png)

3. Finding possible merged FFs & Maximal Clique Extraction
    
    在X' sequence中的S,E交界處插入decision point(D)，在這邊會把X'Y'座標中S與E相鄰的FF remove，因為代表此FF沒有與其他FF overlap。接下來每一個decision point會做一次判斷，看哪些FF可以cluster，它將S在D前的FF設為related FF，接著從Y'的sequence中找出由related FFs所組成的max clique(Partial Y')
    ![image](https://hackmd.io/_uploads/rkUrWEj10.png)
    
4. Candidate Choose
    
    找E在previous and current decision point中間 & S在next and current decision point 中間的    
    ->這兩種都是FF的feasible region與related FF較近的    
    同時會計算每個candidate FF最大的feasible region，即從connected path中可借到的最多salck，若此region與maximal clique的overlapping region未overlap則從candidate中移除
    ![image](https://hackmd.io/_uploads/rkWIUOhk0.png)
    
    這篇採用inclusion force來決定要選擇哪個candidate    
    (larger means less slack borrowing and smaller impact on original maximal clique)
    
    ![image](https://hackmd.io/_uploads/S12Ww_3yC.png)
    
    :::success 
    Fatt (Attractive factor) : 考慮feasible region與maximal clique的overlap region的xy距離遠近    
    ![image](https://hackmd.io/_uploads/SklE_Oh1A.png)
    :::
    :::success 
    Frep (Repulsive factor) : 受到目前的maximal clique與candidate FF本身所屬的maximal clique size差異影響
    
    ![image](https://hackmd.io/_uploads/SkW9__nkR.png)
    
    SMFFj代表candidate FFj所屬的maximal clique的size
    
    ![image](https://hackmd.io/_uploads/HkX_Kd2J0.png)
    代表不小於SMFFj的FF perfect size
    
    ![image](https://hackmd.io/_uploads/B15jFd2y0.png)
    代表比SMFFj小的FF perfect size
    
    第一、二條式子代表SMFFj比perfect size更大、小的情況    
    從第三條式子可以看出若SMFFj剛好為perfect size則Frep=0
    :::
    計算完inclusion force後根據大小排序，從最高force的candidate加入maximal clique並更新overlapping region，直到clique size達到perfect size或沒有candidate後停止。
    
    *決定好哪些FF要cluster後要將其移除X' sequence
5. MBFF Generation and Placement (這篇沒有提到如何決定MBFF位置) 
6. Slack Release

    在當前iteration clustering的FF，其diamond region可以壓縮到僅包圍MBFF的矩形，並將多餘的slack分給其他connected and unclustered的FFs
    
7. Iteratively Back to Step2
    
    repeat直到所有decision points and FFs visited

* 資料結構

    因為需要一直做slack redistribution，X'中FF的順序會一直改變，因此儲存X'Y' sequence用red-black tree可以快速delete and insert

:::info
check for other data structure that has better time complexity for insertion and deletion
:::

* 實驗結果比較
    
    與mean shift algorithm相比在timing的部分稍顯退步，但在power的結果較好，rumtime部分也比採用平行運算的mean shift algorithm快一點點

### 1.Graceful Register Clustering by Effective Mean Shift Algorithm for Power and Timing Balancing.

[from iccad 2015 problem c](https://iccad-contest.org/2015/problem_C/default.html)

[Paper](https://waynelin567.github.io/files/meanshift.pdf)

[Github](https://github.com/waynelin567/Register_Clustering)

*    Code execution:
```bash=
#Install boost first
$ wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz
$ tar xvf boost_1_84_0.tar.gz
$ ./bootstrap.sh --prefix=/usr/local/
$ ./b2
$ setenv BOOSTDIR "/home/vdalab/coherent17/boost_1_84_0"
$ setenv LD_LIBRARY_PATH "$BOOSTDIR/stage/lib"
    
$ git clone https://github.com/waynelin567/Register_Clustering.git
# Change CFLAGS in makefile to "CFLAGS = -O3 $(DEPENDDIR) -fopenmp -std=c++17 -lboost_program_options"
$ make -j
$ ./bin/clustering <input> <output>
```

演算法大綱:
透過改良傳統的mean shift algorithm並且整合KNN的概念，來進行cluster。



*    傳統的mean shift algorithm
:::success
1. 建立Density Surface
2. Gradient Ascent來移動資料點至最近的peak
3. 系統收斂後，在同一點的即為相同的cluster
:::


想法:  
假設FF的初始位置為機率分布函數(PDF)，想辦法找到這個機率分布函數$f(x,y)$。
用類似Fourier Expansion的方式，在每個資料點上放一個Kernal Function，而後將$n$個Kernal Function的總和視為Kernal Density Estimator(KDE)。

原本的Mean shift algorithm:
$$ 
\begin{gather*}
    f(x)=\frac{1}{nh^d} \sum_{i=1}^{n} K(\frac{x-x_i}{h})
\end{gather*}
$$

where
$$
\left\{
    \begin{matrix}
        n &=& \# \ data \ point(FF)\\
        h &=& \ bandwidth \\
        d &=& Dim \\
    \end{matrix}
\right.
$$

其中$K$是Kernal function，用以擬和Density Surface，會需要是一個對秤原點的方程式，在論文中使用Gaussian Function:

$$ 
\begin{gather*}
    K(u)=\frac{1}{\sqrt(2 \cdot \pi)} \cdot e^\frac{-1}{2}u^2
\end{gather*}
$$


:::success
可以思考是否要維持使用Gaussian Function而不使用其他Kernal Function。  
因為Gaussian Function計算相對複雜，如果使用三角波的superposition會不會比較簡單?  
只是要確保該Kernal Function積分起來要是1就好。  
或是可以使用某個正比於FF的參數來控制積分起來要是多少。  
![image](https://hackmd.io/_uploads/S1QaINOkC.png)
:::

前面所提到的$h$控制的類似Gaussian中的 $\sigma$，控制視野，因此若是h越大，Gaussian function底下的寬越大。


![image](https://hackmd.io/_uploads/SJBCP28kR.png)
當h很小時，所有FF會自成一類，但是當h很大時，視野會很大，因此很有可能會將所有FF都歸在同一類:

![image](https://hackmd.io/_uploads/Sya2SnLy0.png)

:::success
因此如何決定$h$也是很重要的事情。
:::

*    Kernal Density Estimator Visualization

會是這樣的概念，基本上就是很多Kernal Function的Superposition
![image](https://hackmd.io/_uploads/ryvVo2IyA.png)

建完density surface之後，對FF做gradient ascent，往density surface中的peak移動。

考慮到整體的座標，因此可以看出中心不會在該cluster中間(可以用KNN來限縮影響範圍)
![mean_shift](https://hackmd.io/_uploads/rJQylAIyA.gif)
https://colab.research.google.com/drive/1ZN9Ku9mcUwqLLArbum8pQgHzgiC1czLZ?usp=sharing



這篇論文提出說h不該是定值，應該要將h設為變數，用timing constraint來定義h要是多少。而後，建立density surface時，不需要考慮所有的FF的位置，只需要考慮前M近的FF就好，以免FF的displacement過大，因此他先使用KNN來找到前K近(Manhaton Distance)的，再去建density surface再做gradient ascent來做FF cluster。


$$ 
\begin{gather*}
    h_i(x_i)=min(h_{max}, \alpha ||x_i-x_{i,M} ||)
\end{gather*}
$$

透過KNN找到第M近的neighbor與自己的距離，而後如果是非常timing critical 的FF那就可以將$\alpha$設為接近0，因此$h_i$便會很小，因此會比較容易自成一類，讓displacement也不會有太大的影響。  

:::info
因此$h_i$便是$x$的變數了，因此在計算gradient的時候便需要考慮該項。
:::

:::danger
除了displacement delay，能否把$Delay_{D2Q}$也納入考量?
:::

所以綜合以上$h_i$的調整及整合$KNN$，新的density function為:

$$ 
\begin{gather*}
    f(x)=\frac{1}{n} \sum_{i \in KNN(x) } \frac{1}{h_i^d} K(\frac{x-x_i}{h_i})
\end{gather*}
$$


:::info
要思考KNN是否只用距離來當作參數，AREA POWER能否一概考慮?  
需要有一個機制來讓超過MTFF library制定規定bit的FF移動到其他的cluster。
:::

*    平行化的可行性:
因為每個FF都是個別去相對於原本的座標點來做計算及移動，因此只要一開始先有一份原始座標點的副本後，每個FF都是可以被平行計算的。
:::warning
但我認為這邊有個問題，因為當$FF_A$在移動後$FF_B$仍是看到它原本的座標，這樣會不會不太好?是否有更好的方式讓$FF_B$可以考慮到的是移動後$FF_A$的座標?
:::


### 2.Flip-flop clustering by weighted K-means algorithm

[Paper] https://home.engineering.iastate.edu/~cnchu/pubs/c88.pdf

### 3.Generation of Mixed-Driving Multi-Bit Flip-Flops for Power Optimization
[Paper](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=10069663)
    
考慮MBFF留有空的pin ex:3個
1-bit FF 合成4bit，一個位置留空

### 4.INTEGRA: Fast Multibit Flip-Flop Clustering forClock Power Saving
[Paper](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=6132648)

### 5.Post-Placement Power Optimization with Multi-Bit Flip-Flops
[Paper](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5654155)

:::info
基本上這篇演算法流程可以分為:  
*    用progressive window sliding找到可以被merge在一起的多個FF
*    建立TSFG(timing slack free group)，透過B&B找到哪些bit合起來比較好的候選人
*    接著從這些候選人中找到independent set來確認哪些FF要合在一起
*    最後透過考慮placement density及HPWL找到該MBFF要放在哪個bin上
:::

*    a.Progressive window sliding & expansion
![image](https://hackmd.io/_uploads/Bk87Jqyl0.png)

(a)是2x2 bins來找到局部最佳解，比起整個電路flatten下去解，透過只考慮框框內做最佳化會比較好，為了找到更好的解，會再把windows size調大(b)。而後續再做FF cluster & placement時只會考慮到該框框內的所有FF。

*    b.建立TSFR & TSFG
        *    TSFR(timing slack free region)
        
        ![image](https://hackmd.io/_uploads/rJfwxcylR.png)
        *    TSFG(timing slack free group)
        
        ![image](https://hackmd.io/_uploads/HyORx5yxC.png)
        
        f1 & f2因為TSFR有重疊，因此可以構成一個TSFG，也就是如果f1跟f2 bank在一起，放在這個重疊區域不會有timing violation的問題。便可以將f1 & f2視為一個TSFG

        ![image](https://hackmd.io/_uploads/B1LwZqyeC.png)

將每個FF視為一個node，如果兩個FF間的TSFR有重疊，那就將他們之間加一個edge。當這個intersetion graph建好之後，如果要找m-bit的FF cluster，便可以將問題變為找到所有的m-clique的問題，可以透過branch & bound來解。

:::info
補充: What is clique?  
就是一個graph的subgraph，且對於subgraph中的所有vertex，任兩點都會有一條邊。
:::

因此上圖可以找到兩個4-clique:
*    $G^4=(g^4_1,g^4_2)= {(n1,n2,n3,n4),(n1,n3,n4,n6)}$

那究竟要將哪個合為4-bit FF呢?找到TSFG的independent set(IS)。

*    IS(independent set)
接著，為了將前一步找到的TSFG進一步分類為哪些FF要合在一起，因為有些FF會重複出現在不同的clique中，因此這邊需要決定該FF要跟誰合在一起，因為該FF不會影分身之術，可以一次存在兩個cluster中。這邊他提出的演算法是考慮了Area及HPWL來greedy的做決定，而非DP來解optimal solution。

![image](https://hackmd.io/_uploads/SJvrcqkl0.png)

因此他便會先去排序剛剛$G^m$中所有組合的面積減HPWL後，而後$F^\prime$為記錄已拜訪過的FF，然後$G^m_{IS}$則為那些不重複的$g^m_i$

舉例:
假設4-clique問題:
$G^4=(g^4_1,g^4_2,g^4_3)= {(n1,n2,n3,n4),(n1,n3,n4,n6),(n6,n7,n8,n9)}$

$i=1$:$F^\prime$={n1,n2,n3,n4},$G^4_{IS}=(g^4_1)$  
$i=2$:$g^4_2$中有元素與$F^\prime$重複，skip  
$i=3$:$F^\prime$={n1,n2,n3,n4,n6,n7,n8,n9},$G^4_{IS}=(g^4_1,g^4_3)$

因此共可以合出兩個4-bit FF

*    MBFF placement
這邊在placement的時候有兩個考量:
        *    1. Placement Density  
            基本上就是將MBFF擺在前面圈起來那些FF的intersetion中的bin，挑bin density小的擺
        *    2. Interconnecting Wirelength  
             找到這些FF原始座標的中位數，找到與前面intersecton重疊的部分，如果找不到就往附近的座標外擴

![image](https://hackmd.io/_uploads/BkW4yiylA.png)


### 6.FF-Bond: Multi-bit Flip-flop Bonding at Placement
[Paper](https://www.ispd.cc/slides/2013/8_tsai.pdf)

:::success
用化學鍵結來解??  
這群人太有創意了，用來解Reference Paper 0所提到的斥力問題，用於決定要cluster pre-defined MBFF library成幾個bit......
:::

:::info
能否整合原子半徑的問題，與前面mean shift所提到的視野bandwidth $h$來做呢?

eg:  
MBFF Library = 元素週期表  
FF = 價電子  
動態的$h$ = 原子半徑  
正比於當下cluster了多少FF，用於決定現在的視野要多大?

![image](https://hackmd.io/_uploads/rJ4wSEayR.png)


能否結合熱力學第二定律:系統會朝熵增(最大亂度，最低能量)的方向前進? 非退火而是analytical的方式去找到熵增的方向?
:::

![image](https://hackmd.io/_uploads/S1ts1VpJA.png)


## Taskflow Simple Usage & Install
By Claire
```bash=
$ git clone https://github.com/taskflow/taskflow.git
```

Simply include header :
```c=
#include <taskflow/taskflow.hpp>
```
版本要c++17

## Force Directed Placement

*    Phase1: relative position(overlap)
*    Phase2: slot assignment

拉力只能在該partition中，不能拉超過partition? 不然displacement會太大
我越想越覺得Force directed好像沒啥用==
wire-length driven placement有啥可以轉譯成cluster? Force-directed 用於timing優化 TNS的部分?


[Paper] https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1084652

[Github1]
https://github.com/kabazoka/Wirelength-Driven-Detailed-Macro-Placement-with-Force-Directed-Method/tree/main

[Github2]
https://github.com/madhav427/Force-Directed-Placement-and-Maze-Routing-Tool/tree/master


## Other cluster algorithm


### K-means
Hierarchical用KNN分群，在該群中bank/debank?

feature吃x,y coordinate?
用cost function決定是否bank?

pros:displacement小

cons:易受初始狀態及outlier影響、cluster數量要預先設定


### SVM

### Halfspace
可用Linear Programming解

## hmetis manual & binary
[Github](https://github.com/coherent17/2023-ICCAD-Problem-B/blob/main/lib/hmetis/manual.pdf)

## 留言區


## TODO

1. Partition into 8 regions
2. Different cluster merge優化方法
3. 單一cluster內banking/debanking
4. Taskflow usage
5. 最近jemalloc好夯，去研究遺下這是啥鬼 (Done smth @ compile time)
6. 研究若不是在直角坐標，若是在極座標，漣漪演算法的可行性?
7. MTFF Library technology mapping?
![image](https://hackmd.io/_uploads/HkdE-IuJA.png)
8. Legalization Algorithm help? Min Sum of the displacement??
9. check for boost geometry
10. cpp linter [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html)

    Please also help me confirm whether this .clang-format work for the linter.

    .clang-format:
    ```bash=
    BasedOnStyle: Chromium
    Language: Cpp
    MaxEmptyLinesToKeep: 3
    IndentCaseLabels: false
    AllowShortIfStatementsOnASingleLine: false
    AllowShortCaseLabelsOnASingleLine: false
    AllowShortLoopsOnASingleLine: false
    DerivePointerAlignment: false
    PointerAlignment: Right
    SpaceAfterCStyleCast: true
    TabWidth: 4
    UseTab: Never
    IndentWidth: 4
    BreakBeforeBraces: Linux
    AccessModifierOffset: -4
    ```

12. static analysis [cppcheck](https://cppcheck.sourceforge.io/)
13. ci to run the testcase & verifier after commit [CI](https://medium.com/%E6%8A%80%E8%A1%93%E7%AD%86%E8%A8%98/%E4%BD%95%E8%AC%82-ci-cd-%E5%88%A9%E7%94%A8-github-actions-%E5%81%9A%E4%B8%80%E5%80%8B%E7%B0%A1%E5%96%AE%E7%9A%84-ci-cd-2d55e6dabeed)

13. Simple git usage
*    [Download](https://git-scm.com/downloads)
```bash=
# replace with your own info & type in cmd (first use only)
$ git config --global user.name "Coherent17"
$ git config --global user.email mnb51817@gmail.com
```

*    fork the repo (can use this repo to practice: https://github.com/coherent17/test_repo)
*    git clone from your forked repo
*    write code, commit & push
*    pull request and wait for merge
*    pull after admin merge
*    [reference video](https://youtu.be/x24fOAPclL4?si=tInV83pWkKSUbM1Q)

:::danger
禁止直接commit到mater branch
:::

14. Topological sort based legalization from秀儒?

15. Meeting per 2 weeks?

16. check post placement vs logic synthesis兩個階段對於MBFF cluster的作法有何不同之處?

17. How to deal with the pre-place MBFF to cluster with other 1-bit FF?