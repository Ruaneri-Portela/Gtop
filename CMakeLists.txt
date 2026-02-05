cmake_minimum_required(VERSION 3.10)
project(Gtop VERSION 0.1.0 LANGUAGES CXX)

add_executable(Gtop gpu.cpp iokit.cpp main.cpp)

# Linkar com frameworks do macOS
target_link_libraries(Gtop
    "-framework IOKit"
    "-framework CoreFoundation"
)

# Definir padrão C++ moderno
target_compile_features(Gtop PRIVATE cxx_std_17)
