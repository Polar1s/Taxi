CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(gptree)

ADD_COMPILE_OPTIONS(-O3 -std=c++11)

LINK_LIBRARIES(metis)

ADD_EXECUTABLE(
    gptreetest
    src/GPTree.cpp
    src/GPTree.h
    src/GPTreeTest.cpp
)

ADD_LIBRARY(
    taxi SHARED
    src/GPTree.cpp
    src/GPTree.h
    src/Taxi.cpp
)
