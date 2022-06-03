cmake_minimum_required(VERSION 3.22)
project(ModernDataBase)

set(CMAKE_CXX_STANDARD 14)

add_executable(ModernDataBase main.cpp src/Parser.cpp src/Parser.h src/Schema.cpp src/Schema.h src/Types.h src/BufferManager.cpp src/BufferManager.h src/BufferFrame.cpp src/BufferFrame.h src/SlottedPage.cpp src/SlottedPage.h src/Page.h src/TID.h src/Record.h)
