cmake_minimum_required (VERSION 2.6)

#The name of the .sln file
project (CatchMyTail)

#Adding VC Projects
add_subdirectory(Core)
add_subdirectory(Server)

#Setting Dependencies
add_dependencies(CatchMyTailServer ServerCore)