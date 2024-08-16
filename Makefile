# Compiler & Linker settings
CXX = g++
CXXFLAGS = -I ./inc -std=c++14 -fopenmp
OPTFLAGS = -march=native -funroll-loops -finline-functions -ffast-math -O3
DEBUGFLAGS = -DENABLE_DEBUG_DP -DENABLE_DEBUG_LGZ -DENABLE_DEBUG_CHECKER -DENABLE_DEBUG_TIMER -DENABLE_DEBUG_MS -DENABLE_DEBUG_BAN -DENABLE_DEBUG_MGR #-DNDEBUG(for assert) 
RELEASEFLAGS = -DNDEBUG
WARNINGS = -g -Wall -static

# Valgrind for memory issue
CHECKCC = valgrind
CHECKFLAGS = --leak-check=full -s --show-leak-kinds=all --track-origins=yes

# Cppcheck for static analysis
CPPCHECKCC = cppcheck
CPPCHECKFLAGS = --enable=all --inconclusive --std=c++11 --language=c++ --suppress=missingIncludeSystem --suppress=style --suppress=unusedFunction --suppress=unreadVariable --suppress=unmatchedSuppression --suppress=unusedStructMember --suppress=functionConst

# Source files and object files
SRCDIR = src
OBJDIR = obj
INCDIR = inc
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.d,$(SRCS))

# Control the build verbosity
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

.PHONY: all check clean calc

# Name of the executable
BIN = cadb0015

all: $(BIN)

$(OBJDIR):
	@mkdir $(OBJDIR)

$(BIN): main.cpp $(OBJS)
	$(VECHO) "	LD\t$@\n"
	$(Q)$(CXX) $(WARNINGS) $(DEBUGFLAGS) $(CXXFLAGS) $(OPTFLAGS) $^ -o $@ $(LINKER)

# Include generated dependency files
-include $(DEPS)

# Compilation rule for object files with automatic dependency generation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) Makefile
	$(VECHO) "	CC\t$@\n"
	$(Q)$(CXX) $(WARNINGS) $(DEBUGFLAGS) $(CXXFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

release: 
	rm -rf $(OBJDIR)
	$(MAKE) DEBUGFLAGS="$(RELEASEFLAGS)" BIN=cadb0015

run1:
	./$(BIN) testcase/sampleCase testcase/sampleCase.out
	chmod +x sanity_checker/sanity
	chmod +x sanity_checker/placement_checker
	./sanity_checker/sanity testcase/sampleCase testcase/sampleCase.out
	./sanity_checker/placement_checker testcase/sampleCase testcase/sampleCase.out

run2:
	./$(BIN) testcase/testcase1_0812.txt testcase/testcase1_0812.txt.out
	chmod +x sanity_checker/sanity
	chmod +x sanity_checker/placement_checker
	./sanity_checker/sanity testcase/testcase1_0812.txt testcase/testcase1_0812.txt.out
	./sanity_checker/placement_checker testcase/testcase1_0812.txt testcase/testcase1_0812.txt.out

run3:
	./$(BIN) testcase/testcase2_0812.txt testcase/testcase2_0812.txt.out
	chmod +x sanity_checker/sanity
	chmod +x sanity_checker/placement_checker
	./sanity_checker/sanity testcase/testcase2_0812.txt testcase/testcase2_0812.txt.out
	./sanity_checker/placement_checker testcase/testcase2_0812.txt testcase/testcase2_0812.txt.out

run4:
	./$(BIN) testcase/testcase1_MBFF.txt testcase/testcase1_MBFF.txt.out
	chmod +x sanity_checker/sanity
	chmod +x sanity_checker/placement_checker
	./sanity_checker/sanity testcase/testcase1_MBFF.txt testcase/testcase1_MBFF.txt.out
	./sanity_checker/placement_checker testcase/testcase1_MBFF.txt testcase/testcase1_MBFF.txt.out

run5:
	./$(BIN) testcase/testcase2_MBFF.txt testcase/testcase2_MBFF.txt.out
	chmod +x sanity_checker/sanity
	chmod +x sanity_checker/placement_checker
	./sanity_checker/sanity testcase/testcase2_MBFF.txt testcase/testcase2_MBFF.txt.out
	./sanity_checker/placement_checker testcase/testcase2_MBFF.txt testcase/testcase2_MBFF.txt.out

runMBFF:
	./$(BIN) testcase/sampleCaseMBFF testcase/sampleCaseMBFF.out
	chmod +x sanity_checker/sanity
	./sanity_checker/sanity testcase/sampleCaseMBFF testcase/sampleCaseMBFF.out

drawALL:
	./drawDie/drawDie -i Preprocessor.out -m 1_Preprocessor.png -t Preprocessor -g -p -nl -o
	./drawDie/drawDie -i Meanshift.out -m 2_Meanshift.png -t Meanshift -g -p -nl -o
	./drawDie/drawDie -i Banking.out -m 3_Banking.png -t Banking -g -p -nl -o
	./drawDie/drawDie -i PostCG.out -m 4_PostCG.png -t PostCG -g -p -nl -o
	./drawDie/drawDie -i Legalize.out -m 5_Legalize.png -t Legalize -g -p -nl -o

draw:
	./drawDie/drawDie -i Legalize.out -m 5_Legalize.png -t Legalize -g -p -nl -o

valgrind:
	$(CHECKCC) $(CHECKFLAGS) ./$(BIN) testcase/testcase1_0614.txt testcase/testcase1_0614.txt.out
	cat valgrind.log

cppcheck:
	$(CPPCHECKCC) $(CPPCHECKFLAGS) -I$(INCDIR) $(SRCDIR) main.cpp $(INCDIR)/*.h

calc:
	(find src inc -type f -name "*.cpp" -o -name "*.h" && echo main.cpp) | xargs wc -l | tail -n 1

boost:
	chmod +x scripts/GetBoost.sh
	./scripts/GetBoost.sh
	rm boost_1_84_0.tar.gz

gitlog:
	git log --graph --decorate --oneline

clean:
	rm -rf $(OBJDIR) $(BIN) testcase/*.out *.log *.out *.png


# TODO, make release to compile with static-linking
# ref: https://github.com/fbacchus/MaxHS/issues/3 valgrind error when using valgrind