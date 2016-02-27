EXE	= out
DIRS	= actions split_pattern cgacode
SRCS	=	$(wildcard src/*.cpp)
SRCS2 =	$(wildcard src/actions/*.cpp)
SRCS3 =	$(wildcard src/split_pattern/*.cpp)
SRCS4 =	$(wildcard src/cgacode/*.cpp)

LIBS	=  -lCGAL
CFLAGS = -Wall -std=c++11 -g -frounding-math

OBJS	= $(addprefix obj/,$(notdir $(SRCS:.cpp=.o)))
OBJS2 = $(addprefix obj/actions/,$(notdir $(SRCS2:.cpp=.o)))
OBJS3 = $(addprefix obj/split_pattern/,$(notdir $(SRCS3:.cpp=.o)))
OBJS4 = $(addprefix obj/cgacode/,$(notdir $(SRCS4:.cpp=.o)))

$(EXE): $(OBJS) $(OBJS2) $(OBJS3) $(OBJS4)
	g++ $(OBJS) $(OBJS2) $(OBJS3) $(OBJS4) -o $(EXE) $(LIBS)

clean:
	rm -f *~ $(EXE) $(OBJS) $(OBJS2) $(OBJS3) $(OBJS4)

rm_parsers:
	cd src/ ;	for d in $(DIRS); do (cd $$d ; rm -f *_parser* *_scanner.cpp stack.hh) done

gen_parsers:
	cd src/ ;	for d in $(DIRS); do (cd $$d ; flex -d *.l ; bison *.y) done

obj:
	mkdir obj; cd obj; for d in $(DIRS); do (mkdir $$d) done

obj/main.o: src/main.cpp
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
obj/%.o: src/%.cpp src/%.h
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
obj/actions/%.o: src/actions/%.cpp
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
obj/split_pattern/%.o: src/split_pattern/%.cpp
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
obj/cgacode/%.o: src/cgacode/%.cpp
	g++ -c $< -o $@ $(LIBS) $(CFLAGS)
