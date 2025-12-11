project(file-storing-service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
        src/main.cpp
        src/config/config.cpp
        src/db/database.cpp
        src/utils/hashutils.cpp
        src/repository/filerepository.cpp
        src/service/fileservice.cpp
        src/handlers/filehandlers.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/libs
)

find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

target_include_directories(${PROJECT_NAME} PRIVATE ${PQXX_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        ${PQXX_LIBRARIES}
        pthread
)