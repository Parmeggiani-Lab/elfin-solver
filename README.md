# elfin-solver [![Build Status](https://travis-ci.com/joy13975/elfin-solver.svg?branch=master)](https://travis-ci.com/joy13975/elfin-solver)

The `master` branch is currently under development and is not currently functional. The old v1 version code and documentation can be found in branch `v1`.

Since [elfin-ui](https://github.com/joy13975/elfin-ui) are still undergoing active development, the internal data representation and the problem scope are still evolving. That forced me to put a brake on 
working on elfin-solver.

### To Download
```
git clone --depth 1 git@github.com:joy13975/elfin-solver.git
cd elfin-solver
git submodule update --init --recursive
```

### To Update
```
git pull
git submodule update
./fetch_db
```

### Weekly Goals

- [ ] Restructure source tree into ```./src``` folder. 

- [ ] Create handlers stubs for each work type.

- [ ] Started working on the main core solver.
​