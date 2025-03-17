cmake_minimum_required(VERSION 3.30)
project(GraphTradingEngine)
set(CMAKE_CXX_STANDARD 20)


find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem thread)

if(Boost_LOG_FOUND)
    message(STATUS "Boost Log is installed")
else()
    message(STATUS "Boost Log is NOT installed")
endif()

if (Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
    message(STATUS "Boost.Thread version: ${Boost_THREAD_VERSION}")
endif()

include_directories(${Boost_INCLUDE_DIRS})



add_executable(GraphTradingEngine main.cpp
        DataReader/class_DataReader.h
        DataReader/class_DataReader.cpp
        Helper/class_FutureHelper.cpp
        Helper/class_FutureHelper.h
        Helper/class_DateHelper.cpp
        Helper/class_DateHelper.h
        Core/Node/class_Node.cpp
        Core/Node/class_Node.h
        Logger/class_Logger.cpp
        Logger/class_Logger.h
)


target_link_libraries(GraphTradingEngine boost_date_time arrow parquet boost_filesystem fmt)

target_link_libraries(GraphTradingEngine
        ${Boost_LOG_LIBRARY}    # Boost.Log library
        ${Boost_THREAD_LIBRARY}  # Boost.Thread library
        ${Boost_SYSTEM_LIBRARY}  # Boost.System library
        ${Boost_FILESYSTEM_LIBRARY} # Boost.Filesystem library
)