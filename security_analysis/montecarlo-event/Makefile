OUT = run
BINDIR = bin
CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -O3

$(BINDIR)/$(OUT): src/MonteCarlo.cpp
		$(CXX) $(CXXFLAGS) src/MonteCarlo.cpp -o $(BINDIR)/$(OUT)

.PHONY: clean
clean:
	rm -rf $(BINDIR)/$(OUT)
