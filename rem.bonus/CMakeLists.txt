cmake_minimum_required( VERSION 3.5 )

project(contract)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract(rem.bonus rem.bonus src/rem.bonus.cpp)

target_include_directories(rem.bonus
        PUBLIC
        include)

set_target_properties(rem.bonus
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( rem.bonus PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
