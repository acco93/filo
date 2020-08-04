[This repository](https://github.com/acco93/filo) and [COBRA](https://github.com/acco93/cobra) contain source code and support material associated with the paper *A Fast and Scalable Heuristic for the Solution of Large-Scale Capacitated Vehicle Routing Problems* (draft available [here](http://or.dei.unibo.it/technical-reports/2020)).

***

**However, the source code of both projects will be made available once the paper is accepted.**

***

#### Building the code
1. Build and install the [COBRA](https://github.com/acco93/cobra) library
2. Clone the repository and build the algorithm
```
  git clone https://github.com/acco93/filo.git
  cd filo
  mkdir build && cd build
  cmake .. -DCMAKE_BUILD_TYPE=Release
  make -j
```

#### Build options
The are a number of build options that are disabled by default.
In order to enable an option `XYZ`, add `-DXYZ=ON` to the `cmake` command before calling `make`

Available options
* `ENABLE_VERBOSE` output some information during the resolution
* `ENABLE_GUI` creates a GLFW window showing a graphical representation of the best found solution along with some information regarding move generators and recently accessed vertices, and another GLFW window showing the algorithm search trajectory. Some additional packages, e.g. `libglfw3-dev`, may be necessary to compile the code when this option is enabled.

#### Running the code
Once `filo` has been built, you can run it by giving as first mandatory argument the path to a compatible instance file.

As an example, if the executable is in the `build` directory and both `cobra` and `filo` projects are into a `git` directory in the `user` home
```
cd /home/user/git/filo/build

./filo /home/user/git/cobra/instances/X/X-n936-k151.vrp
```

An help menu explaining available optional command line arguments can be read by executing `filo --help`.

More examples on how to run the code can be found in the [`scripts`](https://github.com/acco93/filo/tree/master/scripts) directory.

#### How can I exactly reproduce the results shown in the [`results`](https://github.com/acco93/filo/tree/master/results) directory?

###### 1. Using docker (preferred way)
1. Download [a copy of the environment we used](#) to compile the code
2. `cat env.tar.gz | docker import - filo:20200804`
3. `docker run -t -i filo:20200804 /bin/bash`
4. The project together with an already compiled executable can be found in `/data/git/filo`

###### 2. Building the code with specific `c++` compiler and standard library version
To ensure a complete reproducibility the code should be compiled with `g++ 8` which uses the c++ standard library version `libstdc++.so.6.0.25`.

In fact, as a result of an unfortunate design choice we iterate over some `std::unordered_set` data structures (check the COBRA project in `/include/cobra/LocalSearch.hpp`) with the aim of inserting Static Move Descriptors (SMDs) into a binary heap data structure. The heap vector is then linearly scanned searching for a feasible SMD. However given that a binary heap is not unique for a given set of entries, the order in which SMDs are inserted might cause the linear scan of the heap to reach an SMD rather than another. The `c++` standard does not impose a given order for iterating over unordered data structures thus a slightly different `c++` version might result in a different iteration order causing the search trajectory to be be slightly different.

Note however that average results over a reasonable number of runs, say 10, would still remain comparable even by using different compilers/versions.

#### Screenshots
Console output (`ENABLE_VERBOSE=ON`). ROUTEMIN: % Inf = n. infeasible solution over number of performed iterations. COREOPT: Gamma = average sparsification factor, Omega = average shaking intensity.
![Console output](images/console.png)

Solution representation (`ENABLE_GUI=ON`). Routes are shown in green-to-red shades, the redder the loaded.
Move generators are shown in cyan. The current area in which filo is working is identified by the yellow squares.
![Solution representation](images/solution.png)

Search trajectory (`ENABLE_GUI=ON`). The best solution value is represented by the blue line while the current solution value by the yellow line. Red and green dots represent the value of the shaken solution and the re-optimized local optima respectively. Each horizontal dashed line identifies a 1% improvement of the gap with respect to the initial solution value.
![Search trajectory](images/trajectory.png)
