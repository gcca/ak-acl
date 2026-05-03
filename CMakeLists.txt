cmake_minimum_required(VERSION 3.20)

project(ak-acl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(
  ak-acl
  src/ak-acl.cc
  src/ak_acl/handlers/auth.cpp
  src/ak_acl/handlers/auth/utils.cpp
  src/ak_acl/handlers/inventory.cpp
  src/ak_acl/handlers/management.cpp
  src/ak_acl/handlers/middlewares.cpp
  src/ak_acl/utils.cpp
  views/AuthSignIn.cc
  views/ManagementEmployeeControl.cc
  views/ManagementEmployeeControlList.cc
  views/ManagementEmployeeControlCreate.cc)
add_executable(ak-acl_cmd_create-application
               cmd/ak-acl_cmd_create-application.cc src/ak_acl/utils.cpp)
add_executable(ak-acl_cmd_create-user cmd/ak-acl_cmd_create-user.cc
                                      src/ak_acl/utils.cpp)

target_include_directories(ak-acl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ak-acl PRIVATE Drogon::Drogon OpenSSL::Crypto)

target_include_directories(ak-acl_cmd_create-application
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ak-acl_cmd_create-application PRIVATE sqlite3
                                                            OpenSSL::Crypto)

target_include_directories(
  ak-acl_cmd_create-user PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
                                 ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
target_link_libraries(ak-acl_cmd_create-user PRIVATE sqlite3 OpenSSL::Crypto)
