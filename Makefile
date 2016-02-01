EXE	= out
DIRS	= actions split_pattern
SRCS	=	$(wildcard src/*.cpp)
SRCS2 =	$(wildcard src/actions/*.cpp)
SRCS3 =	$(wildcard src/split_pattern/*.cpp)

LIBS	=  -lCGAL
CFLAGS = -Wall -std=c++11 -g -frounding-math

OBJS	= $(addprefix obj/,$(notdir $(SRCS:.cpp=.o)))
OBJS2 = $(addprefix obj/actions/,$(notdir $(SRCS2:.cpp=.o)))
OBJS3 = $(addprefix obj/split_pattern/,$(notdir $(SRCS3:.cpp=.o)))

$(EXE): $(OBJS) $(OBJS2) $(OBJS3)
	g++ $(OBJS) $(OBJS2) $(OBJS3) -o $(EXE) $(LIBS)

clean:
	rm -f *~ $(EXE) $(OBJS) $(OBJS2) $(OBJS3)

rm_parsers:
	cd src/ ;	for d in $(DIRS); do (cd $$d ; rm -f *_parser* *_lexer*) done

gen_parsers:
	cd src/ ;	for d in $(DIRS); do (cd $$d ; flex -d *.l ; bison *.y) done

obj:
	mkdir obj; cd obj; for d in $(DIRS); do (mkdir $$d) done

obj/main.o: src/main.cpp
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
obj/%.o: src/%.cpp src/%.h
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
obj/actions/%.o: src/actions/%.cpp src/actions/%.h
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
obj/split_pattern/%.o: src/split_pattern/%.cpp src/split_pattern/%.h
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
