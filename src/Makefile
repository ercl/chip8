CXX = g++
CXXFLAGS = -std=c++14 
SDLFLAGS = $(shell sdl2-config --libs --cflags)

OBJECTS = main.o chip8.o

main.out: $(OBJECTS)
		$(CXX) $(CXXFLAGS) $(SDLFLAGS) -framework OpenGL -o $@ $^

clean:
	rm *.o *.out

$(OBJECTS): chip8.h
