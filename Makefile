CXX = c++

CXXFLAGS = -std=c++14 -Wall -Weffc++ -g -O3

INC_DIR = inc
SRC_DIR	= src
OBJ_DIR = obj

OBJECTS = \
	$(OBJ_DIR)/main.o			\
	$(OBJ_DIR)/onitama.o


all:    	onitama

onitama:	$(OBJECTS)
		$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cc
		$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c -o $@  $<

clean:
		rm -f onitama
		rm -f $(OBJECTS)

rebuild:	clean all
