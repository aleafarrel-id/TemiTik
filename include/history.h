/**
 * @file history.h
 * @brief Deklarasi antarmuka untuk manajemen riwayat dan statistik pemain.
 * 
 * Menyediakan fungsionalitas penyimpanan (I/O) serta operasi manipulasi
 * sekumpulan data statistik permainan, termasuk pencarian dan pengurutan (sorting).
 */

#pragma once

// Menyediakan fungsionalitas deklarasi murni, tidak ada pemanggilan library STL.
#include "dataStructs.h"
#include <string>

// Deklarasi fungsi-fungsi Manajemen Riwayat (History)

/**
 * @brief Menyimpan data rekaman skor baru ke dalam file riwayat.
 * 
 * @param newRecord Objek data rekaman skor baru.
 */
void saveRecordToFile(ScoreRecord newRecord);

/**
 * @brief Menghapus seluruh data riwayat skor dari penyimpanan (file).
 */
void clearAllHistoryRecords();

/**
 * @brief Mengurutkan rekaman skor secara menaik (Ascending) berdasarkan kriteria tertentu.
 * 
 * @param records Array berisi daftar rekaman skor.
 * @param count Jumlah rekaman dalam array.
 */
void sortRecordsAscending(ScoreRecord* records, int count);

/**
 * @brief Mengurutkan rekaman skor secara menurun (Descending) berdasarkan kriteria tertentu.
 * 
 * @param records Array berisi daftar rekaman skor.
 * @param count Jumlah rekaman dalam array.
 */
void sortRecordsDescending(ScoreRecord* records, int count);

/**
 * @brief Memfilter rekaman menggunakan pencocokan parsial dan pencarian biner eksak.
 * Memenuhi spesifikasi PRD mata kuliah Struktur Data.
 * 
 * @param source Array sumber data.
 * @param sourceCount Jumlah data sumber.
 * @param dest Array tujuan hasil filter.
 * @param query Kata kunci pencarian.
 * @param isAscending Mode urutan saat ini (mempengaruhi Binary Search).
 * @return int Jumlah data hasil pemfilteran.
 */
int filterHistoryRecords(ScoreRecord* source, int sourceCount, ScoreRecord* dest, std::string query, bool isAscending);
