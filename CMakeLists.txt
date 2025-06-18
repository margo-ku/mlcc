cmake_minimum_required(VERSION 3.16...3.27)
project(mlcc)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
            COMMAND brew --prefix bison
            RESULT_VARIABLE BREW_BISON
            OUTPUT_VARIABLE BREW_BISON_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
        message(STATUS "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}")
        set(BISON_EXECUTABLE "${BREW_BISON_PREFIX}/bin/bison")
    endif ()

    execute_process(
            COMMAND brew --prefix flex
            RESULT_VARIABLE BREW_FLEX
            OUTPUT_VARIABLE BREW_FLEX_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
        message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
        set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex")
        include_directories(${BREW_FLEX_PREFIX}/include)
    endif ()
endif ()

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(CMAKE_CXX_STANDARD 20)

file(MAKE_DIRECTORY ${mlcc_SOURCE_DIR}/grammar/generated)
set(GENERATED_DIR ${mlcc_SOURCE_DIR}/grammar/generated)

set(
        HEADERS
        ${mlcc_SOURCE_DIR}/driver.h
        ${mlcc_SOURCE_DIR}/parser.hh
)

BISON_TARGET(
        MyParser
        grammar/parser.y
        ${GENERATED_DIR}/parser.cpp
        DEFINES_FILE ${GENERATED_DIR}/parser.hh
        COMPILE_FLAGS "-Wcounterexamples"
)

FLEX_TARGET(
        MyScanner
        grammar/lexer.l
        ${GENERATED_DIR}/scanner.cpp
)

ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${mlcc_SOURCE_DIR}/build)

set(
        AST_SOURCES
        src/ast/declarations.cpp
        src/ast/expressions.cpp
        src/ast/statements.cpp
        src/ast/translation_unit.cpp
)

set(
        DRIVER_SOURCES
        src/driver/driver.cpp
)

set(
        VISITOR_SOURCES
        src/visitors/print_visitor.cpp
        src/visitors/compile_visitor.cpp
)

add_executable(
        mlcc
        main.cpp
        ${AST_SOURCES}
        ${DRIVER_SOURCES}
        ${VISITOR_SOURCES}
        ${BISON_MyParser_OUTPUTS}
        ${FLEX_MyScanner_OUTPUTS}
)

target_include_directories(mlcc PRIVATE ${GENERATED_DIR})
target_include_directories(mlcc PRIVATE ${mlcc_SOURCE_DIR})