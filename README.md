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
    - There is going to be quite a bit of redesign/rewriting of logic due to new reference frames calculation method.
    - Many parts of the v1 code can use improvement in both style and design.
    - Some old code likely won't make it to v2.

- [ ] Parse JSON output file exported from elfin-ui.

- [ ] Create handlers stubs for each work type.

- [ ] Started working on the main core solver - if there's time.
​