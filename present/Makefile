CXX = g++
CXXFLAGS += -Wall -Wextra -Wpedantic -Wredundant-decls \
	-Wshadow -Wpointer-arith
VPATH := src
SOURCES = util.cpp clausenode.cpp 
HEADERS = params.h util.h clausenode.h presentfunctions.h
SATFLAGS = -lcryptominisat5


.PHONY: all clean run kill

all :\
	bin/present80 \
	bin/present80NS \
	bin/present80NSNK \
	bin/present128 \
	bin/present128NS \
	bin/present128NSNK \


bin/present80: testPresent.cpp $(SOURCES) $(HEADERS) present80functions.cpp
	$(CXX) $(CXXFLAGS) -DPRESENT_MODE=80 -DSAT_MODE=0 \
		-o $@ $^ $(SATFLAGS)
bin/present80NS: testPresent.cpp $(SOURCES) $(HEADERS) present80functions.cpp
	$(CXX) $(CXXFLAGS) -DPRESENT_MODE=80 -DSAT_MODE=1 \
		-o $@ $^ $(SATFLAGS)
bin/present80NSNK: testPresent.cpp $(SOURCES) $(HEADERS) present80functions.cpp
	$(CXX) $(CXXFLAGS) -DPRESENT_MODE=80 -DSAT_MODE=2 \
		-o $@ $^ $(SATFLAGS)


bin/present128: testPresent.cpp $(SOURCES) $(HEADERS) present128functions.cpp
	$(CXX) $(CXXFLAGS) -DPRESENT_MODE=128 -DSAT_MODE=0 \
		-o $@ $^ $(SATFLAGS)
bin/present128NS: testPresent.cpp $(SOURCES) $(HEADERS) present128functions.cpp
	$(CXX) $(CXXFLAGS) -DPRESENT_MODE=128 -DSAT_MODE=1 \
		-o $@ $^ $(SATFLAGS)
bin/present128NSNK: testPresent.cpp $(SOURCES) $(HEADERS) present128functions.cpp
	$(CXX) $(CXXFLAGS) -DPRESENT_MODE=128 -DSAT_MODE=2 \
		-o $@ $^ $(SATFLAGS)

run: 
	bash ./runPresent.sh &

kill: 
	bash ./killPresent.sh &


clean:
	rm -f ./bin/present*


