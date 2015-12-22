CIBLE = out
SRCS =  $(wildcard src/*.cpp)
LIBS =  -lCGAL

CFLAGS = -Wall -std=c++11 -g -frounding-math

OBJS = $(addprefix obj/,$(notdir $(SRCS:.cpp=.o)))

$(CIBLE): $(OBJS)
	g++ $(OBJS) -o $(CIBLE) $(LIBS) $(LDFLAGS)

clean:
	rm -f *~ $(CIBLE) $(OBJS)
	rm -f src/*_parser* src/*_lexer*

gen_parsers:
	cd src/ ; flex -d *.l ; bison *.y

obj/main.o: src/main.cpp
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
obj/%.o: src/%.cpp src/%.h
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
