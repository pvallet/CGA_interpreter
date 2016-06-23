# CGA-_interpreter

This program is a procedural architecture generator inspired by CGA and CGA++ grammars  
https://www.cs.purdue.edu/homes/aliaga/cs535-12/lectures/grammars/proc-mod-bldgs.pdf  
http://research.michael-schwarz.com/publ/2015/cgapp/  

For more details read report.pdf  

The current project is built only for GNU/Linux. It requires:  
```
flex  
bison  
libcgal-dev  
```

You can also use `meshlab` for visualizing the `.obj` generated files. You'd need version 1.3.3 or higher as 1.3.2 importing of OBJs is broken.  

To compile, create the necessary obj folders for the first time  
```
make obj  
```

Then run  
```
make gen_parsers  
make  
```
