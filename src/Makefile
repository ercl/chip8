CXX = g++
CXXFLAGS = -std=c++14 
SDL2FLAGS = $(shell sdl2-config --libs --cflags)
SDL2MIXERFLAGS = -lSDL2_mixer

OBJECTS = main.o chip8.o

main.out: $(OBJECTS)
		$(CXX) $(CXXFLAGS) $(SDL2FLAGS) $(SDL2MIXERFLAGS) -framework OpenGL -o $@ $^

clean:
	rm *.o *.out

$(OBJECTS): chip8.h
