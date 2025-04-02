CXX = g++
CXXFLAGS += -Wall -Wextra -Wpedantic -Wredundant-decls \
	-Wshadow -Wpointer-arith
VPATH := src
SOURCES = util.cpp clausenode.cpp 
HEADERS = params.h util.h clausenode.h skinnyfunctions.h
SATFLAGS = -lcryptominisat5


.PHONY: all clean run kill

all :\
	bin/skinny_64_1 \
	bin/skinny_64_1_NS \
	bin/skinny_64_2 \
	bin/skinny_64_2_NS \
	bin/skinny_64_3 \
	bin/skinny_64_3_NS \


bin/skinny_64_1: testSkinny.cpp $(SOURCES) $(HEADERS) skinnyfunctions.cpp
	$(CXX) $(CXXFLAGS) -DSKINNY_MODE=64 -DTWEAKEY_MODE=1 -DSAT_MODE=1 \
		-o $@ $^ $(SATFLAGS)
bin/skinny_64_1_NS: testSkinny.cpp $(SOURCES) $(HEADERS) skinnyfunctions.cpp
	$(CXX) $(CXXFLAGS) -DSKINNY_MODE=64 -DTWEAKEY_MODE=1 -DSAT_MODE=2 \
		-o $@ $^ $(SATFLAGS)
bin/skinny_64_2: testSkinny.cpp $(SOURCES) $(HEADERS) skinnyfunctions.cpp
	$(CXX) $(CXXFLAGS) -DSKINNY_MODE=64 -DTWEAKEY_MODE=2 -DSAT_MODE=1 \
		-o $@ $^ $(SATFLAGS)
bin/skinny_64_2_NS: testSkinny.cpp $(SOURCES) $(HEADERS) skinnyfunctions.cpp
	$(CXX) $(CXXFLAGS) -DSKINNY_MODE=64 -DTWEAKEY_MODE=2 -DSAT_MODE=2 \
		-o $@ $^ $(SATFLAGS)
bin/skinny_64_3: testSkinny.cpp $(SOURCES) $(HEADERS) skinnyfunctions.cpp
	$(CXX) $(CXXFLAGS) -DSKINNY_MODE=64 -DTWEAKEY_MODE=3 -DSAT_MODE=1 \
		-o $@ $^ $(SATFLAGS)
bin/skinny_64_3_NS: testSkinny.cpp $(SOURCES) $(HEADERS) skinnyfunctions.cpp
	$(CXX) $(CXXFLAGS) -DSKINNY_MODE=64 -DTWEAKEY_MODE=3 -DSAT_MODE=2 \
		-o $@ $^ $(SATFLAGS)


run: 
	bash ./runSkinny.sh &

kill: 
	bash ./killSkinny.sh &


clean:
	rm -f ./bin/skinny*


