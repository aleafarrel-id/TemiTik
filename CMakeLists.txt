# CMakeLists.txt
# File konfigurasi CMake untuk membangun proyek TemiTik secara otomatis.
# Menggabungkan seluruh file implementasi di folder src/ menjadi satu executable.

cmake_minimum_required(VERSION 3.10)
project(TemiTik)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Menyertakan folder include agar file header bisa diakses dari file sumber mana saja
include_directories(include)

# Mengumpulkan semua file .cpp di dalam direktori src/
file(GLOB SOURCES "src/*.cpp")

# Membuat executable target "TemiTik" dari semua source file
add_executable(TemiTik ${SOURCES})
