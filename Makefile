# Compiler & Linker settings
CXX = g++
CXXFLAGS = -I ./inc -std=c++14 -fopenmp
OPTFLAGS = -march=native -funroll-loops -finline-functions -ffast-math -O3
WARNINGS = -g -Wall

# Valgrind for memory issue
CHECKCC = valgrind
CHECKFLAGS = --leak-check=full -s --show-leak-kinds=all --track-origins=yes --log-file=valgrind.log

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
	$(Q)$(CXX) $(WARNINGS) $(CXXFLAGS) $(OPTFLAGS) $^ -o $@ $(LINKER)

# Include generated dependency files
-include $(DEPS)

# Compilation rule for object files with automatic dependency generation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) Makefile
	$(VECHO) "	CC\t$@\n"
	$(Q)$(CXX) $(WARNINGS) $(CXXFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

run1:
	./$(BIN) testcase/sampleCase testcase/sampleCase.out
	chmod +x sanity_checker/sanity
	./sanity_checker/sanity testcase/sampleCase testcase/sampleCase.out

run2:
	./$(BIN) testcase/sample.txt testcase/sample.txt.out
	chmod +x sanity_checker/sanity
	./sanity_checker/sanity testcase/sample.txt testcase/sample.txt.out

run3:
	./$(BIN) testcase/testcase1.txt testcase/testcase1.txt.out
	chmod +x sanity_checker/sanity
	./sanity_checker/sanity testcase/testcase1.txt testcase/testcase1.txt.out

run4:
	./$(BIN) testcase/testcase1_0614.txt testcase/testcase1_0614.txt.out
	chmod +x sanity_checker/sanity
	./sanity_checker/sanity testcase/testcase1_0614.txt testcase/testcase1_0614.txt.out

visual4:
	./$(BIN) testcase/testcase1_0614.txt testcase/testcase1_0614.txt.out drawDie/testcase/testcase1_0614_visual.txt.out
	chmod +x drawDie/drawDie
	./drawDie/drawDie -i ./drawDie/testcase/testcase1_0614_visual.txt.out -m die_pic.png -t testcase1_0614 -g -p -nl -o -c ./drawDie/testcase/cell_lst2.txt -n ./drawDie/testcase/net_lst2.txt

v1:
	./$(BIN) testcase/sampleCase testcase/sampleCase.out testcase/sampleCase_visual.txt.out
	chmod +x drawDie/drawDie
	./drawDie/drawDie -i testcase/sampleCase_visual.txt.out -m die_pic.png -t sampleCase -g -p -nl -o

v2:
	./$(BIN) testcase/sample.txt testcase/sample.out testcase/sample_visual.txt.out
	chmod +x drawDie/drawDie
	./drawDie/drawDie -i testcase/sample_visual.txt.out -m die_pic.png -t sample -g -p -nl -o

v4:
	./$(BIN) testcase/testcase1_0614.txt testcase/testcase1_0614.txt.out testcase/testcase1_0614_visual.txt.out
	chmod +x drawDie/drawDie
	./drawDie/drawDie -i testcase/testcase1_0614_visual.txt.out -m die_pic.png -t testcase1_0614 -g -p -nl -o

valgrind:
	./$(BIN) testcase/testcase1_0614.txt testcase/testcase1_0614.txt.out testcase/testcase1_0614_visual.txt.out
	cat valgrind.log

cppcheck:
	$(CPPCHECKCC) $(CPPCHECKFLAGS) -I$(INCDIR) $(SRCDIR) main.cpp $(INCDIR)/*.h

calc:
	(find src inc -type f -name "*.cpp" -o -name "*.h" && echo main.cpp) | xargs wc -l | tail -n 1
	find inc src main.cpp drawDie/drawDie.py -type f -name '*.*' -exec git blame {} \; | grep "coherent17" | wc -l
	find inc src main.cpp drawDie/drawDie.py -type f -name '*.*' -exec git blame {} \; | grep "c119cheng" | wc -l
	find inc src main.cpp drawDie/drawDie.py -type f -name '*.*' -exec git blame {} \; | grep "clairekuo" | wc -l

boost:
	chmod +x scripts/GetBoost.sh
	./scripts/GetBoost.sh
	rm boost_1_84_0.tar.gz

gitlog:
	git log --graph --decorate --oneline

clean:
	rm -rf $(OBJDIR) $(BIN) testcase/*.out *.log


# TODO, make release to compile with static-linking
# ref: https://github.com/fbacchus/MaxHS/issues/3 valgrind error when using valgrind