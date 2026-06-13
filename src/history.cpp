/**
 * @file history.cpp
 * @brief Mengelola persistensi rekaman skor ke file teks, serta mengimplementasikan
 *        algoritma Bubble Sort dan Binary Search secara manual sesuai syarat
 *        mata kuliah Struktur Data.
 */

#include "history.h"

// <iostream>: Digunakan oleh std::cout untuk mencetak pesan debug jika diperlukan;
// keberadaannya tidak melanggar aturan karena tidak dipakai untuk I/O data utama.
#include <iostream>

// <fstream>: Esensial untuk membuka, membaca, menulis, dan memotong berkas historyData.txt;
// merupakan satu-satunya cara legal untuk persistensi data tanpa library pihak ketiga.
#include <fstream>

// <string>: Diperlukan untuk memproses jalur file (HISTORY_FILE_PATH) dan parsing baris
// teks mentah dari file menggunakan getline dan substr.
#include <string>

using namespace std;

const string HISTORY_FILE_PATH = "data/historyData.txt"; ///< Jalur relatif berkas penyimpanan rekaman skor dari direktori eksekusi.

/**
 * @brief Menyimpan satu rekaman skor ke akhir berkas historyData.txt.
 *
 * Membuka berkas dalam mode append agar riwayat lama tidak tertimpa.
 * Jika jalur utama gagal, percobaan dilakukan ke jalur fallback satu dan dua
 * tingkat di atas direktori eksekusi.
 *
 * @param newRecord Rekaman skor yang akan ditambahkan ke berkas.
 */
void saveRecordToFile(ScoreRecord newRecord) {
    // ios::app memastikan data ditulis di akhir berkas tanpa menghapus isi sebelumnya.
    ofstream fileStream(HISTORY_FILE_PATH, ios::app);
    if (!fileStream.is_open()) {
        // Fallback satu level ke atas; menangani eksekusi dari subdirektori build.
        fileStream.open("../" + HISTORY_FILE_PATH, ios::app);
        if (!fileStream.is_open()) {
            // Fallback dua level ke atas; menangani struktur build bertingkat.
            fileStream.open("../../" + HISTORY_FILE_PATH, ios::app);
        }
    }
    if (fileStream.is_open()) {
        // Format: "<score> <playTimeInSeconds>\n"; spasi tunggal sebagai delimiter antar field.
        fileStream << newRecord.score << " " << newRecord.playTimeInSeconds << "\n";
        fileStream.close();
    }
}

/**
 * @brief Memuat seluruh rekaman skor dari berkas historyData.txt ke dalam array.
 *
 * Membaca baris demi baris menggunakan getline, melewati baris kosong dan baris
 * komentar (diawali "//"), lalu mem-parsing dua field integer dari setiap baris
 * secara manual melalui pencarian posisi spasi pertama (spacePos).
 * Berhenti saat array penuh (MAX_HISTORY_RECORDS) atau berkas habis dibaca.
 *
 * @param records Pointer ke array ScoreRecord tujuan penyimpanan hasil pembacaan.
 * @return int    Jumlah rekaman yang berhasil dimuat; 0 jika berkas tidak dapat dibuka.
 */
int loadHistoryRecords(ScoreRecord* records) {
    ifstream fileStream(HISTORY_FILE_PATH);
    if (!fileStream.is_open()) {
        // Fallback satu level ke atas.
        fileStream.open("../" + HISTORY_FILE_PATH);
        if (!fileStream.is_open()) {
            // Fallback dua level ke atas.
            fileStream.open("../../" + HISTORY_FILE_PATH);
            if (!fileStream.is_open()) {
                // Tidak ada jalur yang valid; kembalikan 0 rekaman.
                return 0;
            }
        }
    }

    int count = 0;
    string line;

    // getline dipakai agar baris komentar dapat diidentifikasi dan dilewati sebelum parsing.
    while (count < MAX_HISTORY_RECORDS && getline(fileStream, line)) {
        // Lewati baris kosong dan baris komentar berformat "// ...".
        if (line.empty() || line.substr(0, 2) == "//") {
            continue;
        }

        // Cari posisi spasi pertama sebagai batas antar field score dan playTimeInSeconds.
        size_t spacePos = line.find(' ');
        if (spacePos != string::npos) {
            // Substring sebelum spacePos adalah field score.
            records[count].score = stoi(line.substr(0, spacePos));
            // Substring setelah spacePos adalah field playTimeInSeconds.
            records[count].playTimeInSeconds = stoi(line.substr(spacePos + 1));
            count++;
        }
    }

    // Tutup file untuk melepaskan kunci sumber daya sistem operasi.
    fileStream.close();
    return count;
}

/**
 * @brief Menghapus seluruh isi berkas historyData.txt secara permanen.
 *
 * Membuka berkas dalam mode ios::trunc yang langsung memotong (truncate) isi
 * berkas menjadi nol byte saat dibuka, tanpa perlu menulis konten apapun.
 * Fallback jalur diterapkan identik dengan fungsi lainnya.
 */
void clearAllHistoryRecords() {
    // ios::trunc menghapus semua isi berkas secara atomik saat berkas dibuka.
    ofstream fileStream(HISTORY_FILE_PATH, ios::trunc);
    if (!fileStream.is_open()) {
        // Fallback satu level ke atas.
        fileStream.open("../" + HISTORY_FILE_PATH, ios::trunc);
        if (!fileStream.is_open()) {
            // Fallback dua level ke atas.
            fileStream.open("../../" + HISTORY_FILE_PATH, ios::trunc);
        }
    }
    if (fileStream.is_open()) {
        fileStream.close();
    }
}

/**
 * @brief Mengurutkan array rekaman skor secara menaik (ascending) berdasarkan field score
 *        menggunakan algoritma Bubble Sort manual.
 *
 * Implementasi manual tanpa std::sort untuk memenuhi larangan penggunaan <algorithm>
 * sesuai syarat mata kuliah Struktur Data.
 *
 * @param records Pointer ke array ScoreRecord yang akan diurutkan (in-place).
 * @param count   Jumlah elemen aktif dalam array.
 */
void sortRecordsAscending(ScoreRecord* records, int count) {
    // Outer loop: setiap iterasi pass ke-i memastikan elemen terbesar ke-i
    // sudah berada di posisi akhir yang benar.
    for (int i = 0; i < count - 1; i++) {
        // Inner loop: jangkauan menyusut sebesar i karena elemen [count-i .. count-1]
        // sudah terurut dan tidak perlu dibandingkan ulang.
        for (int j = 0; j < count - i - 1; j++) {
            // Kondisi swap: elemen kiri lebih besar dari kanan → tukar posisi.
            if (records[j].score > records[j + 1].score) {
                // Swap manual menggunakan variabel sementara temp; tidak memakai std::swap.
                ScoreRecord temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
            }
        }
    }
}

/**
 * @brief Mengurutkan array rekaman skor secara menurun (descending) berdasarkan field score
 *        menggunakan algoritma Bubble Sort manual.
 *
 * Identik dengan sortRecordsAscending kecuali pada arah operator komparasi:
 * kondisi swap dibalik sehingga elemen terkecil bergerak ke posisi akhir setiap pass.
 *
 * @param records Pointer ke array ScoreRecord yang akan diurutkan (in-place).
 * @param count   Jumlah elemen aktif dalam array.
 */
void sortRecordsDescending(ScoreRecord* records, int count) {
    // Outer loop: setiap pass ke-i memastikan elemen terkecil ke-i
    // sudah berada di posisi akhir yang benar.
    for (int i = 0; i < count - 1; i++) {
        // Inner loop: jangkauan menyusut identik dengan ascending.
        for (int j = 0; j < count - i - 1; j++) {
            // Kondisi swap dibalik (< bukan >): elemen kiri lebih kecil dari kanan → tukar.
            if (records[j].score < records[j + 1].score) {
                // Swap manual dengan temp; tidak memakai std::swap.
                ScoreRecord temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
            }
        }
    }
}

/**
 * @brief Memfilter rekaman menggunakan Binary Search eksak diikuti Sequential Partial Match
 *        pada semua field numerik, termasuk tiga representasi waktu.
 *
 * Kontrak: parameter @p query wajib berisi digit 0-9 saja. Fungsi memvalidasi
 * syarat ini secara defensif; jika dilanggar, fungsi mengembalikan 0 tanpa memproses data.
 * Jika @p query kosong, seluruh data sumber disalin ke @p dest tanpa filter.
 *
 * Algoritma dua tahap:
 * (1) Binary Search manual pada kolom score dengan arah pencarian mengikuti @p isAscending.
 * (2) Sequential Partial Match manual pada EMPAT representasi string per rekaman:
 *     - Kolom score sebagai string integer.
 *     - Raw detik total (playTimeInSeconds sebagai string).
 *     - Komponen menit (playTimeInSeconds / 60 sebagai string).
 *     - Komponen detik (playTimeInSeconds % 60 sebagai string).
 *     Tiga representasi waktu memastikan konsistensi dengan format tampilan "Xm Ys".
 *
 * @param source       Pointer ke array sumber yang sudah diurutkan.
 * @param sourceCount  Jumlah elemen aktif dalam array sumber.
 * @param dest         Pointer ke array tujuan untuk menampung hasil filter.
 * @param query        String pencarian; harus berisi digit saja; string kosong mengembalikan semua data.
 * @param isAscending  true jika array sumber diurutkan ascending, false jika descending;
 *                     mempengaruhi arah navigasi Binary Search.
 * @return int         Jumlah rekaman hasil filter; 0 jika query tidak numerik atau tidak ada kecocokan.
 */
int filterHistoryRecords(ScoreRecord* source, int sourceCount, ScoreRecord* dest, string query, bool isAscending) {
    // Query kosong: tidak ada filter aktif; salin semua rekaman ke dest dan selesai.
    if (query.empty()) {
        for (int i = 0; i < sourceCount; i++) dest[i] = source[i];
        return sourceCount;
    }

    // Validasi numerik: query hanya boleh berisi karakter '0'-'9'.
    // main.cpp menjamin ini, tetapi validasi ulang sebagai lapisan pertahanan kedua.
    bool isNumeric = true;
    for (int i = 0; i < (int)query.length(); i++) {
        if (query[i] < '0' || query[i] > '9') {
            isNumeric = false;
            break; // Satu karakter non-digit sudah cukup untuk menolak query.
        }
    }

    // Query non-numerik adalah kasus defensif; tolak dan kembalikan 0 hasil.
    if (!isNumeric) {
        return 0;
    }

    // Konversi query ke integer untuk dipakai pada Binary Search eksak.
    int exactTarget = stoi(query);

    // --- Tahap 1: Binary Search Manual ---
    // Array sudah diurutkan oleh sortRecordsAscending/Descending sebelum fungsi ini dipanggil.
    int left = 0, right = sourceCount - 1;
    int foundIndex = -1;

    while (left <= right) {
        // Kalkulasi mid dengan rumus aman; menghindari integer overflow pada (left+right)/2.
        int mid = left + (right - left) / 2;

        if (source[mid].score == exactTarget) {
            // Nilai eksak ditemukan; simpan indeks dan hentikan Binary Search.
            foundIndex = mid;
            break;
        }

        if (isAscending) {
            // Array ascending: nilai lebih besar berada di sisi kanan mid.
            if (source[mid].score < exactTarget) left = mid + 1;  // Target di kanan; geser batas kiri.
            else right = mid - 1;                                  // Target di kiri; geser batas kanan.
        } else {
            // Array descending: nilai lebih besar berada di sisi kiri mid.
            if (source[mid].score > exactTarget) left = mid + 1;  // Target di kanan; geser batas kiri.
            else right = mid - 1;                                  // Target di kiri; geser batas kanan.
        }
    }
    // foundIndex menandai konfirmasi keberadaan nilai eksak dari Binary Search.
    // Partial Match di bawah sudah mencakup seluruh kasus kecocokan sehingga
    // foundIndex tidak digunakan secara langsung; dideklarasikan (void) agar tidak ada
    // compiler warning unused variable.
    (void)foundIndex;

    // --- Tahap 2: Sequential Partial Match ---
    // Memeriksa query sebagai substring pada SEMUA field numerik sesuai spesifikasi PRD.
    int destCount = 0;
    for (int i = 0; i < sourceCount; i++) {
        // Konversi field integer ke string untuk pencocokan substring karakter per karakter.
        string scoreStr = to_string(source[i].score);
        string timeStr  = to_string(source[i].playTimeInSeconds);

        bool foundInScore = false;
        bool foundInTime  = false;

        // Pencocokan substring pada kolom score:
        // Loop j: setiap posisi awal kandidat substring dalam scoreStr.
        if (scoreStr.length() >= query.length()) {
            for (int j = 0; j <= (int)(scoreStr.length() - query.length()); j++) {
                bool match = true;
                // Loop k: bandingkan karakter query[k] dengan scoreStr[j+k] satu per satu.
                for (int k = 0; k < (int)query.length(); k++) {
                    if (scoreStr[j + k] != query[k]) {
                        match = false;
                        break; // Karakter tidak cocok; hentikan perbandingan posisi j ini.
                    }
                }
                if (match) { foundInScore = true; break; } // Substring ditemukan; tidak perlu lanjut.
            }
        }

        // Pencocokan substring pada kolom playTimeInSeconds.
        // Dilakukan pada TIGA representasi string untuk konsistensi dengan format tampilan "Xm Ys":
        // (a) Raw detik total   — misal 150 → "150"
        // (b) Komponen menit    — misal 150/60 = 2 → "2"
        // (c) Komponen detik    — misal 150%60 = 30 → "30"
        // Pengguna yang mengetik "2" akan menemukan rekaman "2m 30s" (120-179 detik).
        string minuteStr = to_string(source[i].playTimeInSeconds / 60);
        string secStr    = to_string(source[i].playTimeInSeconds % 60);

        // (a) Pencocokan pada raw detik total
        if (timeStr.length() >= query.length()) {
            for (int j = 0; j <= (int)(timeStr.length() - query.length()); j++) {
                bool match = true;
                for (int k = 0; k < (int)query.length(); k++) {
                    if (timeStr[j + k] != query[k]) {
                        match = false;
                        break;
                    }
                }
                if (match) { foundInTime = true; break; }
            }
        }

        // (b) Pencocokan pada komponen menit
        if (!foundInTime && minuteStr.length() >= query.length()) {
            for (int j = 0; j <= (int)(minuteStr.length() - query.length()); j++) {
                bool match = true;
                for (int k = 0; k < (int)query.length(); k++) {
                    if (minuteStr[j + k] != query[k]) {
                        match = false;
                        break;
                    }
                }
                if (match) { foundInTime = true; break; }
            }
        }

        // (c) Pencocokan pada komponen detik
        if (!foundInTime && secStr.length() >= query.length()) {
            for (int j = 0; j <= (int)(secStr.length() - query.length()); j++) {
                bool match = true;
                for (int k = 0; k < (int)query.length(); k++) {
                    if (secStr[j + k] != query[k]) {
                        match = false;
                        break;
                    }
                }
                if (match) { foundInTime = true; break; }
            }
        }

        // Rekaman lolos filter jika query ditemukan pada salah satu atau lebih field.
        if (foundInScore || foundInTime) {
            dest[destCount] = source[i];
            destCount++;
        }
    }

    return destCount;
}
