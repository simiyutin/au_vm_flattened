cmake_minimum_required(VERSION 3.9)
project(my_repo)

set(CMAKE_CXX_STANDARD 11)

add_executable(my_repo
        include/ast.h
        include/mathvm.h
        include/visitors.h
        my_include/astprinter.h
        my_include/astprinter_visitor.h
        my_include/bytecode_translator_visitor.h
        my_include/interpreter.h
        my_include/identity.h
        my_include/instruction_factory.h
        my_include/stack_frame.h
        vm/ast.cpp
        vm/interpreter.cpp
        vm/mathvm.cpp
        vm/parser.cpp
        vm/parser.h
        vm/scanner.cpp
        vm/scanner.h
        vm/translator.cpp
        vm/utils.cpp
        my_vm/astprinter.cpp
        my_vm/astprinter_visitor.cpp
        my_vm/bytecode_translator.cpp
        my_vm/bytecode_translator_visitor.cpp
        my_vm/interpreter.cpp
        main.cpp
        my_vm/translator_factory.cpp
        my_include/native_caller.h
        my_include/includes.h
        my_include/stack.h my_vm/native_caller.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
target_link_libraries(my_repo ${CMAKE_DL_LIBS})
