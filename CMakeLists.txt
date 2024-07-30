cmake_minimum_required(VERSION 3.27)
project(ecommerce)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE) # Enforce the standard
set(CMAKE_CXX_EXTENSIONS OFF) # Disable compiler-specific extensions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic") #-Werror

# Specify the directories for the binary and object files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/obj)

add_executable(ecommerce
        src/main.cpp
        src/db/dbutils.cpp
        src/db/PostgresConnectionPool.cpp
        src/redis/rdutils.cpp
        src/redis/RedisConnectionPool.cpp
        src/models/User.cpp
        src/models/Customer.cpp
        src/models/Supplier.cpp
        src/models/Transporter.cpp
        src/Utils.cpp
        src/models/Order.cpp
)

# Link to redis and postgresql (including C++ versions)
target_link_libraries(ecommerce PRIVATE -lredis++ -lhiredis -lpqxx -lpq)
