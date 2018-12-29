
CXX=g++

EXE=elfin

OBJ_DIR 		:= .obj
SRC_TREE 		:= $(shell find . -type d | grep -v './.obj\|\.dSYM\|^.$$')
OBJ_TREE 		:= $(addprefix $(OBJ_DIR)/, $(SRC_TREE))
BIN_DIR 		:= ./bin/
$(shell mkdir -p $(OBJ_TREE) $(BIN_DIR))

C_SRC 			:= $(shell find src lib/jutil/src/jutil.c -name '*.c')
CC_SRC 			:= $(shell find src -name '*.cc')

OBJS 			:= $(C_SRC:%.c=$(OBJ_DIR)/%.o) $(CC_SRC:%.cc=$(OBJ_DIR)/%.o)
DEPS 			:= $(C_SRC:%.c=$(OBJ_DIR)/%.d) $(CC_SRC:%.cc=$(OBJ_DIR)/%.d)

# $(info Sources to be compiled: [${C_SRC}] [${CC_SRC}])
# $(info Objects to be compiled: [${OBJS}])

# Flag switches
DEBUG=basic
ifeq ($(DEBUG),basic)
	DEBUG_FLAGS := -ggdb3 -rdynamic
else ifeq ($(DEBUG),asan)
	DEBUG_FLAGS := -fsanitize=address -fno-omit-frame-pointer
else
	DEBUG_FLAGS := -DNDBUG
endif

EIGEN=yes
ifeq ($(EIGEN),yes)
	EIGEN_FLAGS = -DUSE_EIGEN
endif

TARGET=cpu
ifeq ($(TARGET),gpu)
$(info Using clang++ for GPU target)
$(info This was only tested on the Zoo cluster)
	CXX = clang++
	OMP_FLAGS = -fopenmp=libomp -fopenmp-targets=nvptx64-nvidia-cuda \
		--cuda-path=/nfs/modules/cuda/8.0.61/ -DTARGET_GPU
endif

ifeq ($(CXX),clang++)
	ifeq ($(TARGET),cpu)
		OMP_FLAGS += -openmp
	endif
else
	OMP_FLAGS     += -fopenmp
endif

INCLUDES += -I. -I./headers -I./lib/jutil/headers -I./lib 

ifeq ($(CXX),clang++)
	ifeq ($(OS),Windows_NT)
	else
		UNAME_S := $(shell uname -s)
		ifeq ($(UNAME_S),Darwin)
		       	# For clang, these include directories vary from system to system
		        # Find the ones your gcc/g++ use
	        	INCLUDES += -I/usr/local/Cellar/gcc/6.1.0/include/c++/6.1.0 \
	                                   -I/usr/local/Cellar/gcc/6.1.0/include/c++/6.1.0/x86_64-apple-darwin15.5.0 \
	                                   -I/usr/local/Cellar/gcc/6.1.0/include/c++/6.1.0/backward \
	                                   -I/usr/local/Cellar/gcc/6.1.0/lib/gcc/6/gcc/x86_64-apple-darwin15.5.0/6.1.0/include \
	                                   -I/usr/local/include \
	                                   -I/usr/local/Cellar/gcc/6.1.0/include \
	                                   -I/usr/local/Cellar/gcc/6.1.0/lib/gcc/6/gcc/x86_64-apple-darwin15.5.0/6.1.0/include-fixed
		        # You also need libiomp (I got this from homebrew; your version, and hence include directory, may differ)
		        INCLUDES += -I/usr/local/Cellar/libiomp/20150701/include/libiomp
		        LD_FLAGS += -Wl,-rpath,-L/usr/local/Cellar/gcc/6.1.0/lib/gcc/6 -lgomp
		endif
	endif

	ERR_FLAGS 	:=
	CC_FLAGS 	+= -stdlib=libstdc++
else ifeq ($(CXX),g++)
	MAX_ERRORS  = 1
	ERR_FLAGS	:= -fdiagnostics-color=always -fmax-errors=$(MAX_ERRORS) -Werror
endif

CC_FLAGS 		+= -MMD -std=gnu++14
OPT_FLAGS       += -Ofast

COMPILE 		:= $(CXX) $(CC_FLAGS) $(ERR_FLAGS) \
	$(OPT_FLAGS) $(EIGEN_FLAGS) $(DEBUG_FLAGS) $(OMP_FLAGS) \
	$(DEFS) $(INCLUDES) $(EXTRA_FLAGS)

BINRAY=$(BIN_DIR)$(EXE)

EXTS=c cc
define make_rule
$(OBJ_DIR)/%.o: %.$1
	$(COMPILE) -o $$@ -c $$< $(EXTRA_FLAGS)
endef
$(foreach EXT,$(EXTS),$(eval $(call make_rule,$(EXT))))

#
#
# Rules
#
#

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
