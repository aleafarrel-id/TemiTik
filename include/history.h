/**
 * @file history.h
 * @brief Deklarasi antarmuka untuk persistensi dan manipulasi rekaman skor permainan.
 *
 * Menyediakan kontrak fungsi untuk operasi I/O berkas riwayat (simpan, muat, hapus)
 * serta operasi manipulasi data (pengurutan Bubble Sort dan pencarian Binary Search + Partial Match)
 * yang semuanya diimplementasikan secara manual tanpa algoritma STL.
 */

#pragma once

#include "dataStructs.h" ///< Menyediakan definisi struct ScoreRecord dan konstanta MAX_HISTORY_RECORDS.
#include <string>        ///< Diperlukan untuk tipe parameter std::string pada filterHistoryRecords.

/**
 * @brief Menyimpan satu rekaman skor ke akhir berkas historyData.txt dalam mode append.
 *
 * Format baris yang ditulis: "<score> <playTimeInSeconds>\n".
 * Jika jalur utama tidak dapat dibuka, fungsi mencoba dua jalur fallback
 * relatif terhadap direktori eksekusi.
 *
 * @param newRecord Rekaman skor yang akan ditambahkan; nilai score dan playTimeInSeconds
 *                  ditulis sebagai dua integer terpisah spasi dalam satu baris.
 */
void saveRecordToFile(ScoreRecord newRecord);

/**
 * @brief Memuat seluruh rekaman skor dari berkas historyData.txt ke dalam array.
 *
 * Membaca baris demi baris; melewati baris kosong dan baris komentar (diawali "//").
 * Setiap baris valid di-parsing secara manual menggunakan posisi spasi pertama
 * sebagai delimiter antar field score dan playTimeInSeconds.
 * Pembacaan berhenti saat array penuh (MAX_HISTORY_RECORDS) atau berkas habis dibaca.
 *
 * @param records Pointer ke array ScoreRecord dengan kapasitas minimal MAX_HISTORY_RECORDS
 *                sebagai tujuan penyimpanan rekaman yang dimuat.
 * @return int    Jumlah rekaman yang berhasil dimuat; 0 jika berkas tidak dapat dibuka
 *                pada semua jalur yang dicoba.
 */
int loadHistoryRecords(ScoreRecord* records);

/**
 * @brief Menghapus seluruh isi berkas historyData.txt secara permanen menggunakan ios::trunc.
 *
 * Tidak menerima parameter dan tidak mengembalikan nilai; operasi bersifat destruktif
 * dan tidak dapat dibatalkan. Fallback jalur diterapkan identik dengan fungsi I/O lainnya.
 */
void clearAllHistoryRecords();

/**
 * @brief Mengurutkan array rekaman skor secara menaik (ascending) berdasarkan field score
 *        menggunakan algoritma Bubble Sort manual tanpa std::sort.
 *
 * Modifikasi dilakukan in-place langsung pada array yang ditunjuk oleh @p records.
 * Jangkauan inner loop menyusut setiap pass karena elemen terbesar sudah berada
 * di posisi akhir yang benar.
 *
 * @param records Pointer ke array ScoreRecord yang akan diurutkan secara in-place.
 * @param count   Jumlah elemen aktif dalam array.
 */
void sortRecordsAscending(ScoreRecord* records, int count);

/**
 * @brief Mengurutkan array rekaman skor secara menurun (descending) berdasarkan field score
 *        menggunakan algoritma Bubble Sort manual tanpa std::sort.
 *
 * Identik dengan sortRecordsAscending kecuali arah operator komparasi dibalik
 * sehingga elemen terkecil bergerak ke posisi akhir setiap pass.
 *
 * @param records Pointer ke array ScoreRecord yang akan diurutkan secara in-place.
 * @param count   Jumlah elemen aktif dalam array.
 */
void sortRecordsDescending(ScoreRecord* records, int count);

/**
 * @brief Memfilter rekaman dari @p source ke @p dest menggunakan Binary Search eksak
 *        diikuti Sequential Partial Match pada semua field numerik.
 *
 * Kontrak numerik: @p query wajib berisi digit 0-9 saja; fungsi memvalidasi syarat ini
 * secara defensif dan mengembalikan 0 jika dilanggar. String kosong melewati filter
 * dan mengembalikan salinan seluruh data sumber.
 *
 * Algoritma dua tahap:
 * (1) Binary Search manual pada kolom score untuk mengkonfirmasi keberadaan nilai eksak;
 *     arah navigasi menyesuaikan @p isAscending.
 * (2) Sequential Partial Match manual: @p query dicocokkan sebagai substring pada
 *     kolom score dan TIGA representasi waktu agar konsisten dengan format tampilan "Xm Ys":
 *       - Raw detik total (misal "150").
 *       - Komponen menit (misal "2" dari 150/60 = 2).
 *       - Komponen detik (misal "30" dari 150%60 = 30).
 *
 * @param source       Pointer ke array sumber yang sudah diurutkan oleh sortRecordsAscending
 *                     atau sortRecordsDescending sebelum pemanggilan fungsi ini.
 * @param sourceCount  Jumlah elemen aktif dalam array sumber.
 * @param dest         Pointer ke array tujuan dengan kapasitas minimal @p sourceCount
 *                     untuk menampung hasil filter.
 * @param query        String pencarian; harus berisi digit 0-9 saja; string kosong
 *                     mengembalikan seluruh data tanpa filter.
 * @param isAscending  true jika @p source diurutkan ascending; false jika descending.
 *                     Parameter ini menentukan arah navigasi Binary Search pada tahap pertama.
 * @return int         Jumlah rekaman yang lolos filter dan disalin ke @p dest;
 *                     0 jika @p query tidak numerik atau tidak ada kecocokan ditemukan.
 */
int filterHistoryRecords(ScoreRecord* source, int sourceCount, ScoreRecord* dest, std::string query, bool isAscending);
