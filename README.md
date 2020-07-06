This repository and [COBRA](https://github.com/acco93/cobra) contain source code and support material associated to the paper *Fast and Scalable Heuristic Solution of Large-Scale Capacitated Vehicle Routing Problem*.

***
**However, the source code will be made available once the paper is accepted.**
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
The are a number of build options that are off by default.
In order to enable an option `XYZ`, add `-DXYZ=ON` to the `cmake` command before calling `make`

Available options
* `ENABLE_VERBOSE` output some information during the resolution
* `ENABLE_GUI` creates a GLFW window showing a graphical representation of the best found solution along with some information regarding move generators and recently accessed vertices, and another GLFW window showing the algorithm search trajectory. Some additional packages, e.g. `libglfw3-dev`, may be necessary to compile the code with this option.

#### Running the code
Once `filo` has been built, three arguments are required to run the executable
1. a path to a compatible instance file
2. an integer seed
3. a path to a parameters file, some examples can be found in the [parameters directory](https://github.com/acco93/filo/parameters) in the root of the project

As an example, if the executable is in the `build` directory and both `cobra` and `filo` projects are into a `git` directory in the `user` home
```
cd /home/user/git/filo/build

./filo /home/user/git/cobra/instances/X/X-n936-k151.vrp 0 ../parameters/x
```

The parameter file, in addition to parameters explained in the paper contains 2 additional entries
* `OutputPath` defining where to output result files. Default is the current directory. Note that a writable directory must be selected.
* `ParserType` defining the parser to be used to process the instance. Possible values are `X`, `K`, and `Z`. The `X` parser works with **X** and **B** datasets. The `K` parser with the **K** dataset and the `Z` one with the **Z** dataset. For adding additional parsers check [AbstractInstanceParser.hpp](https://github.com/acco93/cobra/blob/master/include/cobra/AbstractInstanceParser.hpp).

For the datasets described in the paper, please use the default parameters file found [here](https://github.com/acco93/filo/parameters).

#### Screenshots
Console output (`ENABLE_VERBOSE=ON`). ROUTEMIN: % Inf = n. infeasible solution over number of performed iterations. COREOPT: Gamma = average sparsification factor, Omega = average shaking intensity.
![Console output](images/console.png)

Solution representation (`ENABLE_GUI=ON`). Routes are shown in green-to-red shades, the redder the loaded.
Move generators are shown in cyan. The current area in which filo is working is identified by the yellow squares.
![Solution representation](images/solution.png)

Search trajectory (`ENABLE_GUI=ON`). The best solution value is represented by the blue line while the current solution value by the yellow line. Red and green dots represent the value of the shaken solution and the re-optimized local optima respectively. Each horizontal dashed line identifies a 1% improvement of the gap with respect to the initial solution value.
![Search trajectory](images/trajectory.png)
