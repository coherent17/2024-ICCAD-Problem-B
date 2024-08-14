#!/bin/bash

# Define variables (these should match your Makefile variables)
OBJDIR="./obj/"  # Replace with the actual path to your object directory
RELEASEFLAGS="-DNDEBUG"  # Replace with your release flags
DEBUGFLAGS="-DENABLE_DEBUG_DP -DENABLE_DEBUG_LGZ -DENABLE_DEBUG_CHECKER -DENABLE_DEBUG_TIMER -DENABLE_DEBUG_MS -DENABLE_DEBUG_BAN"  # Replace with your debug flags
TESTCASES="./testcase/testcase1_0812.txt ./testcase/testcase2_0812.txt"  # Replace with your actual test cases

# Remove object files directory
rm -rf "$OBJDIR"

# Build the release version
make DEBUGFLAGS="$RELEASEFLAGS" BIN=cadb0015_release -j

# Remove object files directory again
rm -rf "$OBJDIR"

# Build the debug version
make DEBUGFLAGS="$DEBUGFLAGS" BIN=cadb0015 -j

echo -e "\nCompare with normal version"

# Loop over each test case
for testcase in $TESTCASES; do
    # Run the debug version and capture output
    ./cadb0015 "$testcase" "$testcase.out" > /dev/null
    
    # Measure runtime of the release version
    start_time=$(date +%s.%N)
    ./cadb0015_release "$testcase" "$testcase.release.out"
    end_time=$(date +%s.%N)
    
    # Calculate duration
    duration=$(echo "$end_time - $start_time" | bc)
    
    # Compare outputs
    if diff "$testcase.out" "$testcase.release.out" > /dev/null; then
        echo "Correct: $testcase"
    else
        echo "Fail: $testcase"
    fi
    
    # Output runtime
    echo "Runtime for $testcase: $duration seconds"
    echo
done
