# Simple build without CMake:  make        (build everything)
#                              make test   (build and run the tests)
#                              make clean
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -Iinclude
LIB_SRC  := $(filter-out src/main.cpp,$(wildcard src/*.cpp))
LIB_OBJ  := $(LIB_SRC:src/%.cpp=build/%.o)
TESTS    := $(basename $(notdir $(wildcard tests/test_*.cpp)))

all: mess_tracker generate_sample_data

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

mess_tracker: $(LIB_OBJ) src/main.cpp
	$(CXX) $(CXXFLAGS) src/main.cpp $(LIB_OBJ) -o $@

generate_sample_data: $(LIB_OBJ) tools/generate_sample_data.cpp
	$(CXX) $(CXXFLAGS) tools/generate_sample_data.cpp $(LIB_OBJ) -o $@

build/%: tests/%.cpp $(LIB_OBJ)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJ) -o $@

test: $(addprefix build/,$(TESTS))
	@for t in $(TESTS); do echo "== $$t"; ./build/$$t || exit 1; done

clean:
	rm -rf build mess_tracker generate_sample_data

.PHONY: all test clean
