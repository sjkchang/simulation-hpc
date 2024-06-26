# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sjkchang/classes/275/projects/simulation-code/cpp-src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b

# Include any dependencies generated for this target.
include src/CMakeFiles/basic_socket.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/basic_socket.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/basic_socket.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/basic_socket.dir/flags.make

src/CMakeFiles/basic_socket.dir/app/main.cpp.o: src/CMakeFiles/basic_socket.dir/flags.make
src/CMakeFiles/basic_socket.dir/app/main.cpp.o: ../src/app/main.cpp
src/CMakeFiles/basic_socket.dir/app/main.cpp.o: src/CMakeFiles/basic_socket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/basic_socket.dir/app/main.cpp.o"
	cd /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/src && mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/basic_socket.dir/app/main.cpp.o -MF CMakeFiles/basic_socket.dir/app/main.cpp.o.d -o CMakeFiles/basic_socket.dir/app/main.cpp.o -c /home/sjkchang/classes/275/projects/simulation-code/cpp-src/src/app/main.cpp

src/CMakeFiles/basic_socket.dir/app/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/basic_socket.dir/app/main.cpp.i"
	cd /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/src && mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sjkchang/classes/275/projects/simulation-code/cpp-src/src/app/main.cpp > CMakeFiles/basic_socket.dir/app/main.cpp.i

src/CMakeFiles/basic_socket.dir/app/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/basic_socket.dir/app/main.cpp.s"
	cd /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/src && mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sjkchang/classes/275/projects/simulation-code/cpp-src/src/app/main.cpp -o CMakeFiles/basic_socket.dir/app/main.cpp.s

# Object files for target basic_socket
basic_socket_OBJECTS = \
"CMakeFiles/basic_socket.dir/app/main.cpp.o"

# External object files for target basic_socket
basic_socket_EXTERNAL_OBJECTS =

lib/libbasic_socket.a: src/CMakeFiles/basic_socket.dir/app/main.cpp.o
lib/libbasic_socket.a: src/CMakeFiles/basic_socket.dir/build.make
lib/libbasic_socket.a: src/CMakeFiles/basic_socket.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library ../lib/libbasic_socket.a"
	cd /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/src && $(CMAKE_COMMAND) -P CMakeFiles/basic_socket.dir/cmake_clean_target.cmake
	cd /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/basic_socket.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/basic_socket.dir/build: lib/libbasic_socket.a
.PHONY : src/CMakeFiles/basic_socket.dir/build

src/CMakeFiles/basic_socket.dir/clean:
	cd /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/src && $(CMAKE_COMMAND) -P CMakeFiles/basic_socket.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/basic_socket.dir/clean

src/CMakeFiles/basic_socket.dir/depend:
	cd /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sjkchang/classes/275/projects/simulation-code/cpp-src /home/sjkchang/classes/275/projects/simulation-code/cpp-src/src /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/src /home/sjkchang/classes/275/projects/simulation-code/cpp-src/b/src/CMakeFiles/basic_socket.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/basic_socket.dir/depend

