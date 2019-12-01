cmake_minimum_required( VERSION 3.5 )

project(contract)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract(rem.backing rem.backing src/rem.backing.cpp)

target_include_directories(rem.backing
        PUBLIC
        include)

set_target_properties(rem.backing
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( rem.backing PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
