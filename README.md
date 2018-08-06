# Elfin Solver V1
[![Build Status](https://travis-ci.com/joy13975/elfin-solver.svg?branch=master)](https://travis-ci.com/joy13975/elfin-solver)

This is an old version of the elfin-solver and will not be applicable to v2 file formats.

### Before Compiling
```
git clone --depth 1 git@github.com:joy13975/elfin-solver.git
cd elfin-solver
git submodule init
```

### To Update
```
git pull
git submodule update
./fetch_db
```

### To compile:
```make```

To compile with your choice of compiler e.g. clang++:

```make CXX='clang++'```

### To run:
```./bin/elfin [arguments to override config.json]```


To get help:
```./bin/elfin -h```

### Notes:
1. The default configuration file is ```config.json```.
2. This repository is meant to be a submodule of the main [elfin] suit of tools, hence the strange path settings in ```config.json```.
3. Makefile adds ```jutil/src/``` to the include path, that's why ```util.h``` gets included as if it was in 
the same directory everywhere in the source.
4. If ```jutil/src``` is not installed, you need to do.

# Old (v1) Documentation
## Core Solver Setup
First you need GCC/G++ 5 and above. On Macs for instance, you can get OpenMP-enabled GCC-6 by simply installing from Homebrew:

```
brew install gcc --without-multilib
```

The installation could take a while. After this is done, you should get commands ```gcc-6``` and ```g++-6```. You can choose to make these your default compilers by overwriting the symbolic links at ```/usr/local/bin/gcc``` and ```/usr/local/bin/g++```:
```
ln -Fs `which gcc-6` /usr/local/bin/gcc
ln -Fs `which g++-6` /usr/local/bin/g++
```

For troubleshooting on MacOS, please refer to [this Stackoverflow post](https://stackoverflow.com/questions/35134681/installing-openmp-on-mac-os-x-10-11). 

If your compiler complains about ```omp_get_initial_device``` not declared, that's because your OpenMP version is too old. Check with ``` echo |cpp -fopenmp -dM |grep -i open```; only versions above 201511 seem to define this function. This is not vital to the application so if you really do not want to install a newer compiler then you may comment out that erring line.

Then install the jutil submodule and get ready to compile.

```
git submodule init
git submodule update --init --force --remote            
cd celfin                                                   
make
```

Notes:
 - Specify the compiler of your choice by e.g. for clang++: ```make CXX=clang++```.
 - For clang, you will need to specify the C++ standard library include path, which depends on where you installed GCC. See ./celfin/Makefile for details (the INCS variable). You will also need to include libiomp and library load paths (again see Makefile).
 - To speed up the compilation, add the ```-j``` flag with the number of cores on your system.
 - To build without OpenMP, you can specify ```make OMP=no```

## Usage
Once you have compiled Elfin Core successfully, you can test run it with:
```
# cd to celfin
./bin/elfin                                             
```

The default configuration is at ```./celfin/config.json``` and uses the ```./bm/l10/1696.json``` example input. This demonstrates that the Core GA finds the zero score solution i.e. the original module constituents of the input. Execution outputs will be stored in ```./celfin/output/``` in JSON format (files named after their memory addresses). You can specify the output directory using the ```-o``` flag. Even if you force Elfin to stop using ctrl-c, the most recent best candidates will be saved.

To test a different example, you may try:

```
./bin/elfin -i ../bm/l20/mqj2.json
```

All examples in ```./bm/l10```, ```./bm/l20``` and ```./bm/l30``` should reach score zero before the maximum number of iterations (default=1000) is exhausted. However, because the random number generator might behave differently on your machine, in some rare cases Elfin might not reach score zero. If this happens, try using larger population sizes e.g. using the ```-gps``` flag.

To get help:
```
./bin/elfin -h
```

A typical Elfin run with specific population size and input file would be:
```
./bin/elfin -gps <POPULATION_SIZE> -i <INPUT_FILE>
```

Notes:
 - Output JSONs are named after their sorted memory addresses. The lower value the hex name has, the better solution it is (lower score).
 - Command-line arguments will override arguments specified in the configuration file.


## Design Verification
After running a shape design through Elfin Core, you should find output solutions in ```./celfin/output/```. Recall that smaller hexadecimal values in the file name corresponds to better solutions (they're just memory address of a sorted array). Here we shall use the default input as example.

```
for f in `ls output`; do echo $f; break; done #print best solution file name
# take for instance 0x10e63c000.json
# go back to repo root       
```
Inspect the output file to see what it contains (score and module/node names). The score is the Kabsch "simularity" score between the solution and the input shape. The lower this score is, the better the solution.

**PDB Reconstruction**

To be re-written.

**Visualisation**

At this point, you can already visualise the design outputs in PyMOL by loading the synthesised PDB. You can also plot the solution centre-of-mass points (stored in the CSV output mentioned above). In PyMOL: load the plotting script from ```./scripts/PyMolUtils/LineUtils.py```. Then, type the following in the command window:
```
draw_csv('path/to/your/csv/file')                       #to plot any CSV 3D point file
draw_json('path/to/your/json/file')                     #to plot JSON files that have a 'coms' 3D point array
```

More plotting options can be found by reading the script ```./scripts/PyMolUtils/LineUtils.py```. Note that the default plotting color is black, so you must do ```bg white``` to see them properly. Also note that input specification CSV files can be plotted using the same command.

**Calculating Global RMSD**

To compute the RMSD, we first need to relax the result PDB. This is the same as when we relaxed databse PDBs:

```
relax </path/to/PDB/for/relaxation>
```

Apply the same environment variable changes as needed (mentioned in section 6).
The relaxation of larger structures will take longer (much much longer than DB modules). Using our previous example (on ```./bm/l10/1696.json```) we should yield a file called ```./celfin/output/0x10e63c000_Synth_Main_s1.0_l10_0001.pdb``` and ```./celfin/output/0x10e63c000_Synth_Main_s1.0_l10_0001_relax.sc```. 

The first file is the relaxed PDB structure, and the second is the score file that contains information about how different the structures are before and after relaxation. To get a Rosetta RMSD estimate (global), open the score file and look at the second column. In this case, I got 2.424.

In the score file the first row is the column headers. Each subsequent row denotes values obtained from each subsequent Rosetta transformation of the structure in question. It is useful to divide the global RMSD by the number of modules in the design to get a more unbiased perspective of RMSD per module. This is because lever effect is obviously much more pronounced in longer/larger protein designs.

**Calculating Windowed RMSD**

Ensure that you have a relaxed result PDB first. To compute the windowed RMSD, invoke:
```
rmsd.py <solutionDir> <minimisedDir> <windowLen=300> <overlapRatio=0.50> <warnThreshold=5.0>
```

Before you can run this script, copy the relaxed (or minimised) PDBs you want to compare into a different folder - one which the script shall treat as ```<minimisedDir>```. For each relaxed/minimised PDB in this folder, the script will compute a windowed RMSD with respect to their original structure (should exist in ```<solutionDir>```) before the Rosetta transformation.

For instance if we run this on our example output:
```
mkdir ./tmp                                             #make a temporary folder in repo root
cp ./celfin/output/0x10e63c000_Synth_Main_s1.0_l10_0001.pdb ./tmp 
./scripts/Python/RMSDStat.py ./celfin/output ./tmp          #invoke the windowed RMSD script
```

The output indicated:
```
avg: 1.37087369713 min 0.979476939189 max 1.65581158797
```

Which is consistent with the result in my thesis (for benchmark ```./bm/l10/1696.json```).