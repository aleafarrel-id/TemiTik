/**
 * @file main.cpp
 * @brief Entry point utama aplikasi permainan.
 * 
 * Menginisialisasi modul inti permainan dan menjalankan game loop utama.
 * Memanfaatkan pola arsitektur state machine untuk mengelola siklus hidup 
 * sesi aplikasi mulai dari navigasi menu hingga akhir permainan.
 */

// Alasan penggunaan <iostream>: Diperlukan secara fundamental oleh komponen cout/cin untuk penulisan ke buffer stdout terminal.
#include <iostream>

// Alasan penggunaan <conio.h>: Menyediakan fungsi _getch() untuk menangkap input non-blocking (asinkron) secara instan tanpa menunggu ENTER. Ini adalah komponen wajib untuk navigasi menu dan pergerakan real-time pada Windows.
#include <conio.h>

// Alasan penggunaan <windows.h>: Menyediakan API interaksi sistem operasi Windows level rendah (GetConsoleScreenBufferInfo, FillConsoleOutputCharacter, Sleep) yang wajib untuk merender TUI dan memanipulasi buffer terminal. Tidak melanggar aturan logika prosedural manual.
#include <windows.h>

#include "dataStructs.h"
#include "visual.h"
#include "loader.h"
#include "gameEngine.h"
#include "history.h"

using namespace std;

/**
 * @brief Menunggu input pengguna secara asinkron sambil memantau perubahan ukuran terminal.
 * Jika ukuran terminal berubah, fungsi ini akan mengembalikan 0 untuk memicu render ulang.
 * 
 * @param currentWidth Referensi ke lebar terminal saat ini yang tersimpan.
 * @param currentHeight Referensi ke tinggi terminal saat ini yang tersimpan.
 * @return int Karakter yang ditekan, atau 0 jika terminal di-resize.
 */
int getAsyncInputOrResize(int& currentWidth, int& currentHeight) {
    while (true) {
        if (_kbhit()) {
            return _getch();
        }
        
        int checkWidth, checkHeight;
        getTerminalSize(checkWidth, checkHeight);
        if (checkWidth != currentWidth || checkHeight != currentHeight) {
            return 0; // Sinyal bahwa terminal diubah ukurannya
        }
        Sleep(ASYNC_INPUT_SLEEP_MS); // Tidur singkat untuk efisiensi CPU tanpa memblokir
    }
}

int main() {
    // Inisialisasi Terminal
    // Mengatur terminal untuk mendukung karakter ANSI dan menyembunyikan kursor bawaan.
    initTerminal();
    
    // Status Program utama yang mengontrol perulangan aplikasi.
    bool isRunning = true;
    
    // Variabel state machine utama. Menentukan layar mana yang sedang aktif saat ini.
    // Dimulai dari layar Menu (layar awal).
    GameState currentState = Menu;
    GameState previousState = (GameState)-1;
    
    // Struktur Data Penyimpanan Internal
    // Inisialisasi state awal secara terpusat untuk keseluruhan aplikasi.
    
    // Menyimpan status pemain saat ini (nyawa awal 3, skor 0, kecepatan awal 1).
    PlayerState playerState = {"", STARTING_HEALTH, 0, INITIAL_DROP_SPEED};
    
    // Array statis untuk menampung daftar riwayat permainan yang sudah diurutkan atau akan diurutkan.
    // Kapasitas maksimum didefinisikan oleh MAX_HISTORY_RECORDS (100).
    ScoreRecord historyRecords[MAX_HISTORY_RECORDS];
    int recordCount = loadHistoryRecords(historyRecords);
    
    // Array statis penampung hasil pemfilteran pencarian (Search)
    ScoreRecord filteredRecords[MAX_HISTORY_RECORDS];
    int filteredCount = recordCount;
    
    // Waktu bermain sesi ini
    int currentSessionTime = 0;
    
    // Menyimpan status interaksi di dalam menu riwayat (pencarian, halaman aktif, posisi kursor).
    HistoryState historyState = {"", 0, 0, -1, true, false};
    
    // Inisialisasi antrean kata untuk permainan
    Queue wordQueue;
    wordQueue.front = nullptr;
    wordQueue.rear = nullptr;
    wordQueue.count = 0;
    
    // Variabel pelacak ukuran terminal untuk mendeteksi resize
    int currentTerminalWidth, currentTerminalHeight;
    getTerminalSize(currentTerminalWidth, currentTerminalHeight);
    
    // Game Loop Utama
    // Menangani rendering dan input berdasarkan state aktif aplikasi.
    while (isRunning) {
        // Deteksi perubahan ukuran terminal
        int newTerminalWidth, newTerminalHeight;
        getTerminalSize(newTerminalWidth, newTerminalHeight);
        bool terminalResized = false;
        
        if (newTerminalWidth != currentTerminalWidth || newTerminalHeight != currentTerminalHeight) {
            currentTerminalWidth = newTerminalWidth;
            currentTerminalHeight = newTerminalHeight;
            terminalResized = true;
        }

        bool stateChanged = (currentState != previousState) || terminalResized;
        previousState = currentState;
        
        // Menggunakan switch-case untuk menangani aksi berdasarkan layar yang sedang aktif.
        switch (currentState) {
            
            // =========================================================
            // STATE: LAYAR MENU UTAMA
            // =========================================================
            case Menu: {
                // Merender (menggambar) bingkai dan menu utama ke terminal.
                renderMainMenu(stateChanged);
                
                // Menunggu dan menangkap input secara asinkron (mendukung resize terminal).
                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue; // Kembali ke awal loop untuk merender ulang
                
                // Navigasi State:
                if (ch == KEY_ENTER_WIN || ch == KEY_ENTER_NIX) {
                    // Jika pengguna menekan ENTER, muat kata dan pindah ke sesi permainan.
                    loadWordsFromFile("data/wordBank.txt", &wordQueue);
                    
                    // Reset status pemain sebelum bermain
                    playerState.currentHealth = STARTING_HEALTH;
                    playerState.currentScore = 0;
                    playerState.levelSpeed = INITIAL_DROP_SPEED;
                    playerState.currentInput = "";
                    
                    currentState = Play;
                } else if (ch == 'h' || ch == 'H') {
                    // Jika pengguna menekan H, buka layar Riwayat (History).
                    currentState = History;
                } else if (ch == 'q' || ch == 'Q') {
                    // Jika pengguna menekan Q, keluar dari loop (menghentikan program).
                    isRunning = false;
                }
                break;
            }
            
            // =========================================================
            // STATE: SESI PERMAINAN (GAME PLAY)
            // =========================================================
            case Play: {
                ULONGLONG sessionStartTime = GetTickCount64();
                
                // Delegasi seluruh logika dan rendering in-game ke gameEngine
                runGameLoop(&playerState, currentState, &wordQueue);
                
                // Menghitung durasi sesi permainan dalam satuan detik
                currentSessionTime = (GetTickCount64() - sessionStartTime) / MS_PER_SECOND;
                
                // Setelah game loop selesai, currentState mungkin berubah (ke End atau Menu)
                // Bersihkan memori queue yang tersisa agar tidak memory leak
                while (wordQueue.front != nullptr) {
                    QueueNode* temp = wordQueue.front;
                    wordQueue.front = wordQueue.front->next;
                    delete temp;
                }
                wordQueue.rear = nullptr;
                wordQueue.count = 0;
                break;
            }
            
            // =========================================================
            // STATE: LAYAR AKHIR (GAME OVER)
            // =========================================================
            case End: {
                // Merender statistik akhir pemain dengan mengirim state skor terkini dan durasi.
                renderEndScreen(playerState.currentScore, currentSessionTime, stateChanged); 
                
                // Menangkap input dari keyboard
                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;
                
                if (ch == KEY_ENTER_WIN || ch == KEY_ENTER_NIX) {
                    // Simpan history skor & waktu secara otomatis ke file
                    ScoreRecord newRecord = {playerState.currentScore, currentSessionTime};
                    saveRecordToFile(newRecord);
                    
                    // Memuat ulang data dari file ke dalam memori
                    recordCount = loadHistoryRecords(historyRecords);
                    
                    // Kembali ke Menu utama
                    currentState = Menu;
                } else if (ch == 'c' || ch == 'C') {
                    // Menekan C akan membuka layar Kredit (Daftar Tim).
                    currentState = Credits;
                }
                break;
            }
            
            // =========================================================
            // STATE: LAYAR DAFTAR RIWAYAT (HISTORY MENU)
            // =========================================================
            case History: {
                // Pengurutan (Sorting) Array Dasar
                if (historyState.isAscending) {
                    sortRecordsAscending(historyRecords, recordCount);
                } else {
                    sortRecordsDescending(historyRecords, recordCount);
                }
                
                // Pemfilteran & Pencarian Biner (Searching)
                filteredCount = filterHistoryRecords(historyRecords, recordCount, filteredRecords, historyState.searchQuery, historyState.isAscending);
                
                // Logika Pagination dan Kursor:
                // Jika tidak ada data, paksa posisi kursor ke index 0.
                if (filteredCount == 0) {
                    historyState.cursorIndex = 0;
                } else {
                    // Memastikan kursor navigasi tidak keluar dari batas data yang tersedia 
                    // pada halaman yang sedang ditampilkan.
                    int startIdx = historyState.currentPage * MAX_RECORDS_PER_PAGE;
                    int endIdx = startIdx + MAX_RECORDS_PER_PAGE - 1;
                    if (endIdx >= filteredCount) endIdx = filteredCount - 1;
                    
                    if (historyState.cursorIndex < startIdx) historyState.cursorIndex = startIdx;
                    if (historyState.cursorIndex > endIdx) historyState.cursorIndex = endIdx;
                }
                
                // Menggambar antarmuka tabel History. Jika kosong, akan merender peringatan "No history available".
                renderHistoryMenu(filteredRecords, filteredCount, &historyState, stateChanged);
                
                // Menangkap input keyboard
                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;
                
                if (historyState.isSearchActive) {
                    if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
                        historyState.isSearchActive = false;
                    } else if (ch == '\b') {
                        if (!historyState.searchQuery.empty()) {
                            historyState.searchQuery.pop_back();
                        }
                    } else if (isalnum(ch) || isspace(ch)) {
                        historyState.searchQuery += (char)ch;
                    }
                } else {
                    if (ch == KEY_ESC) { 
                        // Menekan ESC mengembalikan ke Menu utama.
                        currentState = Menu;
                    } else if (ch == 's' || ch == 'S') {
                        historyState.isSearchActive = true;
                    } else if (ch == KEY_ENTER_WIN || ch == KEY_ENTER_NIX) {
                        // Menekan ENTER akan masuk ke layar Detail (Stats) dari baris riwayat yang disorot, 
                        // tapi hanya jika ada data riwayat yang tersimpan.
                        if (recordCount > 0) currentState = HistoryStats;
                    } else if (ch == 'c' || ch == 'C') {
                        // Masuk ke dialog konfirmasi penghapusan seluruh data riwayat.
                        currentState = ClearHistoryConfirmation;
                    } else if (ch == 'a' || ch == 'A') {
                        // Mengubah mode pengurutan tabel menjadi menaik (Ascending).
                        historyState.isAscending = true;
                    } else if (ch == 'd' || ch == 'D') {
                        // Mengubah mode pengurutan tabel menjadi menurun (Descending).
                        historyState.isAscending = false;
                    } else if (ch == 'p' || ch == 'P') {
                        // Pindah ke halaman tabel sebelumnya (Prev Page).
                        if (historyState.currentPage > 0) historyState.currentPage--;
                    } else if (ch == 'n' || ch == 'N') {
                        // Pindah ke halaman tabel selanjutnya (Next Page).
                        int maxPage = (filteredCount > 0) ? (filteredCount - 1) / MAX_RECORDS_PER_PAGE : 0;
                        if (historyState.currentPage < maxPage) historyState.currentPage++;
                    } else if (ch == KEY_ARROW_PREFIX1 || ch == KEY_ARROW_PREFIX2) { 
                        // Tombol panah pada keyboard akan memancarkan dua kode.
                        // Jika kode pertama (prefix) tertangkap, kita perlu menangkap kode kedua untuk mengetahui arah panah.
                        ch = _getch();
                        
                        // Kalkulasi batas awal dan batas akhir index baris pada halaman saat ini.
                        int startIdx = historyState.currentPage * MAX_RECORDS_PER_PAGE;
                        int endIdx = startIdx + MAX_RECORDS_PER_PAGE - 1;
                        if (endIdx >= filteredCount) endIdx = filteredCount - 1;
                        
                        if (ch == KEY_UP) { 
                            // Jika panah atas ditekan, gerakkan kursor naik satu baris (tidak boleh kurang dari baris paling atas).
                            if (historyState.cursorIndex > startIdx) historyState.cursorIndex--;
                        } else if (ch == KEY_DOWN) { 
                            // Jika panah bawah ditekan, gerakkan kursor turun satu baris (tidak boleh melebihi data terakhir).
                            if (historyState.cursorIndex < endIdx) historyState.cursorIndex++;
                        }
                    }
                }
                break;
            }
            // =========================================================
            // STATE: LAYAR DETAIL RIWAYAT SPESIFIK (HISTORY STATS)
            // =========================================================
            case HistoryStats: {
                // Menampilkan detail spesifik hanya untuk satu rekaman yang disorot oleh kursor.
                if (filteredCount > 0 && historyState.cursorIndex < filteredCount) {
                    renderHistoryStats(&filteredRecords[historyState.cursorIndex], stateChanged);
                }
                
                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;
                
                if (ch == KEY_ESC) { 
                    // Menekan ESC akan mengembalikan ke layar tabel History.
                    currentState = History;
                }
                break;
            }
            
            // =========================================================
            // STATE: LAYAR KREDIT (CREDITS)
            // =========================================================
            case Credits: {
                // Menampilkan layar tim pengembang aplikasi.
                renderCreditsScreen(stateChanged);
                
                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;
                
                if (ch == KEY_ESC) { 
                    // Menekan ESC akan kembali ke End Screen.
                    currentState = End;
                }
                break;
            }
            
            // =========================================================
            // STATE: KONFIRMASI HAPUS SEMUA RIWAYAT (CLEAR HISTORY)
            // =========================================================
            case ClearHistoryConfirmation: {
                // Menampilkan jendela dialog untuk menanyakan persetujuan penghapusan data.
                renderClearHistoryConfirmation(stateChanged);
                
                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;
                
                if (ch == 'y' || ch == 'Y') {
                    // Menghapus data asli di berkas
                    clearAllHistoryRecords();
                    
                    // Jika menyetujui, kembalikan recordCount menjadi 0 (data terhapus).
                    recordCount = 0;
                    filteredCount = 0;
                    historyState.cursorIndex = 0;
                    historyState.currentPage = 0;
                    currentState = History;
                } else if (ch == 'n' || ch == 'N') {
                    // Jika menolak/batal, langsung kembali ke History.
                    currentState = History;
                }
                break;
            }
        }
    }
    
    // Penutupan Program (Teardown)
    // Membersihkan layar dan menampilkan kembali kursor bawaan.
    clearScreen();
    showCursor();
    
    return 0;
}
