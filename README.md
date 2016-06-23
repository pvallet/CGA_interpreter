# CGA-_interpreter

This program is a procedural architecture generator inspired by CGA and CGA++ grammars  
https://www.cs.purdue.edu/homes/aliaga/cs535-12/lectures/grammars/proc-mod-bldgs.pdf  
http://research.michael-schwarz.com/publ/2015/cgapp/  

For more details read report.pdf  

This program requires:  
```flex  
bison  
libcgal-dev  
meshlab (to visualize .obj files ; you need at least the 1.3.3 version)  
```

To compile, create the necessary obj folders for the first time  
`make obj`  

Then run  
```make gen_parsers  
make  
```
