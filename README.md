# CGA-_interpreter

This is actually an interpreter of the architectural grammar CGA++

The paper is accessible here :

http://research.michael-schwarz.com/publ/2015/cgapp/

This is currently a WIP for a school project

This program requires :
  flex
  bison
  libcgal-dev
  meshlab (to visualize obj ; you need at least the 1.3.3 version)

To compile, create the necessary obj folders for the first time
  make obj

Then run
  make gen_parsers
  make
