<div align="center">
  <h1>TemiTik (Terminal Mengetik)</h1>
  <p><i>A fast-paced TUI typing game implemented in raw procedural C++.</i></p>
  
  [![C++17](https://img.shields.io/badge/C++-17-blue.svg?logo=c%2B%2B)](https://isocpp.org/)
  [![CMake](https://img.shields.io/badge/CMake-Ready-064F8C.svg?logo=cmake)](https://cmake.org/)
  [![Paradigm](https://img.shields.io/badge/Paradigm-Procedural-success.svg)](#)
  [![Platform](https://img.shields.io/badge/Platform-Terminal%20%2F%20CLI-lightgrey.svg)](#)
</div>

## 📌 About The Project
**TemiTik** is a terminal-based typing game designed as a Data Structures final project. The game challenges your speed and accuracy in typing words that fall from the top of the screen before they hit the bottom. 

This project strictly adheres to a **Procedural C++ (Non-OOP)** paradigm and manually implements foundational Data Structures (Queues, Pointers, Arrays, Sorting, and Searching) entirely from scratch without the use of the C++ Standard Template Library (STL).

## 🎮 Features
- **Dynamic Word Drop**: Words drop at accelerating speeds as your score increases.
- **Asynchronous Input**: Smooth, non-blocking terminal gameplay.
- **Local History System**: Records scores and allows viewing sorted history (ascending/descending).
- **Manual Data Structures**: Utilizes raw pointers, manual queues, Selection/Bubble sort, and Binary Search mechanics.

## 📂 Project Structure
```text
TemiTik/
├── CMakeLists.txt           # Build configuration
├── blueprint/               # TUI mockups and design blueprints
├── data/                    # Static resources (wordBank.txt, historyData.txt)
├── documentation/           # Obsidian Vault knowledge base (PRD)
├── include/                 # Header files (Structs & Declarations)
└── src/                     # C++ Implementation files (Core Logic)
```

## 🛠️ Build & Run
Ensure you have a C++ compiler (like GCC or MSVC) and **CMake** installed.

```bash
# 1. Clone the repository
git clone https://github.com/aleafarrel-id/TemiTik.git
cd TemiTik

# 2. Build the project using CMake
mkdir build && cd build
cmake ..
cmake --build .

# 3. Run the executable
./TemiTik
```

---
*Developed using strict clean code and procedural programming principles.*
