/**
 * @file loader.h
 * @brief Deklarasi antarmuka untuk sistem pemuatan sumber daya (I/O).
 * 
 * Menyediakan utilitas untuk membaca dan memuat set data eksternal
 * ke dalam memori permainan secara efisien.
 */

#pragma once

// Alasan penggunaan <string>: Mendukung passing path file.
#include <string>
#include "dataStructs.h"

// Deklarasi fungsi-fungsi Sistem Pemuatan (Loader)

/**
 * @brief Memuat daftar kata dari file eksternal ke dalam antrean (Queue).
 * 
 * @param filePath Path atau rute menuju file word bank.
 * @param targetQueue Pointer ke struktur data Queue untuk menampung kata.
 * @return true Jika pemuatan berhasil.
 * @return false Jika file tidak ditemukan atau gagal dimuat.
 */
bool loadWordsFromFile(std::string filePath, Queue* targetQueue);
