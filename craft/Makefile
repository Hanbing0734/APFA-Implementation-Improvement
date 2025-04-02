CXX = g++
CXXFLAGS += -Wall -Wextra -Wpedantic -Wredundant-decls \
	-Wshadow -Wpointer-arith
VPATH := src
SOURCES = util.cpp clausenode.cpp 
HEADERS = params.h util.h clausenode.h craftfunctions.h
SATFLAGS = -lcryptominisat5


.PHONY: all clean run kill

all :\
	bin/craft \
	bin/craft_NS \


bin/craft: testCraft.cpp $(SOURCES) $(HEADERS) craftfunctions.cpp
	$(CXX) $(CXXFLAGS) -DSAT_MODE=1 \
		-o $@ $^ $(SATFLAGS)
bin/craft_NS: testCraft.cpp $(SOURCES) $(HEADERS) craftfunctions.cpp
	$(CXX) $(CXXFLAGS) -DSAT_MODE=2 \
		-o $@ $^ $(SATFLAGS)
run: 
	bash ./runCraft.sh &

kill: 
	bash ./killCraft.sh &


clean:
	rm -f ./bin/craft*


