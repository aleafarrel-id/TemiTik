/**
 * @file visual.cpp
 * @brief Implementasi seluruh fungsi rendering antarmuka terminal (TUI).
 *
 * Bertanggung jawab atas manipulasi buffer konsol, penggambaran elemen UI
 * (kotak, garis, teks berwarna), dan seluruh render layar per state aplikasi.
 * Menyediakan helper internal formatTime() sebagai titik konversi terpusat
 * dari detik mentah ke format "Xm Ys" yang dipakai secara konsisten di semua
 * layar yang menampilkan durasi bermain.
 * Tidak mengandung logika permainan; hanya berhubungan dengan output visual.
 */

#include "visual.h"

// Alasan penggunaan <windows.h>: Menyediakan Windows Console API (GetStdHandle,
// GetConsoleScreenBufferInfo, SetConsoleMode, ENABLE_VIRTUAL_TERMINAL_PROCESSING)
// yang wajib digunakan untuk membaca dimensi terminal aktual dan mengaktifkan
// dukungan ANSI Escape Codes. Tidak melanggar aturan implementasi manual karena
// ini adalah abstraksi I/O sistem operasi, bukan algoritma data.
#include <windows.h>

// Alasan penggunaan <conio.h>: Disertakan untuk konsistensi header lintas modul;
// fungsi _kbhit dan _getch digunakan di main.cpp namun visual.h menjembataninya.
// Tidak ada pemanggilan langsung di file ini.
#include <conio.h>

// Alasan penggunaan <iostream>: Menyediakan std::cout sebagai satu-satunya
// mekanisme pencetakan karakter dan ANSI Escape Codes ke buffer stdout terminal.
#include <iostream>

// Alasan penggunaan <string>: Diperlukan untuk membangun string dinamis label UI
// (misalnya "Page 1/3", "Score: 100 pts") dan menghitung panjang teks untuk
// kalkulasi posisi tengah layar.
#include <string>

using namespace std;

/**
 * @brief Mengonversi total detik menjadi format teks ringkas "Xm Ys" untuk tampilan UI.
 *
 * Digunakan secara konsisten di seluruh layar yang menampilkan durasi bermain
 * (History Menu, History Stats, End Screen) agar format waktu seragam.
 * Jika menit bernilai 0, format yang dihasilkan adalah "Ys" saja.
 *
 * @param totalSeconds Total durasi dalam satuan detik (nilai dari ScoreRecord.playTimeInSeconds).
 * @return string       Representasi teks waktu dalam format "Xm Ys" atau "Ys".
 */
static string formatTime(int totalSeconds) {
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    if (minutes == 0) {
        return to_string(seconds) + "s";
    }
    return to_string(minutes) + "m " + to_string(seconds) + "s";
}

// Kode warna ANSI untuk pewarnaan teks terminal
constexpr int COLOR_RED     = 31; // Merah  — digunakan untuk layar Game Over dan konfirmasi bahaya
constexpr int COLOR_GREEN   = 32; // Hijau  — digunakan untuk elemen aktif dan kursor pilihan
constexpr int COLOR_YELLOW  = 33; // Kuning — digunakan untuk layar History dan elemen informasi
constexpr int COLOR_CYAN    = 36; // Cyan   — digunakan untuk layar Menu Utama dan elemen navigasi
constexpr int COLOR_RESET   = 0;  // Reset  — mengembalikan warna terminal ke default
constexpr int COLOR_MAGENTA = 35; // Magenta — digunakan untuk layar Credits dan placeholder search

// Konstanta dimensi dan tata letak elemen antarmuka pengguna
constexpr int MAIN_MENU_WIDTH        = 32; // Lebar kotak pilihan menu utama
constexpr int MAIN_MENU_HEIGHT       = 7;  // Tinggi kotak pilihan menu utama
constexpr int MAIN_MENU_BANNER_HEIGHT = 7; // Jumlah baris teks ASCII banner judul
constexpr int MAIN_MENU_GAP          = 2;  // Jarak vertikal antara banner dan kotak menu

constexpr int END_MENU_WIDTH  = 32; // Lebar kotak statistik di layar Game Over
constexpr int END_MENU_HEIGHT = 6;  // Tinggi kotak statistik di layar Game Over

constexpr int CREDITS_MENU_WIDTH  = 42; // Lebar kotak daftar anggota tim di layar Credits
constexpr int CREDITS_MENU_HEIGHT = 10; // Tinggi kotak daftar anggota tim di layar Credits

constexpr int HISTORY_HEADER_TOP_Y  = 8;  // Baris Y garis atas header tabel riwayat
constexpr int HISTORY_HEADER_BOTTOM_Y = 10; // Baris Y garis bawah header tabel riwayat
constexpr int HISTORY_DATA_START_Y  = 11; // Baris Y awal area data rekaman pada tabel

constexpr int GAME_BORDER_BOTTOM_Y_OFFSET = 5; // Offset garis batas bawah arena dari tepi bawah terminal
constexpr int GAME_SCORE_Y_OFFSET         = 4; // Offset baris tampilan skor dari tepi bawah terminal
constexpr int GAME_INPUT_Y_OFFSET         = 2; // Offset baris input pemain dari tepi bawah terminal


/**
 * @brief Mengambil dimensi aktual jendela terminal menggunakan Windows Console API.
 *
 * Membaca ukuran jendela konsol yang terlihat (bukan buffer scroll).
 * Jika API gagal atau ukuran di bawah minimum, nilai minimum yang dikembalikan.
 *
 * @param width  Referensi yang akan diisi dengan lebar terminal (kolom).
 * @param height Referensi yang akan diisi dengan tinggi terminal (baris).
 */
void getTerminalSize(int& width, int& height) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        width  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        // Kurangi 1 baris dari tinggi untuk mencegah scroll otomatis terminal
        height = csbi.srWindow.Bottom - csbi.srWindow.Top;
    } else {
        // Fallback ke nilai minimum jika API tidak tersedia
        width  = MIN_SCREEN_WIDTH;
        height = MIN_SCREEN_HEIGHT;
    }

    // Pastikan nilai tidak kurang dari batas minimum yang ditetapkan di dataStructs.h
    if (width  < MIN_SCREEN_WIDTH)  width  = MIN_SCREEN_WIDTH;
    if (height < MIN_SCREEN_HEIGHT) height = MIN_SCREEN_HEIGHT;
}

/**
 * @brief Menginisialisasi terminal untuk mendukung ANSI Escape Codes dan menyembunyikan kursor.
 *
 * Mengaktifkan flag ENABLE_VIRTUAL_TERMINAL_PROCESSING agar terminal Windows
 * dapat memproses kode ANSI (warna, posisi kursor) yang dikirim melalui cout.
 */
void initTerminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    // Aktifkan pemrosesan ANSI Escape Codes di konsol Windows
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    hideCursor();
}

/**
 * @brief Membersihkan seluruh isi layar terminal termasuk riwayat scrollback.
 *
 * Menggunakan tiga urutan ANSI Escape Code secara berurutan:
 * ESC[2J membersihkan layar terlihat, ESC[3J menghapus scrollback buffer,
 * ESC[H mengembalikan posisi kursor ke sudut kiri atas (1,1).
 */
void clearScreen() {
    cout << "\033[2J\033[3J\033[H";
    cout.flush();
}

/**
 * @brief Memindahkan kursor terminal ke koordinat absolut yang ditentukan.
 *
 * Menggunakan ANSI Escape Code ESC[y;xH. Koordinat berbasis 1 (1,1 = sudut kiri atas).
 *
 * @param x Posisi kolom horizontal (1-indexed).
 * @param y Posisi baris vertikal (1-indexed).
 */
void moveCursorTo(int x, int y) {
    cout << "\033[" << y << ";" << x << "H";
}

/**
 * @brief Menyembunyikan kursor berkedip agar tidak mengganggu tampilan TUI.
 *
 * Menggunakan ANSI Escape Code ESC[?25l (mode private — sembunyikan kursor).
 */
void hideCursor() {
    cout << "\033[?25l";
}

/**
 * @brief Menampilkan kembali kursor terminal ke kondisi semula.
 *
 * Menggunakan ANSI Escape Code ESC[?25h (mode private — tampilkan kursor).
 * Dipanggil saat aplikasi akan ditutup agar terminal kembali normal.
 */
void showCursor() {
    cout << "\033[?25h";
}

/**
 * @brief Mengubah warna teks output terminal menggunakan kode warna ANSI.
 *
 * Format: ESC[<kode>m. Efektif hingga resetColor() atau setColor() berikutnya dipanggil.
 *
 * @param colorCode Kode warna ANSI (31=merah, 32=hijau, 33=kuning, 35=magenta, 36=cyan).
 */
void setColor(int colorCode) {
    cout << "\033[" << colorCode << "m";
}

/**
 * @brief Mengembalikan warna teks terminal ke warna default sistem.
 *
 * Menggunakan ANSI Escape Code ESC[0m (reset semua atribut warna).
 */
void resetColor() {
    cout << "\033[0m";
}

/**
 * @brief Menggambar kotak bingkai persegi panjang menggunakan karakter ASCII.
 *
 * Sudut menggunakan '+', sisi horizontal menggunakan '-', sisi vertikal menggunakan '|'.
 * Tidak menggambar jika lebar atau tinggi tidak valid (<= 0).
 *
 * @param x      Kolom awal sudut kiri atas kotak (1-indexed).
 * @param y      Baris awal sudut kiri atas kotak (1-indexed).
 * @param width  Lebar total kotak termasuk bingkai.
 * @param height Tinggi total kotak termasuk bingkai.
 */
void drawBox(int x, int y, int width, int height) {
    if (width <= 0 || height <= 0) return;

    // Gambar baris paling atas: sudut kiri, garis, sudut kanan
    moveCursorTo(x, y);
    cout << "+";
    for (int i = 0; i < width - 2; i++) cout << "-";
    cout << "+";

    // Gambar sisi kiri dan kanan untuk setiap baris di antara tepi atas dan bawah
    for (int i = 1; i < height - 1; i++) {
        moveCursorTo(x, y + i);
        cout << "|";
        moveCursorTo(x + width - 1, y + i);
        cout << "|";
    }

    // Gambar baris paling bawah: sudut kiri, garis, sudut kanan
    moveCursorTo(x, y + height - 1);
    cout << "+";
    for (int i = 0; i < width - 2; i++) cout << "-";
    cout << "+";
}

/**
 * @brief Menggambar garis lurus horizontal menggunakan karakter '-'.
 *
 * Tidak menggambar jika panjang tidak valid (<= 0).
 *
 * @param x      Kolom awal titik kiri garis (1-indexed).
 * @param y      Baris posisi garis (1-indexed).
 * @param length Jumlah karakter '-' yang digambar.
 */
void drawHorizontalLine(int x, int y, int length) {
    if (length <= 0) return;
    moveCursorTo(x, y);
    for (int i = 0; i < length; i++) cout << "-";
}

/**
 * @brief Mencetak teks di posisi tengah horizontal suatu area layar.
 *
 * Menghitung posisi X awal berdasarkan panjang teks dan lebar area,
 * lalu memindahkan kursor ke sana sebelum mencetak.
 *
 * @param text      Teks yang akan dicetak.
 * @param y         Baris vertikal absolut tempat teks dicetak (1-indexed).
 * @param areaWidth Lebar area yang menjadi acuan perhitungan titik tengah (kolom).
 */
void printCentered(const std::string& text, int y, int areaWidth) {
    // Hitung posisi X agar teks berada di tengah area; minimal 1 agar tidak keluar batas
    int x = (areaWidth - (int)text.length()) / 2 + 1;
    if (x < 1) x = 1;
    moveCursorTo(x, y);
    cout << text;
}

/**
 * @brief Merender tampilan Start Menu utama permainan.
 *
 * Menggambar banner ASCII seni judul "TemiTik" dan kotak pilihan navigasi
 * yang dipusatkan secara vertikal dan horizontal di layar.
 * Merujuk pada desain di blueprint/Start Menu.png.
 *
 * @param fullRedraw Jika true, seluruh layar dihapus dan digambar ulang dari nol.
 *                   Jika false, fungsi tidak melakukan apa-apa (optimasi anti-flicker).
 */
void renderMainMenu(bool fullRedraw) {
    if (!fullRedraw) return;

    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);

    clearScreen();
    setColor(COLOR_CYAN);
    drawBox(1, 1, terminalWidth, terminalHeight);
    resetColor();

    // Hitung posisi Y agar grup (banner + gap + kotak menu) berada tepat di tengah vertikal
    int totalGroupHeight = MAIN_MENU_BANNER_HEIGHT + MAIN_MENU_GAP + MAIN_MENU_HEIGHT;
    int titlePositionY   = (terminalHeight - totalGroupHeight) / 2 + 1;
    if (titlePositionY < 2) titlePositionY = 2; // Jaga agar tidak menyentuh border atas

    setColor(COLOR_CYAN);
    printCentered(" _________  _______   _____ ______   ___  _________  ___  ___  __       ", titlePositionY,     terminalWidth);
    printCentered("|\\___   ___\\\\  ___ \\ |\\   _ \\  _   \\|\\  \\|\\___   ___\\\\  \\|\\  \\|\\  \\     ", titlePositionY + 1, terminalWidth);
    printCentered("\\|___ \\  \\_\\ \\   __/|\\ \\  \\\\\\__\\ \\  \\ \\  \\|___ \\  \\_\\ \\  \\ \\  \\/  /|_   ", titlePositionY + 2, terminalWidth);
    printCentered("     \\ \\  \\ \\ \\  \\_|/_\\ \\  \\\\|__| \\  \\ \\  \\   \\ \\  \\ \\ \\  \\ \\   ___  \\  ", titlePositionY + 3, terminalWidth);
    printCentered("      \\ \\  \\ \\ \\  \\_|\\ \\ \\  \\    \\ \\  \\ \\  \\   \\ \\  \\ \\ \\  \\ \\  \\\\ \\  \\ ", titlePositionY + 4, terminalWidth);
    printCentered("       \\ \\__\\ \\ \\_______\\ \\__\\    \\ \\__\\ \\__\\   \\ \\__\\ \\ \\__\\ \\__\\\\ \\__\\", titlePositionY + 5, terminalWidth);
    printCentered("        \\|__|  \\|_______|\\|__|     \\|__|\\|__|    \\|__|  \\|__|\\|__| \\|__|", titlePositionY + 6, terminalWidth);
    resetColor();

    // Hitung posisi kotak menu di bawah banner dengan jarak gap yang ditetapkan
    int menuPositionX = (terminalWidth - MAIN_MENU_WIDTH) / 2 + 1;
    int menuPositionY = titlePositionY + MAIN_MENU_BANNER_HEIGHT + MAIN_MENU_GAP;

    setColor(COLOR_CYAN);
    drawBox(menuPositionX, menuPositionY, MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT);
    printCentered(" MAIN MENU ", menuPositionY, terminalWidth);
    resetColor();

    // Setiap item menu dicetak dengan label tombol dan aksi berwarna berbeda
    int itemY = menuPositionY + 2;

    int startX = (terminalWidth - 18) / 2 + 1;
    moveCursorTo(startX, itemY);
    cout << "[ENTER] ";
    setColor(COLOR_GREEN);
    cout << "START GAME";
    resetColor();

    int histX = (terminalWidth - 11) / 2 + 1;
    moveCursorTo(histX, itemY + 1);
    cout << "[H] ";
    setColor(COLOR_YELLOW);
    cout << "HISTORY";
    resetColor();

    int exitX = (terminalWidth - 8) / 2 + 1;
    moveCursorTo(exitX, itemY + 2);
    cout << "[Q] ";
    setColor(COLOR_RED);
    cout << "EXIT";
    resetColor();
}

/**
 * @brief Merender antarmuka permainan aktif (HUD: health, skor, dan input pemain).
 *
 * Bagian statis (bingkai, border bawah, label kontrol) hanya digambar saat fullRedraw=true.
 * Bagian dinamis (health, skor, input) selalu diperbarui setiap pemanggilan.
 * Merujuk pada desain di blueprint/Game Play.png.
 *
 * @param state      Pointer ke status terkini pemain (health, skor, kecepatan, input).
 * @param fullRedraw Jika true, hapus layar dan gambar ulang bingkai statis.
 */
void renderGameUI(PlayerState* state, bool fullRedraw) {
    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);

    // Hitung posisi baris UI relatif terhadap tepi bawah terminal
    int borderY = terminalHeight - GAME_BORDER_BOTTOM_Y_OFFSET;
    int scoreY  = terminalHeight - GAME_SCORE_Y_OFFSET;
    int inputY  = terminalHeight - GAME_INPUT_Y_OFFSET;

    if (fullRedraw) {
        clearScreen();
        setColor(COLOR_GREEN);
        drawBox(1, 1, terminalWidth, terminalHeight);

        // Garis pemisah antara arena permainan dan area status di bawah
        drawHorizontalLine(2, borderY, terminalWidth - 2);
        resetColor();

        // Label kontrol keyboard di pojok kanan bawah
        string cmdPrefix = "[TAB] Restart | ";
        string cmdExit   = "[ESC] Exit";
        moveCursorTo(terminalWidth - (int)(cmdPrefix.length() + cmdExit.length()) - 2, inputY);
        setColor(COLOR_YELLOW);
        cout << cmdPrefix;
        setColor(COLOR_RED);
        cout << cmdExit;
        resetColor();
    }

    // Render indikator health: setiap slot aktif ditampilkan sebagai "<3", slot kosong sebagai spasi
    moveCursorTo(3, scoreY);
    cout << "Health: ";
    setColor(COLOR_RED);
    for (int i = 0; i < STARTING_HEALTH; i++) {
        if (i < state->currentHealth) cout << "<3 ";
        else cout << "   "; // Timpa ikon health yang sudah hilang dengan spasi kosong
    }
    resetColor();

    // Render skor dan kecepatan saat ini di tengah baris status
    string scoreText = "Score: " + to_string(state->currentScore) + " | Speed: " + to_string(state->levelSpeed);
    int scoreX = (terminalWidth - (int)scoreText.length()) / 2 + 1;
    moveCursorTo(scoreX, scoreY);
    cout << scoreText << "      "; // Padding kanan untuk menghapus sisa teks nilai lama

    // Hapus area input di tengah sebelum mencetak ulang untuk menghindari ghost text
    moveCursorTo(terminalWidth / 4, inputY);
    for (int i = 0; i < terminalWidth / 2; i++) cout << " ";

    // Cetak label "Input: > " dan teks yang sedang diketik pemain
    string labelText = "Input: > ";
    int inputX = (terminalWidth - (int)(labelText.length() + 10)) / 2 + 1;
    if (inputX < 2) inputX = 2;

    moveCursorTo(inputX, inputY);
    setColor(COLOR_YELLOW);
    cout << labelText;
    setColor(COLOR_GREEN);
    cout << state->currentInput;
    resetColor();
}

/**
 * @brief Merender layar akhir permainan (Game Over) beserta statistik sesi.
 *
 * Menampilkan banner "GAME OVER" ASCII art dan kotak statistik berisi skor
 * serta waktu bermain yang dikonversi ke format menit:detik.
 *
 * @param score         Skor akhir yang diperoleh pemain pada sesi tersebut.
 * @param timeInSeconds Total durasi sesi bermain dalam detik.
 * @param fullRedraw    Jika true, seluruh layar dihapus dan digambar ulang.
 */
void renderEndScreen(int score, int timeInSeconds, bool fullRedraw) {
    if (!fullRedraw) return;

    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);

    clearScreen();
    setColor(COLOR_RED);
    drawBox(1, 1, terminalWidth, terminalHeight);
    resetColor();

    int titlePositionY = terminalHeight / 4 - 3;

    setColor(COLOR_RED);
    printCentered(" ____    ______           ____        _____   __  __  ____    ____       ", titlePositionY,     terminalWidth);
    printCentered("/\\  _`\\ /\\  _  \\  /'\\_/`\\/\\  _`\\     /\\  __`\\/\\ \\/\\ \\/\\  _`\\ /\\  _`\\     ", titlePositionY + 1, terminalWidth);
    printCentered("\\ \\ \\L\\_\\ \\ \\L\\ \\/\\      \\ \\ \\L\\_\\   \\ \\ \\/\\ \\ \\ \\ \\ \\ \\ \\L\\_\\ \\ \\L\\ \\   ", titlePositionY + 2, terminalWidth);
    printCentered(" \\ \\ \\L_L\\ \\  __ \\ \\ \\__\\ \\ \\  _\\L    \\ \\ \\ \\ \\ \\ \\ \\ \\ \\  _\\L\\ \\ ,  /   ", titlePositionY + 3, terminalWidth);
    printCentered("  \\ \\ \\/, \\ \\ \\/\\ \\ \\ \\_/\\ \\ \\ \\L\\ \\   \\ \\ \\_\\ \\ \\ \\_/ \\ \\ \\L\\ \\ \\ \\\\ \\  ", titlePositionY + 4, terminalWidth);
    printCentered("   \\ \\____/\\ \\_\\ \\_\\ \\_\\\\ \\_\\ \\____/    \\ \\_____\\ `\\___/\\ \\____/\\ \\_\\ \\_\\", titlePositionY + 5, terminalWidth);
    printCentered("    \\/___/  \\/_/\\/_/\\/_/ \\/_/\\/___/      \\/_____/`\\/__/  \\/___/  \\/_/\\/ /", titlePositionY + 6, terminalWidth);
    resetColor();

    int statisticPositionX = (terminalWidth - END_MENU_WIDTH) / 2 + 1;
    int statisticPositionY = (terminalHeight - END_MENU_HEIGHT) / 2 + 4;

    setColor(COLOR_RED);
    drawBox(statisticPositionX, statisticPositionY, END_MENU_WIDTH, END_MENU_HEIGHT);
    resetColor();
    printCentered("STATISTIC", statisticPositionY, terminalWidth);

    // Konversi durasi ke format ringkas menggunakan helper terpusat
    string scoreString = "SCORE: " + to_string(score) + " Points";
    string timeString  = "TIME: " + formatTime(timeInSeconds);

    printCentered(scoreString, statisticPositionY + 2, terminalWidth);
    printCentered(timeString,  statisticPositionY + 3, terminalWidth);

    printCentered("[ENTER] CONTINUE", statisticPositionY + END_MENU_HEIGHT + 2, terminalWidth);
    printCentered("[C] CREDITS",      statisticPositionY + END_MENU_HEIGHT + 3, terminalWidth);
}

/**
 * @brief Merender layar Credits yang menampilkan daftar anggota tim pengembang.
 *
 * @param fullRedraw Jika true, seluruh layar dihapus dan digambar ulang.
 */
void renderCreditsScreen(bool fullRedraw) {
    if (!fullRedraw) return;

    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);

    clearScreen();
    setColor(COLOR_MAGENTA);
    drawBox(1, 1, terminalWidth, terminalHeight);
    resetColor();

    int titlePositionY = terminalHeight / 4 - 2;
    setColor(COLOR_MAGENTA);
    printCentered("CREDITS", titlePositionY, terminalWidth);
    resetColor();

    int boxPositionX = (terminalWidth - CREDITS_MENU_WIDTH) / 2 + 1;
    int boxPositionY = (terminalHeight - CREDITS_MENU_HEIGHT) / 2;

    setColor(COLOR_MAGENTA);
    drawBox(boxPositionX, boxPositionY, CREDITS_MENU_WIDTH, CREDITS_MENU_HEIGHT);
    resetColor();
    printCentered("TEAM MEMBERS",        boxPositionY,     terminalWidth);
    printCentered("1. Alea Farrel",       boxPositionY + 3, terminalWidth);
    printCentered("2. Arif Wibowo P.",    boxPositionY + 4, terminalWidth);
    printCentered("3. Aria Mahendra U.",  boxPositionY + 5, terminalWidth);
    printCentered("4. Hensa Katelu",      boxPositionY + 6, terminalWidth);
    printCentered("5. Yanuar Adi Candra", boxPositionY + 7, terminalWidth);

    printCentered("[ESC] BACK", boxPositionY + CREDITS_MENU_HEIGHT + 2, terminalWidth);
}

/**
 * @brief Merender tabel riwayat skor beserta kontrol navigasi, sorting, dan pencarian.
 *
 * Fungsi ini memiliki dua bagian render:
 * 1. Statis (fullRedraw=true): bingkai luar, header tabel, dan label kontrol yang tidak berubah.
 * 2. Dinamis (setiap frame): label sort aktif, isi search bar, data rekaman per halaman,
 *    kursor pemilih baris, garis pembatas tengah, dan nomor halaman.
 * Merujuk pada desain di blueprint/History Menu Terisi dan Kosong.png.
 *
 * @param records    Pointer ke array rekaman skor hasil filter yang akan ditampilkan.
 * @param count      Jumlah rekaman dalam array results (setelah difilter).
 * @param state      Pointer ke status navigasi riwayat (halaman, kursor, query, sort).
 * @param fullRedraw Jika true, hapus layar dan gambar ulang seluruh komponen statis.
 */
void renderHistoryMenu(ScoreRecord* records, int count, HistoryState* state, bool fullRedraw) {
    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);

    if (fullRedraw) {
        clearScreen();
        setColor(COLOR_YELLOW);
        drawBox(1, 1, terminalWidth, terminalHeight);
        resetColor();

        // Tombol navigasi kembali di sudut kiri atas
        moveCursorTo(3, 3);
        cout << "[ESC] BACK";

        setColor(COLOR_YELLOW);
        printCentered("HISTORY", 3, terminalWidth);
        resetColor();

        // Kotak pembungkus area search bar (baris 5-7)
        setColor(COLOR_YELLOW);
        drawBox(3, 5, terminalWidth - 4, 3);
        resetColor();

        // Tombol toggle mode pencarian di sudut kanan search bar
        string searchText = state->isSearchActive ? "[Q] Exit Search " : "[S] Quick Search";
        moveCursorTo(terminalWidth - (int)searchText.length() - 3, 6);
        cout << searchText;

        // Render header tabel dua kolom
        setColor(COLOR_YELLOW);

        // Garis atas header
        drawHorizontalLine(2, HISTORY_HEADER_TOP_Y, terminalWidth - 2);
        moveCursorTo(1, HISTORY_HEADER_TOP_Y);          cout << "+";
        moveCursorTo(terminalWidth, HISTORY_HEADER_TOP_Y); cout << "+";

        // Garis bawah header
        drawHorizontalLine(2, HISTORY_HEADER_BOTTOM_Y, terminalWidth - 2);
        moveCursorTo(1, HISTORY_HEADER_BOTTOM_Y);          cout << "+";
        moveCursorTo(terminalWidth, HISTORY_HEADER_BOTTOM_Y); cout << "+";

        // Garis vertikal pembatas dua kolom di tengah header
        moveCursorTo(terminalWidth / 2, HISTORY_HEADER_TOP_Y);     cout << "+";
        moveCursorTo(terminalWidth / 2, HISTORY_HEADER_TOP_Y + 1); cout << "|";
        moveCursorTo(terminalWidth / 2, HISTORY_HEADER_BOTTOM_Y);  cout << "+";

        // Garis penutup bawah tabel (footer)
        drawHorizontalLine(2, terminalHeight - 3, terminalWidth - 2);
        moveCursorTo(1, terminalHeight - 3);          cout << "+";
        moveCursorTo(terminalWidth, terminalHeight - 3); cout << "+";
        moveCursorTo(terminalWidth / 2, terminalHeight - 3); cout << "+";
        resetColor();

        // Label nama kolom header
        string scoreHeader = "SCORE (Points)";
        string timeHeader  = "TIME (min:s)";

        printCentered(scoreHeader, HISTORY_HEADER_TOP_Y + 1, terminalWidth / 2);

        // Hitung posisi X untuk header kolom kanan agar rata tengah di separuh kanan
        int rightHalfWidth = terminalWidth - (terminalWidth / 2) - 1;
        int timeX = (terminalWidth / 2) + 1 + (rightHalfWidth - (int)timeHeader.length()) / 2 + 1;
        moveCursorTo(timeX, HISTORY_HEADER_TOP_Y + 1);
        cout << timeHeader;

        // Label navigasi halaman dan hapus data
        moveCursorTo(3, terminalHeight - 2);
        cout << "[P] PREV | [N] NEXT";

        string clearText = "[C] CLEAR";
        moveCursorTo(terminalWidth - (int)clearText.length() - 2, terminalHeight - 2);
        cout << clearText;
    }

    // Hapus teks label sort sebelumnya sebelum menulis ulang (mencegah ghost text)
    string clearSortText = "                               ";
    moveCursorTo(terminalWidth - (int)clearSortText.length() - 2, 3);
    cout << clearSortText;

    // Render label sort: ASC atau DESC yang aktif diberi warna hijau
    string sortLabel = "Sort: ";
    string ascText   = "[A] ASC";
    string descText  = "[D] DESC";
    int sortLen = (int)(sortLabel.length() + ascText.length() + 3 + descText.length());

    moveCursorTo(terminalWidth - sortLen - 2, 3);
    cout << sortLabel;

    if (state->isAscending) { setColor(COLOR_GREEN); cout << ascText; resetColor(); }
    else { cout << ascText; }

    cout << " | ";

    if (!state->isAscending) { setColor(COLOR_GREEN); cout << descText; resetColor(); }
    else { cout << descText; }

    // Perbarui label tombol toggle search sesuai state aktif/tidak aktif
    string searchText = state->isSearchActive ? "[Q] Exit Search " : "[S] Quick Search";
    moveCursorTo(terminalWidth - (int)searchText.length() - 3, 6);
    setColor(COLOR_YELLOW);
    cout << searchText;
    resetColor();

    // Render isi search bar berdasarkan mode dan konten query
    moveCursorTo(5, 6);
    if (state->isSearchActive) {
        // Mode aktif: tampilkan angka yang sudah diketik diikuti kursor '|'
        // Jika belum ada input, hanya tampilkan kursor '|' sebagai penanda posisi
        if (state->searchQuery.empty()) {
            cout << "Search: |                                    ";
        } else {
            // Kursor '|' mengikuti di ujung angka; trailing spasi menimpa sisa teks lama
            cout << "Search: " << state->searchQuery << "|                              ";
        }
    } else {
        // Mode tidak aktif: tampilkan placeholder jika kosong, atau query tersimpan jika ada
        if (state->searchQuery.empty()) {
            setColor(COLOR_MAGENTA);
            cout << "Search by number...                          ";
            resetColor();
        } else {
            // Query tersimpan tetap terlihat agar pengguna tahu filter masih aktif
            cout << "Search: " << state->searchQuery << "                              ";
        }
    }

    // Kalkulasi lebar dan posisi awal dua kolom data tabel
    int leftHalfWidth  = terminalWidth / 2 - 2; // -2: kompensasi border kiri dan garis tengah
    int rightHalfStart = terminalWidth / 2 + 1;
    int rightHalfWidth = terminalWidth - rightHalfStart - 1; // -1: kompensasi border kanan

    // Bersihkan seluruh area data pada kedua kolom sebelum menggambar ulang
    for (int y = HISTORY_DATA_START_Y; y < terminalHeight - 3; y++) {
        moveCursorTo(2, y);
        for (int c = 0; c < leftHalfWidth; c++) cout << " ";

        moveCursorTo(rightHalfStart, y);
        for (int c = 0; c < rightHalfWidth; c++) cout << " ";
    }

    if (count == 0) {
        // Tidak ada data: tampilkan pesan kosong di tengah area data
        printCentered("No history available.", terminalHeight / 2, terminalWidth);
    } else {
        // Hitung indeks awal dan akhir rekaman pada halaman yang sedang aktif
        int startIndex = state->currentPage * MAX_RECORDS_PER_PAGE;
        int endIndex   = startIndex + MAX_RECORDS_PER_PAGE;
        if (endIndex > count) endIndex = count;

        // Hitung jarak vertikal antar baris agar data tersebar merata di area tabel
        int availableHeight = (terminalHeight - 3) - HISTORY_DATA_START_Y;
        int spacing = availableHeight / MAX_RECORDS_PER_PAGE;
        if (spacing < 1) spacing = 1;

        // Mulai dari tengah sel pertama agar tampak lebih seimbang secara vertikal
        int y = HISTORY_DATA_START_Y + (spacing / 2);
        if (y < HISTORY_DATA_START_Y + 1) y = HISTORY_DATA_START_Y + 1;

        for (int i = startIndex; i < endIndex; i++) {
            // Tampilkan kursor ">>" di kolom paling kiri untuk baris yang dipilih
            if (i == state->cursorIndex) {
                moveCursorTo(3, y);
                setColor(COLOR_GREEN);
                cout << ">>";
            }
            resetColor();

            // Skor rata tengah di kolom kiri
            string scoreStr = to_string(records[i].score) + " pts";
            int scoreX = 2 + (leftHalfWidth - (int)scoreStr.length()) / 2;
            moveCursorTo(scoreX, y);
            cout << scoreStr;

            // Waktu rata tengah di kolom kanan; ditampilkan dalam format "Xm Ys"
            string timeStr = formatTime(records[i].playTimeInSeconds);
            int timeX = rightHalfStart + (rightHalfWidth - (int)timeStr.length()) / 2;
            moveCursorTo(timeX, y);
            cout << timeStr;

            y += spacing;
        }
    }

    // Gambar ulang garis pemisah vertikal tengah; hapus jika data kosong agar tidak bertabrakan
    if (count > 0) {
        setColor(COLOR_YELLOW);
        for (int y = HISTORY_DATA_START_Y; y < terminalHeight - 3; y++) {
            moveCursorTo(terminalWidth / 2, y);
            cout << "|";
        }
        resetColor();
    } else {
        // Timpa garis dengan spasi agar pesan "No history available." tidak terpotong
        for (int y = HISTORY_DATA_START_Y; y < terminalHeight - 3; y++) {
            moveCursorTo(terminalWidth / 2, y);
            cout << " ";
        }
    }

    // Hitung dan cetak nomor halaman saat ini di footer tengah
    int totalPages = (count + MAX_RECORDS_PER_PAGE - 1) / MAX_RECORDS_PER_PAGE;
    if (totalPages == 0) totalPages = 1;
    string pageString = "Page " + to_string(state->currentPage + 1) + "/" + to_string(totalPages);
    int pageX = (terminalWidth - (int)pageString.length()) / 2 + 1;
    moveCursorTo(pageX, terminalHeight - 2);
    cout << pageString << "    "; // Padding kanan menghapus sisa teks halaman sebelumnya
}

/**
 * @brief Merender layar detail statistik untuk satu rekaman riwayat yang dipilih.
 *
 * Menampilkan skor dan waktu bermain rekaman tersebut dalam format yang mudah dibaca
 * (menit dan detik), di dalam kotak yang dipusatkan di layar.
 *
 * @param record     Pointer ke rekaman skor yang akan ditampilkan detailnya.
 * @param fullRedraw Jika true, seluruh layar dihapus dan digambar ulang.
 */
void renderHistoryStats(ScoreRecord* record, bool fullRedraw) {
    if (!fullRedraw) return;

    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);

    clearScreen();
    setColor(COLOR_YELLOW);
    drawBox(1, 1, terminalWidth, terminalHeight);
    resetColor();

    setColor(COLOR_YELLOW);
    printCentered("HISTORY", 5, terminalWidth);
    resetColor();

    int statisticPositionX = (terminalWidth - END_MENU_WIDTH) / 2 + 1;
    int statisticPositionY = (terminalHeight - END_MENU_HEIGHT) / 2;

    setColor(COLOR_YELLOW);
    drawBox(statisticPositionX, statisticPositionY, END_MENU_WIDTH, END_MENU_HEIGHT + 1);
    resetColor();
    printCentered("STATISTIC DETAIL", statisticPositionY, terminalWidth);

    // Konversi durasi ke format ringkas menggunakan helper terpusat
    string scoreString = "SCORE: " + to_string(record->score) + " Points";
    string timeString  = "TIME: " + formatTime(record->playTimeInSeconds);

    printCentered(scoreString, statisticPositionY + 3, terminalWidth);
    printCentered(timeString,  statisticPositionY + 4, terminalWidth);

    printCentered("[ESC] BACK", statisticPositionY + END_MENU_HEIGHT + 4, terminalWidth);
}

/**
 * @brief Merender layar konfirmasi sebelum menghapus seluruh data riwayat permainan.
 *
 * Menampilkan peringatan destruktif dan dua pilihan: konfirmasi [Y] atau batalkan [N].
 * Layar ini berwarna merah untuk menegaskan risiko tindakan yang tidak dapat dibatalkan.
 *
 * @param fullRedraw Jika true, seluruh layar dihapus dan digambar ulang.
 */
void renderClearHistoryConfirmation(bool fullRedraw) {
    if (!fullRedraw) return;

    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);

    clearScreen();
    setColor(COLOR_RED);
    drawBox(1, 1, terminalWidth, terminalHeight);
    resetColor();

    int startY = terminalHeight / 3;

    setColor(COLOR_RED);
    printCentered("!!! CLEAR HISTORY !!!", startY, terminalWidth);
    resetColor();

    printCentered("Are you sure you want to delete all history?", startY + 3, terminalWidth);
    printCentered("This action cannot be undone.",                startY + 4, terminalWidth);

    // Tombol konfirmasi dan batal dicetak berdampingan dengan warna berbeda untuk kontras visual
    string yesText  = "[Y] Yes (Delete All)";
    string gapText  = "        ";
    string noText   = "[N] No (Cancel)";

    int totalLen = (int)(yesText.length() + gapText.length() + noText.length());
    int startX   = (terminalWidth - totalLen) / 2 + 1;
    if (startX < 1) startX = 1;

    moveCursorTo(startX, startY + 8);
    setColor(COLOR_RED);
    cout << yesText;
    resetColor();
    cout << gapText;
    setColor(COLOR_CYAN);
    cout << noText;
    resetColor();
}

/**
 * @brief Stub fungsi render layar kosong — belum diimplementasikan.
 *
 * Dipertahankan sebagai titik ekstensi jika diperlukan layar khusus
 * untuk kondisi riwayat kosong terpisah dari renderHistoryMenu.
 *
 * @param fullRedraw Tidak digunakan; parameter dipertahankan untuk konsistensi antarmuka.
 */
void renderEmptyHistory(bool fullRedraw) {
    (void)fullRedraw;
}

/**
 * @brief Merender layar jeda (Pause) sebagai overlay kotak dialog di tengah layar.
 *
 * Tidak memanggil clearScreen() agar tampilan game yang sedang berjalan tetap
 * terlihat di belakang kotak dialog sebagai konteks visual. Kotak overlay
 * berwarna kuning mengikuti palet warna History untuk konsistensi tema.
 * Pilihan yang tersedia: ENTER untuk melanjutkan, Q untuk keluar ke Menu.
 *
 * @param fullRedraw Jika true, gambar kotak overlay dan isinya; jika false, tidak ada aksi.
 */
void renderPauseScreen(bool fullRedraw) {
    if (!fullRedraw) return;

    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);

    // Dimensi kotak dialog pause
    constexpr int PAUSE_BOX_WIDTH  = 36;
    constexpr int PAUSE_BOX_HEIGHT = 9;

    // Hitung posisi sudut kiri atas kotak agar tepat di tengah layar
    int boxX = (terminalWidth  - PAUSE_BOX_WIDTH)  / 2 + 1;
    int boxY = (terminalHeight - PAUSE_BOX_HEIGHT) / 2;
    if (boxX < 1) boxX = 1;
    if (boxY < 1) boxY = 1;

    // Gambar kotak dialog di atas layar game yang sudah ada (overlay)
    setColor(COLOR_CYAN);
    drawBox(boxX, boxY, PAUSE_BOX_WIDTH, PAUSE_BOX_HEIGHT);
    resetColor();

    // Judul layar Pause di tepi atas kotak, rata tengah
    setColor(COLOR_CYAN);
    printCentered("|| PAUSED ||", boxY, terminalWidth);
    resetColor();

    // Pesan status di baris pertama isi kotak
    printCentered("Game is paused.", boxY + 2, terminalWidth);

    // Pilihan aksi: ENTER untuk lanjut, Q untuk keluar
    int continueX = boxX + (PAUSE_BOX_WIDTH - 22) / 2;
    moveCursorTo(continueX, boxY + 4);
    cout << "[ENTER] ";
    setColor(COLOR_GREEN);
    cout << "Continue";
    resetColor();

    int quitX = boxX + (PAUSE_BOX_WIDTH - 14) / 2;
    moveCursorTo(quitX, boxY + 5);
    cout << "[Q] ";
    setColor(COLOR_RED);
    cout << "Quit to Menu";
    resetColor();
}
