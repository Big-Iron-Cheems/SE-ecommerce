cmake_minimum_required(VERSION 3.27)
project(ecommerce)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") #-Werror -Wpedantic

# Specify the directories for the binary and object files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/obj)

add_executable(ecommerce
        src/main.cpp
        src/db/dbutils.cpp
        src/redis/rdutils.cpp
        src/models/Customer.cpp
        src/models/Supplier.cpp
        src/models/Transporter.cpp
        src/utils.cpp
)

# Link to redis and postgresql (including C++ versions)
target_link_libraries(ecommerce PRIVATE -lredis++ -lhiredis /usr/local/lib/libpqxx.a -lpq)
# FIXME: sudo pacman -S libpqxx and -lpqxx, once the library is updated to 7.9.0
# Used: https://github.com/jtv/libpqxx/blob/master/BUILDING-configure.md
