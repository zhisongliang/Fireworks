OpenGL Fireworks
========================================
Library used: glad, glfw, glm

Concepts used: instance rendering, particle systems

Demo:
![]
(https://github.com/zhisongliang/Fireworks/blob/main/resources/fireworks.gif)

**Building and Running**
===========================================

All platforms
-------------

We'll perform an "out-of- source" build, which means that the binary files
will not be in the same directory as the source files. In the folder that
contains CMakeLists.txt, run the following.

	> mkdir build
	> cd build

Then run one of the following, depending on your choice of platform and IDE.

OSX & Linux Makefile
--------------------

	> cmake ..

This will generate a Makefile that you can use to compile your code. To
compile the code, run the generated Makefile.

	> make -j4

The `-j` argument speeds up the compilation by multithreading the compiler.
This will generate an executable, which you can run by typing

	> ./fireworks

!Note this assume a resources directory

To build in release mode, use `ccmake ..` and change `CMAKE_BUILD_TYPE` to
`Release`. Press 'c' to configure then 'g' to generate. Now `make -j4` will
build in release mode.

To change the compiler, read [this
page](http://cmake.org/Wiki/CMake_FAQ#How_do_I_use_a_different_compiler.3F).
The best way is to use environment variables before calling cmake. For
example, to use the Intel C++ compiler:

	> which icpc # copy the path
	> CXX=/path/to/icpc cmake ..

OSX Xcode
---------

	> cmake -G Xcode ..

This will generate `fireworks.xcodeproj` project that you can open with Xcode.

- To run, change the target to `project` by going to Product -> Scheme -> lab3.
  Then click on the play button or press Command+R to run the application.
- Edit the scheme to add command-line arguments (`../../resources`) or to run
  in release mode.

Windows Visual Studio 2015
--------------------------

	> cmake -G "Visual Studio 14 2015" ..

This will generate `fireworks.sln` file that you can open with Visual Studio.
Other versions of Visual Studio are listed on the CMake page
(<https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html>).

- To build and run the project, right-click on `fireworks` in the project explorer
  and then click on "Set as Startup Project." Then press F7 (Build Solution)
  and then F5 (Start Debugging).
- To add a commandline argument (`../resources`), right-click on `fireworks` in
  the project explorer and then click on "Properties" and then click to
  "Debugging."
