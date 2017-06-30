find_package(LLVM REQUIRED CONFIG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11 -fdiagnostics-color -g -UNDEBUG")

# LLVM detection part
set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")

# A bit of a sanity check:
if( NOT EXISTS ${LLVM_ROOT}/include/llvm )
message(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM install")
endif()

# Incorporate the CMake features provided by LLVM:
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_ROOT}/share/llvm/cmake")

# only needed for multi target build
set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_CFG_INTDIR})
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_CFG_INTDIR})

# get regular includes
include(LLVMConfig)
include(HandleLLVMOptions)
include(AddLLVM)

# Now set the header and library paths:
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(StackShieldPass)
add_subdirectory(tests)

enable_testing()
add_test(test1 tests/tests)