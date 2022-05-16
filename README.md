# Bezier Curves with GPU shaders

A hardware-accelerated implementation of bezier curves, based on Blinn-Loop's method. The method is described in their 2005 SIGGRAPH paper:

https://doi.org/10.1145/1073204.1073303

https://charlesloop.com/LoopBlinn05.pdf

Written in C++17 and OpenGL 4.1.


To compile the project, make sure you have a working Internet connection, and have Cmake installed:
``` sh
$ git clone https://github.com/alifarazz/bezier_curve.git gh_alifarazz_bezier_curve
$ cd gh_alifarazz_bezier_curve
$ cmake -DCMAKE_BUILD_TYPE=Release -B build/ -S .
$ cd build
$ make -j
$ ./bezier ../src/shader/
```
