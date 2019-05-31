TARGET_CPPS := packed_sort.cpp radix_sort.cpp
CPP_FILES := $(filter-out $(TARGET_CPPS),$(wildcard *.cpp))
OBJ_FILES := $(CPP_FILES:.cpp=.o)

CPP_FLAGS = --std=c++17 

all: bitonic_sorting packed_sort

$(OBJ_FILES) $(TARGET_CPPS:.cpp=.o): Makefile

explore:$(OBJ_FILES) packed_sort.o
	g++ -o $@ $^

%.o: %.cpp
	g++ -c $(CPP_FLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o packed_sort radix_sort*~
