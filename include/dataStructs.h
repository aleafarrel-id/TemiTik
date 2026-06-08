/**
 * @file dataStructs.h
 * @brief Deklarasi struktur data dan enumerasi utama permainan.
 * 
 * File ini berisi definisi seluruh tipe data bentukan (struct) dan enumerasi (enum)
 * yang digunakan dalam permainan, seperti representasi entitas, skor, dan status pemain.
 * Hanya berisi struktur data, tanpa implementasi logika.
 */

#pragma once

// Alasan penggunaan <string>: Diperlukan untuk menyimpan teks kata secara dinamis tanpa buffer overflow manual (aman dari buffer overrun pada TUI).
#include <string>

// Konstanta Konfigurasi Global
// Layout & Batas Minimum
constexpr int MIN_SCREEN_WIDTH = 80;
constexpr int MIN_SCREEN_HEIGHT = 24;

// Gameplay
constexpr int STARTING_HEALTH = 3;
constexpr int POINTS_PER_WORD = 10;
constexpr int MAX_ACTIVE_WORDS = 5;
constexpr int INITIAL_DROP_SPEED = 1;
constexpr int SPEED_INCREMENT = 1;

// Data
constexpr int WORD_BANK_CAPACITY = 1000;
constexpr int MAX_HISTORY_RECORDS = 100;
constexpr int MAX_RECORDS_PER_PAGE = 5;

// Konstanta Tombol (Keyboard)
constexpr int KEY_ESC = 27;
constexpr int KEY_ENTER_WIN = '\r';
constexpr int KEY_ENTER_NIX = '\n';
constexpr int KEY_ARROW_PREFIX1 = 224;
constexpr int KEY_ARROW_PREFIX2 = 0;
constexpr int KEY_UP = 72;
constexpr int KEY_DOWN = 80;

/**
 * @brief Enumerasi Status Permainan (Game State).
 * Menentukan layar atau mode permainan yang sedang aktif.
 */
enum GameState {
    Menu = 0,
    Play = 1,
    End = 2,
    History = 3,
    HistoryStats = 4,
    Credits = 5,
    ClearHistoryConfirmation = 6
};

// Struktur Data Inti

/**
 * @brief Data satu kata yang sedang aktif/jatuh di layar.
 */
struct WordItem {
    std::string text;  // Ukuran besar (string)
    int xPosition;     // Posisi horizontal di layar
    int yPosition;     // Posisi vertikal di layar
    bool isActive;     // Status apakah kata masih aktif
};

/**
 * @brief Rekaman skor pemain untuk disimpan di riwayat.
 */
struct ScoreRecord {
    int score;             // Total skor akhir
    int playTimeInSeconds; // Total waktu bermain dalam detik
};

/**
 * @brief Status terkini dari pemain yang sedang bermain.
 */
struct PlayerState {
    std::string currentInput; // Input ketikan pemain saat ini
    int currentHealth;        // Sisa nyawa
    int currentScore;         // Skor sementara
    int levelSpeed;           // Kecepatan jatuh kata saat ini
};

// Queue Node (Linked List Manual)
// Struct Packing dipertimbangkan untuk efisiensi memori (Pointer didahulukan).

/**
 * @brief Simpul (node) untuk struktur data Queue yang berisi WordItem.
 * Diurutkan berdasarkan ukuran data untuk optimalisasi memori (Struct Packing).
 */
struct QueueNode {
    QueueNode* next;   // Pointer ke elemen selanjutnya
    WordItem data;     // Data kata (ukuran besar)
};

// Manual Queue
// Struct Packing: Pointer 'front' dan 'rear' (8 bytes di 64-bit) diletakkan sebelum tipe primitif 'int' (4 bytes).

/**
 * @brief Antrean (Queue) berbasis Linked List untuk menyimpan kata.
 * Diurutkan berdasarkan ukuran data (Struct Packing).
 */
struct Queue {
    QueueNode* front;  // Pointer ke elemen paling depan
    QueueNode* rear;   // Pointer ke elemen paling belakang
    int count;         // Jumlah elemen dalam antrean
};

// Status Navigasi History
// Struct Packing: Objek string didahulukan, diikuti integer, dan diakhiri boolean untuk meminimalkan padding byte.

/**
 * @brief Status dan filter saat sedang berada di menu riwayat.
 * Diurutkan berdasarkan ukuran data (Struct Packing).
 */
struct HistoryState {
    std::string searchQuery; // Kata kunci pencarian
    int cursorIndex;         // Posisi kursor baris terpilih
    int currentPage;         // Halaman saat ini
    int searchResultIndex;   // Indeks hasil pencarian aktif
    bool isAscending;        // Mode urutan (ASC/DESC)
    bool isSearchActive;     // Apakah input pencarian sedang aktif
};
