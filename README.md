# elfin-solver [![Build Status](https://travis-ci.com/joy13975/elfin-solver.svg?branch=master)](https://travis-ci.com/joy13975/elfin-solver)

The `master` branch is under development but has main features working. The old v1 version and documentation is preserved in branch `v1`.

### Compiler

Any C++17 compliant compiler should work, but I have only tested using gcc-5 and gcc-8. 
Important: If you are a Mac user, be sure to swap out Clang for gcc because you will get libiomp complaints.

Get gcc [somehow](https://gcc.gnu.org/). For Macs it's simplest to install through homebrew with `brew install gcc --without-multilib`.

### To Download
```Bash

git clone --depth 1 https://github.com/Parmeggiani-Lab/elfin-solver.git
cd elfin-solver

```

### To Complete Setup (or to update)
```Bash

./update

```

### To Compile
```Bash
make -j4  # Or use the number of your processors
```

### To Run

run the help function to get an overview of all possibilities
```Bash
./bin/elfin -h
```

run standart settings using JSON from elfin-ui output:
```Bash
./bin/elfin --spec_file <path to JSON.json>
```

