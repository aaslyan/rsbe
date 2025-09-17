cmake_minimum_required(VERSION 3.10)

# UTP Client executable
set(UTP_CLIENT_SOURCES
    UTPClient.cpp
    utp_client_main.cpp
)

add_executable(utp_client ${UTP_CLIENT_SOURCES})

# Set C++ standard
target_compile_features(utp_client PRIVATE cxx_std_17)

# Compile flags
target_compile_options(utp_client PRIVATE -Wall -Wextra -O2 -pthread)

# Link libraries
target_link_libraries(utp_client PRIVATE pthread)

# Include directories
target_include_directories(utp_client PRIVATE .)

# Install target
install(TARGETS utp_client DESTINATION bin)