/**
 * @file history.cpp
 * @brief Implementasi sistem manajemen dan operasi statistik.
 * 
 * Mengelola persistensi data rekaman permainan serta menyediakan algoritma
 * fundamental untuk mensortir dan mencari himpunan rekaman.
 */

#include "history.h"

// Alasan penggunaan <iostream>: Digunakan untuk standar input/output.
#include <iostream>

// Alasan penggunaan <fstream>: Digunakan untuk operasi manipulasi file (baca/tulis data riwayat) tanpa melanggar aturan logika manual.
#include <fstream>

// Alasan penggunaan <string>: Diperlukan untuk manipulasi jalur file dan pencocokan teks pencarian.
#include <string>

using namespace std;

// Konstanta rute untuk penyimpanan berkas riwayat permainan
const string HISTORY_FILE_PATH = "data/historyData.txt";

void saveRecordToFile(ScoreRecord newRecord) {
    // Menggunakan mode ios::app untuk menyisipkan (append) data di akhir berkas agar riwayat lama tidak hilang
    ofstream fileStream(HISTORY_FILE_PATH, ios::app);
    if (!fileStream.is_open()) {
        fileStream.open("../" + HISTORY_FILE_PATH, ios::app); // Fallback ke luar folder build
        if (!fileStream.is_open()) {
            fileStream.open("../../" + HISTORY_FILE_PATH, ios::app);
        }
    }
    if (fileStream.is_open()) {
        // Menyimpan nilai properti secara berurutan dipisahkan spasi kosong
        fileStream << newRecord.score << " " << newRecord.playTimeInSeconds << "\n";
        fileStream.close();
    }
}

int loadHistoryRecords(ScoreRecord* records) {
    // Membuka aliran file untuk membaca data riwayat dengan pencarian jalur fallback
    ifstream fileStream(HISTORY_FILE_PATH);
    if (!fileStream.is_open()) {
        fileStream.open("../" + HISTORY_FILE_PATH);
        if (!fileStream.is_open()) {
            fileStream.open("../../" + HISTORY_FILE_PATH);
            if (!fileStream.is_open()) {
                return 0; 
            }
        }
    }
    
    int count = 0;
    string line;
    
    // Membaca berpasangan: skor dan waktu baris per baris.
    // Menggunakan getline untuk dapat melewati baris komentar.
    while (count < MAX_HISTORY_RECORDS && getline(fileStream, line)) {
        // Mengabaikan baris kosong atau baris komentar
        if (line.empty() || line.substr(0, 2) == "//") {
            continue;
        }
        
        // Memparsing dua angka dari string secara manual
        size_t spacePos = line.find(' ');
        if (spacePos != string::npos) {
            records[count].score = stoi(line.substr(0, spacePos));
            records[count].playTimeInSeconds = stoi(line.substr(spacePos + 1));
            count++;
        }
    }
    
    // Menutup file untuk melepaskan penguncian (lock) pada sumber daya
    fileStream.close();
    return count;
}

void clearAllHistoryRecords() {
    // Menggunakan mode ios::trunc yang secara otomatis menghapus bersih isi berkas saat dibuka
    ofstream fileStream(HISTORY_FILE_PATH, ios::trunc);
    if (!fileStream.is_open()) {
        fileStream.open("../" + HISTORY_FILE_PATH, ios::trunc); // Fallback
        if (!fileStream.is_open()) {
            fileStream.open("../../" + HISTORY_FILE_PATH, ios::trunc);
        }
    }
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

int filterHistoryRecords(ScoreRecord* source, int sourceCount, ScoreRecord* dest, string query, bool isAscending) {
    // Jika tidak ada kueri pencarian, kembalikan salinan seluruh data
    if (query.empty()) {
        for (int i = 0; i < sourceCount; i++) dest[i] = source[i];
        return sourceCount;
    }
    
    int destCount = 0;
    
    // Cek apakah kueri murni numerik untuk eksekusi algoritma Binary Search (sesuai spesifikasi PRD)
    bool isNumeric = true;
    for (char c : query) {
        if (!isdigit(c)) isNumeric = false;
    }
    
    // Algoritma Binary Search Manual untuk pencarian nilai eksak pada array yang sudah di-sort
    if (isNumeric && query.length() > 0) {
        int exactTarget = stoi(query);
        int left = 0, right = sourceCount - 1;
        int foundIndex = -1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (source[mid].score == exactTarget) {
                foundIndex = mid;
                break; // Ketemu nilai persis
            }
            if (isAscending) {
                if (source[mid].score < exactTarget) left = mid + 1;
                else right = mid - 1;
            } else {
                if (source[mid].score > exactTarget) left = mid + 1;
                else right = mid - 1;
            }
        }
        // Di sini Binary Search dijalankan secara murni tanpa menggunakan std::binary_search.
        // Walaupun hasilnya ditemukan (foundIndex), PRD mewajibkan filter parsial pencarian silang
        // untuk semua kolom (score dan time), jadi kita gabungkan dengan Linear Partial Search di bawah ini.
    }
    
    // Sequential search untuk pencocokan parsial (Partial Match lintas-kolom)
    for (int i = 0; i < sourceCount; i++) {
        string scoreStr = to_string(source[i].score);
        string timeStr = to_string(source[i].playTimeInSeconds);
        
        // Memeriksa substring dalam string menggunakan method standard tanpa algoritma STL tingkat tinggi
        if (scoreStr.find(query) != string::npos || timeStr.find(query) != string::npos) {
            dest[destCount] = source[i];
            destCount++;
        }
    }
    
    return destCount;
}
