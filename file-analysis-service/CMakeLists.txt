cmake_minimum_required(VERSION 3.20)
project(file-analysis-service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Исходные файлы
set(SOURCES
        src/main.cpp
        src/config/config.cpp
        src/db/database.cpp
        src/repository/reportrepository.cpp
        src/clients/fileserviceclient.cpp
        src/service/analysisservice.cpp
        src/handlers/analysishandlers.cpp
)

# Создаём исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES})

# Include директории
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/libs
)

# Находим библиотеки
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Линкуем
target_include_directories(${PROJECT_NAME} PRIVATE ${PQXX_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        ${PQXX_LIBRARIES}
        pthread
)