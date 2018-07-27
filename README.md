# Elfin Solver
[![Build Status](https://travis-ci.com/joy13975/elfin-solver.svg?branch=master)](https://travis-ci.com/joy13975/elfin-solver)

¡ This solver is currently outdated !

Its new requirements are not yet set because [elfin-ui](https://github.com/joy13975/elfin-ui) and [elfin-data](https://github.com/joy13975/elfin-data) are still undergoing significant changes, which affect not only the internal data representation but also the problem scope that this solver is supposed to solve.

Once those two stages are more or less complete, development will resume for the solver.

### Before compiling
```git submodule init; git submodule update```

### To compile:
```make```

### To compile with your choice of compiler e.g. clang++:
```make CXX='clang++'```

### To get help:
```./bin/elfin -h```

### To run:
```./bin/elfin [arguments to override config.json]```

### Notes:
1. The default configuration file is ```config.json```.
2. This repository is meant to be a submodule of the main [elfin] suit of tools, hence the strange path settings in ```config.json```.
3. Makefile adds ```jutil/src/``` to the include path, that's why ```util.h``` gets included as if it was in 
the same directory everywhere in the source.
4. If ```jutil/src``` is not installed, you need to do.