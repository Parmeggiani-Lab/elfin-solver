
CXX=g++

EXE=elfin

OBJ_DIR 		:= .obj
SRC_TREE 		:= $(shell find . -type d | grep -v './.obj\|\.dSYM\|^.$$')
OBJ_TREE 		:= $(addprefix $(OBJ_DIR)/, $(SRC_TREE))
BIN_DIR 		:= ./bin/
$(shell mkdir -p $(OBJ_TREE) $(BIN_DIR))

C_SRC 			:= $(shell find src jutil -name '*.c')
CC_SRC 			:= $(shell find src jutil -name '*.cc')

OBJS 			:= $(C_SRC:%.c=$(OBJ_DIR)/%.o) $(CC_SRC:%.cc=$(OBJ_DIR)/%.o)
DEPS 			:= $(C_SRC:%.c=$(OBJ_DIR)/%.d) $(CC_SRC:%.cc=$(OBJ_DIR)/%.d)

# $(info Sources to be compiled: [${C_SRC}] [${CC_SRC}])
# $(info Objects to be compiled: [${OBJS}])

DEBUG=yes
OMP=yes
TARGET=cpu
TIMING=yes
MAX_ERRORS=1
ASAN=no

ifeq ($(DEBUG), yes)
	DEBUG_FLAGS+=-ggdb3 -rdynamic
else
	DEBUG_FLAGS+=-DNDBUG
endif

ifeq ($(ASAN), yes)
	DEBUG_FLAGS+=-fsanitize=address -fno-omit-frame-pointer
endif

ifeq ($(TARGET), gpu)
$(info Using clang++ for GPU target)
$(info This was only tested on the Zoo cluster)
	CXX=clang++
	OMP_FLAGS=-fopenmp=libomp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=/nfs/modules/cuda/8.0.61/ -DTARGET_GPU
endif

ifeq ($(CXX), clang++)
	# clang has no GLIBCXX_PARALLEL until c++17
	ifeq ($(TARGET), cpu)
		OMP_FLAGS=-openmp
	endif
else
	OMP_FLAGS=-fopenmp -D_GLIBCXX_PARALLEL
endif

ifeq ($(TIMING), yes)
	TIMING_FLAGS=-DDO_TIMING
else
	TIMING_FLAGS=
endif

INCS += -I./lib -I./headers -I./jutil/src/ -I.

ifeq ($(CXX), clang++)
	ifeq ($(OS),Windows_NT)
	else
		UNAME_S := $(shell uname -s)
		ifeq ($(UNAME_S), Darwin)
		       	# For clang, these include directories vary from system to system
		        # Find the ones your gcc/g++ use
	        	INCS            += -I/usr/local/Cellar/gcc/6.1.0/include/c++/6.1.0 \
	                                   -I/usr/local/Cellar/gcc/6.1.0/include/c++/6.1.0/x86_64-apple-darwin15.5.0 \
	                                   -I/usr/local/Cellar/gcc/6.1.0/include/c++/6.1.0/backward \
	                                   -I/usr/local/Cellar/gcc/6.1.0/lib/gcc/6/gcc/x86_64-apple-darwin15.5.0/6.1.0/include \
	                                   -I/usr/local/include \
	                                   -I/usr/local/Cellar/gcc/6.1.0/include \
	                                   -I/usr/local/Cellar/gcc/6.1.0/lib/gcc/6/gcc/x86_64-apple-darwin15.5.0/6.1.0/include-fixed
		        # You also need libiomp (I got this from homebrew; your version, and hence include directory, may differ)
		        INCS            += -I/usr/local/Cellar/libiomp/20150701/include/libiomp
		        LD_FLAGS        += -Wl,-rpath,-L/usr/local/Cellar/gcc/6.1.0/lib/gcc/6 -lgomp
		endif
	endif

	ERR_FLAGS 	:=
	CC_FLAGS 	+= -stdlib=libstdc++
else ifeq ($(CXX), g++)
	ERR_FLAGS	:= -fdiagnostics-color=always -fmax-errors=1
endif

OPT_FLAGS 		+= -Ofast
CC_FLAGS 		+= -MMD -std=gnu++17 # c++17 for more copy elision
					

COMPILE 		:= $(CXX) $(CC_FLAGS) $(ERR_FLAGS) \
	$(OPT_FLAGS) $(DEBUG_FLAGS) $(OMP_FLAGS) $(TIMING_FLAGS) \
	$(DEFS) $(INCS) -fmax-errors=$(MAX_ERRORS) $(EXTRA_FLAGS)

BINRAY=$(BIN_DIR)$(EXE)

#
# start of rules
#

EXTS=c cc
define make_rule
$(OBJ_DIR)/%.o: %.$1
	$$(COMPILE) -o $$@ -c $$< $(EXTRA_FLAGS)
endef
$(foreach EXT,$(EXTS),$(eval $(call make_rule,$(EXT))))

$(BINRAY): $(OBJS)
	$(COMPILE) $(OBJS) -o $(BINRAY) $(LD_FLAGS)

test: $(BINRAY)
	$(BINRAY) -c config/test.json $(ELFIN_ARGS)

unit: $(BINRAY)
	$(BINRAY) -t $(ELFIN_ARGS)

dry: $(BINRAY)
	$(BINRAY) -c config/test.json -dry $(ELFIN_ARGS)

VALGRIND_FLAGS += --track-origins=yes --leak-check=full --show-leak-kinds=all --show-reachable=no 

valgrind: $(BINRAY)
	valgrind $(VALGRIND_FLAGS) $(BINRAY) -c config/test.json $(ELFIN_ARGS)

valgrind_dry: $(BINRAY)
	valgrind $(VALGRIND_FLAGS) $(BINRAY) -c config/test.json -dry $(ELFIN_ARGS)

FORCE:
.PHONY: all clean

all: $(BINRAY)

clean: FORCE
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/* *.dSYM .DS_Store *.dec *.bin

-include $(DEPS)
