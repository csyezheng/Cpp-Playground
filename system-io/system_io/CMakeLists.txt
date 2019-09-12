set(SYSTEM_IO_HEADER_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

foreach (dir ${SYSTEM_IO_HEADER_DIRS})
    file(GLOB_RECURSE headers ${dir}/*.h)
    set(SYSTEM_IO_HEADERS
            ${SYSTEM_IO_HEADERS}
            ${headers})
endforeach ()

set(SYSTEM_IO_SOURCES
        main.cpp
        File.cpp
        FileUtil.cpp
        LineReader.cpp
        ScopeGuard.cpp
        )

add_executable(system_io ${SYSTEM_IO_SOURCES})

target_include_directories(
        system_io
        PUBLIC
        ${GLOG_INCLUDE_DIRS}
        ${GFLAGS_INCLUDE_DIRS}
)

target_link_libraries(system_io
        PUBLIC
        ${GLOG_LIBRARY}
        ${GFLAGS_LIBRARIES}
        Threads::Threads
        )

add_library(system_io_lib
        ${SYSTEM_IO_HEADERS}
        ${SYSTEM_IO_SOURCES}
        )

target_include_directories(
        system_io_lib
        PUBLIC
        ${GLOG_INCLUDE_DIRS}
        ${GFLAGS_INCLUDE_DIRS}
)

target_link_libraries(system_io_lib
        PUBLIC
        ${GLOG_LIBRARY}
        ${GFLAGS_LIBRARIES}
        Threads::Threads
        )

option(BUILD_TESTS "BUILD_TESTS" ON)
if (BUILD_TESTS)
    option(USE_CMAKE_GOOGLE_TEST_INTEGRATION "If enabled, use the google test integration included in CMake." ON)
    find_package(GMock MODULE REQUIRED)
    if (USE_CMAKE_GOOGLE_TEST_INTEGRATION)
        include(GoogleTest OPTIONAL RESULT_VARIABLE HAVE_CMAKE_GTEST)
        enable_testing()
    else ()
        set(HAVE_CMAKE_GTEST OFF)
    endif ()

    macro(add_gtest test_source test_name)
        add_executable(${test_name} ${test_source} test/TestMain.cpp)
        target_include_directories(
                ${test_name} PUBLIC ${LIBGMOCK_INCLUDE_DIR} ${LIBGTEST_INCLUDE_DIR} ${GLOG_INCLUDE_DIRS}
        )
        target_link_libraries(
                ${test_name}
                system_io_lib
                ${LIBGMOCK_LIBRARIES}
                ${GLOG_LIBRARY}
        )
        add_test(${test_name} bin/${test_name})
    endmacro(add_gtest)

    add_gtest(test/FileTest.cpp FileTest)
    add_gtest(test/LineReaderTest.cpp LineReaderTest)
endif ()