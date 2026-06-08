/**
 * @file history.cpp
 * @brief Implementasi sistem manajemen dan operasi statistik.
 * 
 * Mengelola persistensi data rekaman permainan serta menyediakan algoritma
 * fundamental untuk mensortir dan mencari himpunan rekaman.
 */

#include "history.h"

// Alasan penggunaan <iostream>: Digunakan untuk standar input/output (sementara untuk stub/debugging).
#include <iostream>

// Alasan penggunaan <fstream>: Digunakan untuk operasi manipulasi file (baca/tulis data riwayat) tanpa melanggar aturan logika manual.
#include <fstream>

// Alasan penggunaan <string>: Diperlukan untuk manipulasi jalur file.
#include <string>

using namespace std;

// Konstanta rute untuk penyimpanan berkas riwayat permainan
const string HISTORY_FILE_PATH = "data/historyData.txt";

void saveRecordToFile(ScoreRecord newRecord) {
    // Menggunakan mode ios::app untuk menyisipkan (append) data di akhir berkas agar riwayat lama tidak hilang
    ofstream fileStream(HISTORY_FILE_PATH, ios::app);
    if (fileStream.is_open()) {
        // Menyimpan nilai properti secara berurutan dipisahkan spasi kosong
        fileStream << newRecord.score << " " << newRecord.playTimeInSeconds << "\n";
        fileStream.close();
    }
}

void clearAllHistoryRecords() {
    // Menggunakan mode ios::trunc yang secara otomatis menghapus bersih isi berkas saat dibuka
    ofstream fileStream(HISTORY_FILE_PATH, ios::trunc);
    if (fileStream.is_open()) {
        fileStream.close();
    }
}

void sortRecordsAscending(ScoreRecord* records, int count) {
    // Penggunaan manual algoritma Bubble Sort untuk memenuhi larangan penggunaan <algorithm> std::sort
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            // Membandingkan properti skor dari elemen berdekatan
            if (records[j].score > records[j + 1].score) {
                // Melakukan pertukaran (swap) data manual menggunakan variabel penampung sementara
                ScoreRecord temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
            }
        }
    }
}

void sortRecordsDescending(ScoreRecord* records, int count) {
    // Penggunaan manual algoritma Bubble Sort untuk urutan menurun
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            // Membalikkan logika operator komparasi
            if (records[j].score < records[j + 1].score) {
                ScoreRecord temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
            }
        }
    }
}
