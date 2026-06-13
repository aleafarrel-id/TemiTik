/**
 * @file main.cpp
 * @brief Titik masuk tunggal aplikasi TemiTik.
 *
 * Menginisialisasi seluruh struktur data global (PlayerState, Queue, historyRecords,
 * filteredRecords, HistoryState) lalu menjalankan state machine 7-state aplikasi:
 * Menu, Play, End, History, HistoryStats, Credits, ClearHistoryConfirmation.
 * Setiap state menangani rendering dan input secara mandiri, kemudian mendelegasikan
 * logika domain ke modul spesifik: visual, loader, gameEngine, history.
 */

// <iostream>: Diperlukan oleh komponen cout/cin untuk penulisan ke buffer stdout terminal.
// Tidak melanggar aturan manual karena hanya digunakan untuk I/O dasar, bukan algoritma.
#include <iostream>

// <conio.h>: Menyediakan _kbhit() dan _getch() untuk polling input keyboard non-blocking
// tanpa menunggu ENTER. Wajib untuk navigasi menu dan deteksi keystroke real-time di Windows.
// Tidak melanggar aturan manual karena bukan STL algorithm atau data structure.
#include <conio.h>

// <windows.h>: Menyediakan API sistem operasi Windows level rendah: GetConsoleScreenBufferInfo
// untuk deteksi ukuran terminal, FillConsoleOutputCharacter untuk rendering TUI, dan Sleep
// untuk throttling polling loop. Tidak melanggar aturan manual karena hanya digunakan
// sebagai jembatan ke OS, bukan sebagai pengganti algoritma atau struktur data.
#include <windows.h>

#include "dataStructs.h"
#include "visual.h"
#include "loader.h"
#include "gameEngine.h"
#include "history.h"

using namespace std;

/**
 * @brief Menunggu input keyboard secara asinkron sambil memantau perubahan ukuran terminal.
 *
 * Fungsi ini melakukan polling _kbhit() dan ukuran terminal secara bergantian
 * dalam loop tanpa batas. Jika terdeteksi keystroke, karakter langsung dikembalikan.
 * Jika ukuran terminal berubah, fungsi mengembalikan 0 sebagai sinyal resize
 * agar pemanggil dapat memicu render ulang layar.
 *
 * @param currentWidth  Referensi ke lebar terminal tersimpan; diperbarui oleh pemanggil.
 * @param currentHeight Referensi ke tinggi terminal tersimpan; diperbarui oleh pemanggil.
 * @return int Kode karakter yang ditekan pengguna, atau 0 jika terminal di-resize.
 */
int getAsyncInputOrResize(int& currentWidth, int& currentHeight) {
    while (true) {
        // _kbhit() mengembalikan non-zero jika ada keystroke menunggu di buffer;
        // polling non-blocking agar loop tidak berhenti menunggu input.
        if (_kbhit()) {
            return _getch(); // Ambil dan kembalikan karakter dari buffer input
        }

        // Cek ukuran terminal saat ini untuk mendeteksi event resize jendela.
        int checkWidth, checkHeight;
        getTerminalSize(checkWidth, checkHeight);
        if (checkWidth != currentWidth || checkHeight != currentHeight) {
            return 0; // Nilai 0 digunakan sebagai sinyal resize; bukan kode karakter valid
        }

        // Sleep singkat mencegah busy-wait yang memakan 100% CPU;
        // ASYNC_INPUT_SLEEP_MS dikalibrasi agar respons input tetap terasa instan.
        Sleep(ASYNC_INPUT_SLEEP_MS);
    }
}

int main() {
    // Inisialisasi terminal: mengaktifkan dukungan ANSI escape code dan
    // menyembunyikan kursor bawaan konsol agar TUI tidak berkedip.
    initTerminal();

    // Flag pengontrol loop utama; false menyebabkan aplikasi keluar bersih.
    bool isRunning = true;

    // State machine utama: currentState menentukan layar aktif setiap iterasi.
    // Dimulai dari Menu sebagai layar awal aplikasi.
    GameState currentState = Menu;
    // previousState digunakan untuk mendeteksi transisi state dan memicu render penuh.
    GameState previousState = (GameState)-1;

    // playerState: menyimpan kondisi pemain aktif (nama, nyawa, skor, kecepatan).
    // Diinisialisasi dengan nilai awal: STARTING_HEALTH nyawa, skor 0, kecepatan INITIAL_DROP_SPEED.
    PlayerState playerState = {"", STARTING_HEALTH, 0, INITIAL_DROP_SPEED};

    // historyRecords: array statis penampung seluruh rekaman skor dari file.
    // Kapasitas maksimum dibatasi oleh konstanta MAX_HISTORY_RECORDS.
    ScoreRecord historyRecords[MAX_HISTORY_RECORDS];
    // Muat rekaman dari file persistent ke memori; recordCount mencatat jumlah rekaman valid.
    int recordCount = loadHistoryRecords(historyRecords);

    // filteredRecords: array statis penampung subset historyRecords hasil filter pencarian.
    // filteredCount diinisialisasi sama dengan recordCount (belum ada filter aktif).
    ScoreRecord filteredRecords[MAX_HISTORY_RECORDS];
    int filteredCount = recordCount;

    // Durasi sesi permainan terakhir dalam satuan detik; diteruskan ke layar End dan history.
    int currentSessionTime = 0;

    // historyState: menyimpan seluruh UI state layar History secara terpusat
    // (query pencarian, halaman aktif, posisi kursor, flag pencarian dan urutan).
    HistoryState historyState = {"", 0, 0, -1, true, false};

    // wordQueue: antrean kata aktif untuk sesi permainan.
    // Diinisialisasi sebagai antrean kosong; diisi ulang setiap kali sesi Play dimulai.
    Queue wordQueue;
    wordQueue.front = nullptr;
    wordQueue.rear  = nullptr;
    wordQueue.count = 0;

    // Rekam ukuran terminal awal; dipakai sebagai baseline deteksi resize di setiap iterasi.
    int currentTerminalWidth, currentTerminalHeight;
    getTerminalSize(currentTerminalWidth, currentTerminalHeight);

    // Loop utama: berjalan terus hingga isRunning menjadi false.
    // Setiap iterasi menangani rendering dan input untuk state yang sedang aktif.
    while (isRunning) {
        // Polling ukuran terminal di awal setiap iterasi untuk mendeteksi resize jendela.
        int newTerminalWidth, newTerminalHeight;
        getTerminalSize(newTerminalWidth, newTerminalHeight);
        bool terminalResized = false;

        if (newTerminalWidth != currentTerminalWidth || newTerminalHeight != currentTerminalHeight) {
            currentTerminalWidth  = newTerminalWidth;
            currentTerminalHeight = newTerminalHeight;
            terminalResized = true; // Tandai agar render ulang penuh dipaksa pada iterasi ini
        }

        // stateChanged true jika state baru berbeda dari sebelumnya atau terminal di-resize;
        // dipakai modul rendering untuk memilih antara render penuh vs. render parsial.
        bool stateChanged = (currentState != previousState) || terminalResized;
        previousState = currentState;

        switch (currentState) {

            // =========================================================
            // STATE: LAYAR MENU UTAMA
            // Transisi keluar: ENTER -> Play, H -> History, Q -> keluar aplikasi.
            // =========================================================
            case Menu: {
                // Render bingkai dan opsi menu utama; render penuh hanya jika stateChanged.
                renderMainMenu(stateChanged);

                // Blokir iterasi hingga ada keystroke atau event resize terminal.
                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue; // Resize terdeteksi; lanjut ke iterasi berikutnya untuk render ulang

                if (ch == KEY_ENTER_WIN || ch == KEY_ENTER_NIX) {
                    // Muat bank kata dari file ke wordQueue sebelum sesi dimulai.
                    loadWordsFromFile("data/wordBank.txt", &wordQueue);

                    // Reset seluruh atribut pemain ke nilai awal agar sesi bersih.
                    playerState.currentHealth = STARTING_HEALTH;
                    playerState.currentScore  = 0;
                    playerState.levelSpeed    = INITIAL_DROP_SPEED;
                    playerState.currentInput  = "";

                    currentState = Play;
                } else if (ch == 'h' || ch == 'H') {
                    currentState = History;
                } else if (ch == 'q' || ch == 'Q') {
                    isRunning = false; // Hentikan loop utama; program menuju teardown
                }
                break;
            }

            // =========================================================
            // STATE: SESI PERMAINAN (GAME PLAY)
            // Transisi keluar: ditentukan oleh runGameLoop (ke End atau Menu).
            // =========================================================
            case Play: {
                // Catat timestamp awal sesi untuk menghitung durasi permainan.
                ULONGLONG sessionStartTime = GetTickCount64();

                // Delegasikan seluruh logika game loop ke modul gameEngine;
                // fungsi ini bersifat blocking hingga sesi berakhir (mati atau keluar).
                runGameLoop(&playerState, currentState, &wordQueue);

                // Hitung durasi sesi dalam detik menggunakan selisih tick.
                currentSessionTime = (GetTickCount64() - sessionStartTime) / MS_PER_SECOND;

                // Bersihkan sisa node di wordQueue untuk mencegah memory leak;
                // node yang belum dikonsumsi gameEngine harus dibebaskan secara manual.
                while (wordQueue.front != nullptr) {
                    QueueNode* temp  = wordQueue.front;
                    wordQueue.front  = wordQueue.front->next;
                    delete temp;
                }
                wordQueue.rear  = nullptr;
                wordQueue.count = 0;
                break;
            }

            // =========================================================
            // STATE: LAYAR AKHIR PERMAINAN (GAME OVER / END)
            // Transisi keluar: ENTER -> simpan skor & kembali ke Menu, C -> Credits.
            // =========================================================
            case End: {
                // Render statistik akhir: skor dan durasi sesi diteruskan langsung ke renderer.
                renderEndScreen(playerState.currentScore, currentSessionTime, stateChanged);

                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;

                if (ch == KEY_ENTER_WIN || ch == KEY_ENTER_NIX) {
                    // Simpan rekaman skor dan durasi sesi ke file persistent.
                    ScoreRecord newRecord = {playerState.currentScore, currentSessionTime};
                    saveRecordToFile(newRecord);

                    // Muat ulang historyRecords dari file agar data terbaru tersedia di layar History.
                    recordCount = loadHistoryRecords(historyRecords);

                    currentState = Menu;
                } else if (ch == 'c' || ch == 'C') {
                    currentState = Credits;
                }
                break;
            }

            // =========================================================
            // STATE: LAYAR DAFTAR RIWAYAT (HISTORY MENU)
            // Transisi keluar: ESC -> Menu, ENTER -> HistoryStats, C -> ClearHistoryConfirmation.
            // =========================================================
            case History: {
                // Urutkan historyRecords setiap frame agar perubahan mode sort (A/D)
                // langsung terefleksi pada iterasi berikutnya tanpa state tambahan.
                if (historyState.isAscending) {
                    sortRecordsAscending(historyRecords, recordCount);
                } else {
                    sortRecordsDescending(historyRecords, recordCount);
                }

                // Filter historyRecords berdasarkan searchQuery ke filteredRecords;
                // dipanggil setiap frame agar perubahan query langsung memperbarui tampilan.
                // filterHistoryRecords menggunakan binary search pada array terurut;
                // oleh karena itu harus dipanggil setelah sorting selesai.
                filteredCount = filterHistoryRecords(historyRecords, recordCount, filteredRecords, historyState.searchQuery, historyState.isAscending);

                // Clamp posisi kursor agar selalu berada di dalam rentang baris halaman aktif.
                // Jika tidak ada data, paksa kursor ke index 0 agar tidak mengakses memori invalid.
                if (filteredCount == 0) {
                    historyState.cursorIndex = 0;
                } else {
                    // Hitung batas indeks baris pertama dan terakhir pada halaman saat ini.
                    int startIdx = historyState.currentPage * MAX_RECORDS_PER_PAGE;
                    int endIdx   = startIdx + MAX_RECORDS_PER_PAGE - 1;
                    // Clamp endIdx agar tidak melampaui jumlah rekaman yang tersedia.
                    if (endIdx >= filteredCount) endIdx = filteredCount - 1;

                    // Paksa kursor masuk ke dalam rentang [startIdx, endIdx].
                    if (historyState.cursorIndex < startIdx) historyState.cursorIndex = startIdx;
                    if (historyState.cursorIndex > endIdx)   historyState.cursorIndex = endIdx;
                }

                // Render tabel riwayat; jika filteredCount == 0, renderer menampilkan peringatan kosong.
                renderHistoryMenu(filteredRecords, filteredCount, &historyState, stateChanged);

                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;

                if (historyState.isSearchActive) {
                    // Catat panjang query sebelum modifikasi apa pun;
                    // digunakan untuk mendeteksi apakah query benar-benar berubah
                    // sehingga reset halaman hanya terjadi saat query berubah, bukan setiap frame.
                    int prevQueryLen = (int)historyState.searchQuery.length();

                    if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
                        // Tutup mode pencarian tanpa menghapus query;
                        // filter tetap aktif agar pengguna dapat melihat hasil terakhir
                        // sambil menggunakan navigasi normal (panah, halaman).
                        historyState.isSearchActive = false;
                    } else if (ch == KEY_ENTER_WIN || ch == KEY_ENTER_NIX) {
                        // ENTER diabaikan saat mode pencarian aktif untuk mencegah
                        // transisi tidak sengaja ke HistoryStats saat pengguna
                        // mengira sedang mengonfirmasi input teks.
                    } else if (ch == '\b') {
                        // Backspace: hapus karakter terakhir query.
                        // Guard empty() diperlukan untuk menghindari pop_back() pada string kosong
                        // yang merupakan undefined behavior.
                        if (!historyState.searchQuery.empty()) {
                            historyState.searchQuery.pop_back();
                        }
                    } else if (isdigit(ch)) {
                        // Hanya digit (0-9) yang diterima sesuai kontrak PRD:
                        // field yang dapat dicari adalah skor dan durasi, keduanya numerik.
                        historyState.searchQuery += (char)ch;
                    }

                    // Jika panjang query berubah (pengguna mengetik atau backspace),
                    // reset currentPage ke 0 dan cursorIndex ke 0 agar kursor tidak
                    // berada di luar rentang hasil filter yang baru dan lebih pendek.
                    if ((int)historyState.searchQuery.length() != prevQueryLen) {
                        historyState.currentPage  = 0;
                        historyState.cursorIndex  = 0;
                    }
                } else {
                    if (ch == KEY_ESC) {
                        currentState = Menu;
                    } else if (ch == 's' || ch == 'S') {
                        // Aktifkan mode pencarian; input selanjutnya akan diproses sebagai query.
                        historyState.isSearchActive = true;
                    } else if (ch == KEY_ENTER_WIN || ch == KEY_ENTER_NIX) {
                        // Buka detail rekaman yang disorot kursor; hanya jika ada data.
                        if (recordCount > 0) currentState = HistoryStats;
                    } else if (ch == 'c' || ch == 'C') {
                        currentState = ClearHistoryConfirmation;
                    } else if (ch == 'a' || ch == 'A') {
                        // Ubah mode urutan ke menaik (Ascending); sort diterapkan pada frame berikutnya.
                        historyState.isAscending = true;
                    } else if (ch == 'd' || ch == 'D') {
                        // Ubah mode urutan ke menurun (Descending); sort diterapkan pada frame berikutnya.
                        historyState.isAscending = false;
                    } else if (ch == 'p' || ch == 'P') {
                        // Mundur satu halaman; guard > 0 mencegah underflow ke halaman negatif.
                        if (historyState.currentPage > 0) historyState.currentPage--;
                    } else if (ch == 'n' || ch == 'N') {
                        // Maju satu halaman; guard mencegah melampaui halaman terakhir yang valid.
                        int maxPage = (filteredCount > 0) ? (filteredCount - 1) / MAX_RECORDS_PER_PAGE : 0;
                        if (historyState.currentPage < maxPage) historyState.currentPage++;
                    } else if (ch == KEY_ARROW_PREFIX1 || ch == KEY_ARROW_PREFIX2) {
                        // Tombol panah memancarkan dua kode byte: prefix diikuti kode arah.
                        // Tangkap byte kedua untuk menentukan arah navigasi kursor.
                        ch = _getch();

                        // Hitung batas baris halaman aktif untuk clamp pergerakan kursor.
                        int startIdx = historyState.currentPage * MAX_RECORDS_PER_PAGE;
                        int endIdx   = startIdx + MAX_RECORDS_PER_PAGE - 1;
                        if (endIdx >= filteredCount) endIdx = filteredCount - 1;

                        if (ch == KEY_UP) {
                            // Naikkan kursor satu baris; guard mencegah keluar batas atas halaman.
                            if (historyState.cursorIndex > startIdx) historyState.cursorIndex--;
                        } else if (ch == KEY_DOWN) {
                            // Turunkan kursor satu baris; guard mencegah keluar batas bawah halaman.
                            if (historyState.cursorIndex < endIdx) historyState.cursorIndex++;
                        }
                    }
                }
                break;
            }

            // =========================================================
            // STATE: LAYAR DETAIL REKAMAN RIWAYAT (HISTORY STATS)
            // Transisi keluar: ESC -> History.
            // =========================================================
            case HistoryStats: {
                // Render detail satu rekaman yang disorot kursor; guard mencegah akses array invalid
                // jika filteredCount berubah menjadi 0 akibat perubahan query eksternal.
                if (filteredCount > 0 && historyState.cursorIndex < filteredCount) {
                    renderHistoryStats(&filteredRecords[historyState.cursorIndex], stateChanged);
                }

                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;

                if (ch == KEY_ESC) {
                    // Kembali ke tabel riwayat; posisi kursor dan halaman dipertahankan.
                    currentState = History;
                }
                break;
            }

            // =========================================================
            // STATE: LAYAR KREDIT TIM PENGEMBANG (CREDITS)
            // Transisi keluar: ESC -> End.
            // =========================================================
            case Credits: {
                // Render layar daftar anggota tim pengembang aplikasi.
                renderCreditsScreen(stateChanged);

                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;

                if (ch == KEY_ESC) {
                    // Kembali ke layar End; kredit hanya dapat diakses dari End Screen.
                    currentState = End;
                }
                break;
            }

            // =========================================================
            // STATE: DIALOG KONFIRMASI HAPUS SEMUA RIWAYAT (CLEAR HISTORY CONFIRMATION)
            // Transisi keluar: Y -> hapus & kembali ke History, N -> batal & kembali ke History.
            // =========================================================
            case ClearHistoryConfirmation: {
                // Render dialog konfirmasi dua-opsi (Y/N) sebelum penghapusan permanen.
                renderClearHistoryConfirmation(stateChanged);

                int ch = getAsyncInputOrResize(currentTerminalWidth, currentTerminalHeight);
                if (ch == 0) continue;

                if (ch == 'y' || ch == 'Y') {
                    // Hapus seluruh rekaman dari file persistent secara permanen.
                    clearAllHistoryRecords();

                    // Reset seluruh state terkait history ke kondisi awal:
                    recordCount  = 0;
                    filteredCount = 0;
                    historyState.cursorIndex  = 0;
                    historyState.currentPage  = 0;
                    // Bersihkan searchQuery agar tabel History tampil kosong bersih
                    // tanpa sisa filter dari sesi sebelumnya yang tidak lagi relevan.
                    historyState.searchQuery  = "";
                    // Nonaktifkan mode pencarian agar pengguna tidak terjebak di input mode
                    // saat kembali ke layar History yang sudah kosong.
                    historyState.isSearchActive = false;
                    currentState = History;
                } else if (ch == 'n' || ch == 'N') {
                    // Pengguna membatalkan; kembali ke History tanpa modifikasi data.
                    currentState = History;
                }
                break;
            }
        }
    }

    // Teardown: bersihkan layar terminal agar tidak meninggalkan artefak TUI
    // setelah aplikasi keluar, kemudian tampilkan kembali kursor yang disembunyikan
    // saat initTerminal() dipanggil di awal program.
    clearScreen();
    showCursor();

    return 0;
}
