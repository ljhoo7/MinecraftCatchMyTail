cmake_minimum_required (VERSION 2.6)

#The name of the .sln file
project (GenericBoson)

#Adding VC Projects
add_subdirectory(Core)
add_subdirectory(Server)

#Setting Dependencies
add_dependencies(Server Core)