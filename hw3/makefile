LIBS := $(shell pkg-config --libs opencv)

CXX := mpic++
CXX_FLAGS := -Wall -Wextra -pedantic -std=c++03 -O2 -flto

NAME := mpi_processor_color

all:
	$(CXX) $(CXX_FLAGS) $(NAME).cpp $(LIBS) -o $(NAME)
	$(CXX) $(CXX_FLAGS) mpi_processor_gray.cpp $(LIBS) -o mpi_processor_gray
