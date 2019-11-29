cmake_minimum_required( VERSION 3.5 )

project(contract)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract(guardian.reward guardian.reward ${CMAKE_CURRENT_SOURCE_DIR}/src/guardian.reward.cpp)

target_include_directories(guardian.reward
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(guardian.reward
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( guardian.reward PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
