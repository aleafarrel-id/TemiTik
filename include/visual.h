/**
 * @file visual.h
 * @brief Deklarasi antarmuka untuk rendering visual terminal (TUI).
 * 
 * Menyediakan fungsi-fungsi manipulasi layar, pergerakan kursor,
 * serta fungsi menggambar elemen antarmuka pengguna seperti menu,
 * kotak dialog, dan indikator status.
 */

#pragma once

#include "dataStructs.h"

// Utilitas Terminal

/**
 * @brief Mengambil ukuran aktual terminal saat ini (lebar dan tinggi).
 * 
 * @param width Referensi untuk menyimpan lebar terminal.
 * @param height Referensi untuk menyimpan tinggi terminal.
 */
void getTerminalSize(int& width, int& height);

/**
 * @brief Menginisialisasi pengaturan terminal (mode virtual, dll).
 */
void initTerminal();

/**
 * @brief Membersihkan seluruh layar terminal.
 */
void clearScreen();

/**
 * @brief Memindahkan kursor terminal ke koordinat (x, y) tertentu.
 * 
 * @param x Posisi kolom mendatar (1-indexed).
 * @param y Posisi baris menurun (1-indexed).
 */
void moveCursorTo(int x, int y);

/**
 * @brief Menyembunyikan kursor agar tidak mengganggu tampilan TUI.
 */
void hideCursor();

/**
 * @brief Menampilkan kembali kursor terminal.
 */
void showCursor();

// Utilitas Warna (ANSI Escape Codes)

/**
 * @brief Mengubah warna teks pada terminal menggunakan ANSI Escape Codes.
 * 
 * @param colorCode Kode warna ANSI (misal: 31 untuk merah, 32 hijau, dll).
 */
void setColor(int colorCode);

/**
 * @brief Mengembalikan warna teks terminal ke warna default.
 */
void resetColor();

// Utilitas Gambar

/**
 * @brief Menggambar kotak pembatas persegi panjang di layar.
 * 
 * @param x Koordinat x awal (kiri).
 * @param y Koordinat y awal (atas).
 * @param width Lebar kotak.
 * @param height Tinggi kotak.
 */
void drawBox(int x, int y, int width, int height);

/**
 * @brief Menggambar garis lurus horizontal.
 * 
 * @param x Koordinat x awal (kiri).
 * @param y Koordinat y.
 * @param length Panjang garis.
 */
void drawHorizontalLine(int x, int y, int length);

/**
 * @brief Mencetak teks di posisi tengah suatu area.
 * 
 * @param text Teks yang akan dicetak.
 * @param y Koordinat vertikal tempat teks akan dicetak.
 * @param areaWidth Lebar area yang menjadi patokan perhitungan titik tengah.
 */
void printCentered(const std::string& text, int y, int areaWidth);

// Rendering Layar Utama

/**
 * @brief Merender tampilan awal/menu utama permainan.
 */
void renderMainMenu(bool fullRedraw = true);

/**
 * @brief Merender antarmuka utama permainan saat sesi bermain berlangsung.
 * 
 * @param state Pointer ke status terkini pemain (nyawa, skor, dsb).
 */
void renderGameUI(PlayerState* state, bool fullRedraw = true);

/**
 * @brief Merender layar akhir/Game Over setelah pemain kehabisan nyawa.
 * 
 * @param score Skor akhir yang didapatkan.
 * @param timeInSeconds Lama waktu bertahan dalam detik.
 */
void renderEndScreen(int score, int timeInSeconds, bool fullRedraw = true);

/**
 * @brief Merender layar Credit/tentang tim pembuat permainan.
 */
void renderCreditsScreen(bool fullRedraw = true);

// Rendering Layar History

/**
 * @brief Merender tabel daftar riwayat skor dan navigasi halaman.
 * 
 * @param records Array berisi daftar rekaman skor (sudah diurutkan).
 * @param count Total jumlah rekaman yang tersedia.
 * @param state Pointer ke status navigasi riwayat saat ini (kursor, halaman).
 */
void renderHistoryMenu(ScoreRecord* records, int count, HistoryState* state, bool fullRedraw = true);

/**
 * @brief Merender detail statistik spesifik dari satu rekaman yang dipilih.
 * 
 * @param record Pointer ke rekaman skor yang ingin ditampilkan.
 */
void renderHistoryStats(ScoreRecord* record, bool fullRedraw = true);

/**
 * @brief Merender peringatan konfirmasi sebelum menghapus seluruh data riwayat.
 */
void renderClearHistoryConfirmation(bool fullRedraw = true);

/**
 * @brief Merender layar jeda (Pause) sebagai overlay di atas tampilan game aktif.
 *
 * Menampilkan kotak dialog di tengah layar berisi pilihan:
 * ENTER untuk melanjutkan permainan, Q untuk keluar ke Menu utama.
 * Fungsi ini tidak menghapus layar secara keseluruhan agar konteks game
 * di belakangnya tetap terlihat sebagai latar.
 *
 * @param fullRedraw Jika true, gambar ulang seluruh overlay; jika false, tidak melakukan apa-apa.
 */
void renderPauseScreen(bool fullRedraw = true);

/**
 * @brief Merender tampilan ketika data riwayat kosong.
 */
void renderEmptyHistory();
