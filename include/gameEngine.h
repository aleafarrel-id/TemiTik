/**
 * @file gameEngine.h
 * @brief Deklarasi antarmuka untuk logika utama permainan.
 * 
 * Mendeklarasikan fungsi-fungsi yang mengelola mekanika gameplay,
 * termasuk pergerakan objek, pemrosesan input, perhitungan skor, 
 * dan manajemen antrean objek dalam permainan.
 */

#pragma once

// Tidak ada pemanggilan library standar C++ selain dataStructs.h untuk mematuhi aturan pure procedural.
#include "dataStructs.h"
#include <string>

// Deklarasi fungsi-fungsi Sistem Permainan (Game Engine)

/**
 * @brief Menjalankan siklus permainan utama (Game Loop).
 * 
 * @param playerState Pointer ke status pemain saat ini.
 * @param currentState Referensi ke state game untuk mengontrol transisi.
 * @param wordQueue Pointer ke antrean kata yang akan dimainkan.
 */
void runGameLoop(PlayerState* playerState, GameState& currentState, Queue* wordQueue);

/**
 * @brief Menghitung dan memperbarui posisi vertikal kata yang jatuh.
 * 
 * @param word Pointer ke objek kata yang sedang aktif.
 */
void calculateWordDrop(WordItem* word);

/**
 * @brief Memvalidasi input ketikan pemain terhadap kata yang aktif.
 * 
 * @param input String ketikan pemain saat ini.
 * @param activeWord Pointer ke kata yang sedang aktif di layar.
 * @return true Jika input pemain cocok dengan kata.
 * @return false Jika input belum selesai atau salah.
 */
bool validatePlayerInput(std::string input, WordItem* activeWord);
