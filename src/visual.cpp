
#include "visual.h"
// Alasan penggunaan <windows.h>: Esensial untuk merender UI di Windows (GetStdHandle, FillConsoleOutputCharacter, dll) yang tidak melanggar aturan implementasi logika prosedural.
#include <windows.h>
// Alasan penggunaan <conio.h>: Digunakan untuk input asinkron pada menu meskipun pada visual.cpp jarang dipakai secara langsung.
#include <conio.h>
// Alasan penggunaan <iostream>: Diperlukan untuk stream output (cout) pencetakan ASCII ke layar.
#include <iostream>
// Alasan penggunaan <string>: Diperlukan untuk manipulasi teks statis dan argumen fungsi cetak.
#include <string>

using namespace std;

constexpr int COLOR_RED = 31;
constexpr int COLOR_GREEN = 32;
constexpr int COLOR_YELLOW = 33;
constexpr int COLOR_CYAN = 36;
constexpr int COLOR_RESET = 0;
constexpr int COLOR_MAGENTA = 35;

/**
 * @brief Mengambil ukuran terminal saat ini menggunakan Windows API.
 * 
 * @param width Referensi untuk menyimpan lebar (kolom).
 * @param height Referensi untuk menyimpan tinggi (baris).
 */
void getTerminalSize(int& width, int& height) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top; // Leave 1 line for safety to prevent scroll
    } else {
        width = MIN_SCREEN_WIDTH;
        height = MIN_SCREEN_HEIGHT;
    }
    
    if (width < MIN_SCREEN_WIDTH) width = MIN_SCREEN_WIDTH;
    if (height < MIN_SCREEN_HEIGHT) height = MIN_SCREEN_HEIGHT;
}

/**
 * @brief Menginisialisasi terminal untuk mode virtual processing dan menyembunyikan kursor.
 */
void initTerminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    
    hideCursor();
}

/**
 * @brief Membersihkan seluruh layar terminal dengan karakter spasi kosong.
 */
void clearScreen() {
    // Menggunakan ANSI Escape Codes untuk membersihkan layar secara menyeluruh.
    // \033[2J : Membersihkan seluruh layar yang terlihat.
    // \033[3J : Membersihkan riwayat scrollback (menghilangkan bekas render sebelumnya).
    // \033[H  : Mengembalikan posisi kursor ke ujung kiri atas (1, 1).
    cout << "\033[2J\033[3J\033[H";
    cout.flush();
}

/**
 * @brief Memindahkan kursor terminal ke koordinat spesifik.
 * 
 * @param x Koordinat horizontal (kolom).
 * @param y Koordinat vertikal (baris).
 */
void moveCursorTo(int x, int y) {
    cout << "\033[" << y << ";" << x << "H";
}

/**
 * @brief Menyembunyikan kursor berkedip pada terminal.
 */
void hideCursor() {
    cout << "\033[?25l";
}

/**
 * @brief Menampilkan kembali kursor berkedip pada terminal.
 */
void showCursor() {
    cout << "\033[?25h";
}

/**
 * @brief Mengubah warna teks menggunakan kode warna ANSI.
 * 
 * @param colorCode Kode warna ANSI (misal: 31 untuk merah).
 */
void setColor(int colorCode) {
    cout << "\033[" << colorCode << "m";
}

/**
 * @brief Mengembalikan warna teks ke warna default terminal.
 */
void resetColor() {
    cout << "\033[0m";
}

/**
 * @brief Menggambar kotak bingkai di layar terminal menggunakan karakter ASCII.
 * 
 * @param x Posisi kolom awal.
 * @param y Posisi baris awal.
 * @param width Lebar kotak.
 * @param height Tinggi kotak.
 */
void drawBox(int x, int y, int width, int height) {
    if (width <= 0 || height <= 0) return;
    
    moveCursorTo(x, y);
    cout << "+";
    for (int i = 0; i < width - 2; i++) cout << "-";
    cout << "+";
    
    for (int i = 1; i < height - 1; i++) {
        moveCursorTo(x, y + i);
        cout << "|";
        moveCursorTo(x + width - 1, y + i);
        cout << "|";
    }
    
    moveCursorTo(x, y + height - 1);
    cout << "+";
    for (int i = 0; i < width - 2; i++) cout << "-";
    cout << "+";
}

/**
 * @brief Menggambar garis horizontal menggunakan karakter ASCII.
 * 
 * @param x Posisi kolom awal.
 * @param y Posisi baris awal.
 * @param length Panjang garis.
 */
void drawHorizontalLine(int x, int y, int length) {
    if (length <= 0) return;
    moveCursorTo(x, y);
    for (int i = 0; i < length; i++) cout << "-";
}

/**
 * @brief Mencetak teks secara absolut di tengah horizontal terminal.
 * 
 * @param text String yang ingin dicetak.
 * @param y Posisi baris vertikal absolut.
 * @param areaWidth Lebar area yang menjadi referensi perhitungan titik tengah.
 */
void printCentered(const std::string& text, int y, int areaWidth) {
    int x = (areaWidth - text.length()) / 2 + 1;
    if (x < 1) x = 1; 
    moveCursorTo(x, y);
    cout << text;
}

/**
 * @brief Merender tampilan antarmuka Start Menu.
 * Merujuk secara spesifik pada referensi UI di `blueprint/Start Menu.png`.
 * 
 * @param fullRedraw Flag untuk merender ulang seluruh komponen secara keseluruhan (menghindari flicker).
 */
void renderMainMenu(bool fullRedraw) {
    if (!fullRedraw) return;

    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);
    
    clearScreen();
    // Kembalikan warna bingkai luar menjadi Cyan
    setColor(COLOR_CYAN);
    drawBox(1, 1, terminalWidth, terminalHeight);
    resetColor();
    
    // Kalkulasi posisi agar benar-benar berada di tengah layar secara vertikal
    int totalGroupHeight = 7 + 2 + 7; // Banner (7), Jarak (2), Box (7) = Total 16 baris
    int titlePositionY = (terminalHeight - totalGroupHeight) / 2 + 1;
    if (titlePositionY < 2) titlePositionY = 2; // Batas aman dari atas
    
    setColor(COLOR_CYAN);
    printCentered(" _________  _______   _____ ______   ___  _________  ___  ___  __       ", titlePositionY, terminalWidth);
    printCentered("|\\___   ___\\\\  ___ \\ |\\   _ \\  _   \\|\\  \\|\\___   ___\\\\  \\|\\  \\|\\  \\     ", titlePositionY + 1, terminalWidth);
    printCentered("\\|___ \\  \\_\\ \\   __/|\\ \\  \\\\\\__\\ \\  \\ \\  \\|___ \\  \\_\\ \\  \\ \\  \\/  /|_   ", titlePositionY + 2, terminalWidth);
    printCentered("     \\ \\  \\ \\ \\  \\_|/_\\ \\  \\\\|__| \\  \\ \\  \\   \\ \\  \\ \\ \\  \\ \\   ___  \\  ", titlePositionY + 3, terminalWidth);
    printCentered("      \\ \\  \\ \\ \\  \\_|\\ \\ \\  \\    \\ \\  \\ \\  \\   \\ \\  \\ \\ \\  \\ \\  \\\\ \\  \\ ", titlePositionY + 4, terminalWidth);
    printCentered("       \\ \\__\\ \\ \\_______\\ \\__\\    \\ \\__\\ \\__\\   \\ \\__\\ \\ \\__\\ \\__\\\\ \\__\\", titlePositionY + 5, terminalWidth);
    printCentered("        \\|__|  \\|_______|\\|__|     \\|__|\\|__|    \\|__|  \\|__|\\|__| \\|__|", titlePositionY + 6, terminalWidth);
    resetColor();
    
    int menuWidth = 32;
    int menuHeight = 7; 
    int menuPositionX = (terminalWidth - menuWidth) / 2 + 1;
    int menuPositionY = titlePositionY + 7 + 2; // Mulai kotak 2 baris di bawah banner
    
    setColor(COLOR_CYAN);
    drawBox(menuPositionX, menuPositionY, menuWidth, menuHeight);
    printCentered(" MAIN MENU ", menuPositionY, terminalWidth);
    resetColor();
    
    int itemY = menuPositionY + 2; // Rapat
    
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
 * @brief Merender antarmuka sesi gameplay secara asinkron.
 * Merujuk secara spesifik pada referensi UI di `blueprint/Game Play.png`.
 * 
 * @param state State pemain yang sedang aktif (health, score, dll).
 * @param fullRedraw Flag untuk merender ulang bingkai dasar game saat pertama kali masuk.
 */
void renderGameUI(PlayerState* state, bool fullRedraw) {
    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);
    int statusBarPositionY = terminalHeight - 3;
    
    if (fullRedraw) {
        clearScreen();
        setColor(COLOR_GREEN);
        drawBox(1, 1, terminalWidth, terminalHeight);
        
        drawHorizontalLine(2, statusBarPositionY - 1, terminalWidth - 2);
        resetColor();
        
        string commandsText = "[TAB] Restart [ESC] Back";
        moveCursorTo(terminalWidth - commandsText.length() - 2, statusBarPositionY + 1);
        setColor(COLOR_YELLOW);
        cout << commandsText;
        resetColor();
        
        setColor(COLOR_GREEN);
        printCentered("Type anything to start", terminalHeight / 3, terminalWidth);
        resetColor();
    }
    
    moveCursorTo(3, statusBarPositionY);
    cout << "Health: ";
    setColor(COLOR_RED);
    for (int i = 0; i < STARTING_HEALTH; i++) {
        if (i < state->currentHealth) cout << "<3 ";
        else cout << "   ";
    }
    resetColor();
    
    string scoreText = "Score: " + to_string(state->currentScore) + " | Speed: " + to_string(state->levelSpeed);
    int scoreX = (terminalWidth - scoreText.length()) / 2 + 1;
    moveCursorTo(scoreX, statusBarPositionY);
    cout << scoreText << "      ";
    
    moveCursorTo(3, statusBarPositionY + 1);
    cout << "Input: > " << state->currentInput << "                              ";
}

/**
 * @brief Merender antarmuka layar akhir permainan (Game Over).
 * 
 * @param score Total skor yang didapat pemain.
 * @param timeInSeconds Total waktu bermain dalam detik.
 * @param fullRedraw Flag untuk merender ulang seluruh komponen layar.
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
    printCentered(" ____    ______           ____        _____   __  __  ____    ____       ", titlePositionY, terminalWidth);
    printCentered("/\\  _`\\ /\\  _  \\  /'\\_/`\\/\\  _`\\     /\\  __`\\/\\ \\/\\ \\/\\  _`\\ /\\  _`\\     ", titlePositionY + 1, terminalWidth);
    printCentered("\\ \\ \\L\\_\\ \\ \\L\\ \\/\\      \\ \\ \\L\\_\\   \\ \\ \\/\\ \\ \\ \\ \\ \\ \\ \\L\\_\\ \\ \\L\\ \\   ", titlePositionY + 2, terminalWidth);
    printCentered(" \\ \\ \\L_L\\ \\  __ \\ \\ \\__\\ \\ \\  _\\L    \\ \\ \\ \\ \\ \\ \\ \\ \\ \\  _\\L\\ \\ ,  /   ", titlePositionY + 3, terminalWidth);
    printCentered("  \\ \\ \\/, \\ \\ \\/\\ \\ \\ \\_/\\ \\ \\ \\L\\ \\   \\ \\ \\_\\ \\ \\ \\_/ \\ \\ \\L\\ \\ \\ \\\\ \\  ", titlePositionY + 4, terminalWidth);
    printCentered("   \\ \\____/\\ \\_\\ \\_\\ \\_\\\\ \\_\\ \\____/    \\ \\_____\\ `\\___/\\ \\____/\\ \\_\\ \\_\\", titlePositionY + 5, terminalWidth);
    printCentered("    \\/___/  \\/_/\\/_/\\/_/ \\/_/\\/___/      \\/_____/`\\/__/  \\/___/  \\/_/\\/ /", titlePositionY + 6, terminalWidth);
    resetColor();
    
    int statisticWidth = 32;
    int statisticHeight = 6;
    int statisticPositionX = (terminalWidth - statisticWidth) / 2 + 1;
    int statisticPositionY = (terminalHeight - statisticHeight) / 2 + 4;
    
    setColor(COLOR_RED);
    drawBox(statisticPositionX, statisticPositionY, statisticWidth, statisticHeight);
    resetColor();
    printCentered("STATISTIC", statisticPositionY, terminalWidth);
    
    int minutes = timeInSeconds / 60;
    int seconds = timeInSeconds % 60;
    
    string scoreString = "SCORE: " + to_string(score) + " Points";
    string timeString = "TIME: " + to_string(minutes) + "m " + to_string(seconds) + "s";
    
    printCentered(scoreString, statisticPositionY + 2, terminalWidth);
    printCentered(timeString, statisticPositionY + 3, terminalWidth);
    
    printCentered("[ENTER] CONTINUE", statisticPositionY + statisticHeight + 2, terminalWidth);
    printCentered("[C] CREDITS", statisticPositionY + statisticHeight + 3, terminalWidth);
}

/**
 * @brief Merender antarmuka layar kredit anggota tim pembuat permainan.
 * 
 * @param fullRedraw Flag untuk merender ulang seluruh komponen layar.
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
    
    int boxWidth = 42;
    int boxHeight = 10;
    int boxPositionX = (terminalWidth - boxWidth) / 2 + 1;
    int boxPositionY = (terminalHeight - boxHeight) / 2;
        
    setColor(COLOR_MAGENTA);
    drawBox(boxPositionX, boxPositionY, boxWidth, boxHeight);
    resetColor();
    printCentered("TEAM MEMBERS", boxPositionY, terminalWidth);
    printCentered("1. Alea Farrel", boxPositionY + 3, terminalWidth);
    printCentered("2. Arif Wibowo P.", boxPositionY + 4, terminalWidth);
    printCentered("3. Aria Mahendra U.", boxPositionY + 5, terminalWidth);
    printCentered("4. Hensa Katelu", boxPositionY + 6, terminalWidth);
    printCentered("5. Yanuar Adi Candra", boxPositionY + 7, terminalWidth);
    
    printCentered("[ESC] BACK", boxPositionY + boxHeight + 2, terminalWidth);
}

/**
 * @brief Merender tabel riwayat permainan beserta kontrol navigasinya.
 * Merujuk secara spesifik pada referensi UI di `blueprint/History Menu Terisi dan Kosong.png`.
 * 
 * @param records Pointer ke array riwayat skor yang akan ditampilkan.
 * @param count Jumlah total data riwayat.
 * @param state Pointer ke status menu riwayat (halaman, sorting, pencarian).
 * @param fullRedraw Flag untuk merender ulang bingkai dasar menu riwayat saat pertama kali masuk.
 */
void renderHistoryMenu(ScoreRecord* records, int count, HistoryState* state, bool fullRedraw) {
    int terminalWidth, terminalHeight;
    getTerminalSize(terminalWidth, terminalHeight);
    
    if (fullRedraw) {
        clearScreen();
        setColor(COLOR_YELLOW);
        drawBox(1, 1, terminalWidth, terminalHeight);
        resetColor();
        
        moveCursorTo(3, 3);
        cout << "[ESC] BACK";
        
        setColor(COLOR_YELLOW);
        printCentered("HISTORY", 3, terminalWidth);
        resetColor();
        
        setColor(COLOR_YELLOW);
        drawBox(3, 5, terminalWidth - 4, 3);
        resetColor();
        
        string searchText = state->isSearchActive ? "[Q] Exit Search " : "[S] Quick Search";
        moveCursorTo(terminalWidth - searchText.length() - 3, 6);
        cout << searchText;
        
        // Merender header tabel
        setColor(COLOR_YELLOW);
        // Garis atas header
        drawHorizontalLine(2, 8, terminalWidth - 2);
        moveCursorTo(1, 8); cout << "+";
        moveCursorTo(terminalWidth, 8); cout << "+";
        
        // Garis bawah header
        drawHorizontalLine(2, 10, terminalWidth - 2);
        moveCursorTo(1, 10); cout << "+";
        moveCursorTo(terminalWidth, 10); cout << "+";
        
        // Garis tengah pembatas header
        moveCursorTo(terminalWidth / 2, 8); cout << "+";
        moveCursorTo(terminalWidth / 2, 9); cout << "|";
        moveCursorTo(terminalWidth / 2, 10); cout << "+";
        
        // Garis bawah tabel (footer)
        drawHorizontalLine(2, terminalHeight - 3, terminalWidth - 2);
        moveCursorTo(1, terminalHeight - 3); cout << "+";
        moveCursorTo(terminalWidth, terminalHeight - 3); cout << "+";
        moveCursorTo(terminalWidth / 2, terminalHeight - 3); cout << "+";
        resetColor();
        
        string scoreHeader = "SCORE (Points)";
        string timeHeader = "TIME (Seconds)";
        
        // Print header teks ke tengah kolom masing-masing
        printCentered(scoreHeader, 9, terminalWidth / 2);
        
        int rightHalfWidth = terminalWidth - (terminalWidth / 2) - 1;
        int timeX = (terminalWidth / 2) + 1 + (rightHalfWidth - timeHeader.length()) / 2 + 1;
        moveCursorTo(timeX, 9);
        cout << timeHeader;
        
        moveCursorTo(3, terminalHeight - 2);
        cout << "[P] PREV | [N] NEXT";
        
        string clearText = "[C] CLEAR";
        moveCursorTo(terminalWidth - clearText.length() - 2, terminalHeight - 2);
        cout << clearText;
    }
    
    // Merender baris data dinamis pada tabel
    string sortText = state->isAscending ? " [A] ASCENDING" : "[D] DESCENDING";
    moveCursorTo(terminalWidth - sortText.length() - 2, 3);
    cout << sortText;
    
    string searchText = state->isSearchActive ? "[Q] Exit Search " : "[S] Quick Search";
    moveCursorTo(terminalWidth - searchText.length() - 3, 6);
    setColor(COLOR_YELLOW);
    cout << searchText;
    resetColor();
    
    moveCursorTo(5, 6);
    cout << "Search: " << state->searchQuery << (state->searchQuery.empty() ? "_" : " ") << "                     ";
    
    int leftHalfWidth = terminalWidth / 2 - 2; // -2 karena border kiri dan tengah
    int rightHalfStart = terminalWidth / 2 + 1;
    int rightHalfWidth = terminalWidth - rightHalfStart - 1; // -1 karena border kanan
    
    // Bersihkan area data agar tidak flicker / tumpang tindih
    for (int y = 11; y < terminalHeight - 3; y++) {
        moveCursorTo(2, y);
        for(int c=0; c < leftHalfWidth; c++) cout << " ";
        
        moveCursorTo(rightHalfStart, y);
        for(int c=0; c < rightHalfWidth; c++) cout << " ";
    }
    
    if (count == 0) {
        printCentered("No history available.", terminalHeight / 2, terminalWidth);
    } else {
        int startIndex = state->currentPage * MAX_RECORDS_PER_PAGE;
        int endIndex = startIndex + MAX_RECORDS_PER_PAGE;
        if (endIndex > count) endIndex = count;
        
        // Kalkulasi jarak spasi vertikal antar baris record
        int availableHeight = (terminalHeight - 3) - 11;
        int spacing = availableHeight / MAX_RECORDS_PER_PAGE;
        if (spacing < 1) spacing = 1;
        
        int y = 11 + (spacing / 2); // Mulai agak ke tengah sel
        if (y < 12) y = 12;
        
        for (int i = startIndex; i < endIndex; i++) {
            if (i == state->cursorIndex) {
                moveCursorTo(3, y);
                setColor(COLOR_GREEN);
                cout << ">>";
            }
            resetColor();
            
            // Render Score rata tengah di kolom kiri
            string scoreStr = to_string(records[i].score) + " pts";
            int scoreX = 2 + (leftHalfWidth - scoreStr.length()) / 2;
            moveCursorTo(scoreX, y);
            cout << scoreStr;
            
            // Render Time rata tengah di kolom kanan
            string timeStr = to_string(records[i].playTimeInSeconds) + " s";
            int timeX = rightHalfStart + (rightHalfWidth - timeStr.length()) / 2;
            moveCursorTo(timeX, y);
            cout << timeStr;
            
            y += spacing;
        }
    }
    
    // Gambar ulang garis vertikal tengah pembatas tabel hanya jika ada data
    if (count > 0) {
        setColor(COLOR_YELLOW);
        for(int y = 11; y < terminalHeight - 3; y++) {
            moveCursorTo(terminalWidth / 2, y);
            cout << "|";
        }
        resetColor();
    }
    
    int totalPages = (count + MAX_RECORDS_PER_PAGE - 1) / MAX_RECORDS_PER_PAGE;
    if (totalPages == 0) totalPages = 1;
    string pageString = "Page " + to_string(state->currentPage + 1) + "/" + to_string(totalPages);
    int pageX = (terminalWidth - pageString.length()) / 2 + 1;
    moveCursorTo(pageX, terminalHeight - 2);
    cout << pageString << "    ";
}

/**
 * @brief Merender antarmuka detail statistik dari satu record riwayat spesifik.
 * 
 * @param record Pointer ke objek record riwayat yang dipilih.
 * @param fullRedraw Flag untuk merender ulang seluruh komponen layar.
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
    
    int statisticWidth = 32;
    int statisticHeight = 7;
    int statisticPositionX = (terminalWidth - statisticWidth) / 2 + 1;
    int statisticPositionY = (terminalHeight - statisticHeight) / 2;
    
    setColor(COLOR_YELLOW);
    drawBox(statisticPositionX, statisticPositionY, statisticWidth, statisticHeight);
    resetColor();
    printCentered("STATISTIC DETAIL", statisticPositionY, terminalWidth);
    
    string scoreString = "SCORE: " + to_string(record->score) + " Points";
    
    int minutes = record->playTimeInSeconds / 60;
    int seconds = record->playTimeInSeconds % 60;
    string timeString = "TIME: " + to_string(minutes) + "m " + to_string(seconds) + "s";
    
    printCentered(scoreString, statisticPositionY + 3, terminalWidth);
    printCentered(timeString, statisticPositionY + 4, terminalWidth);
    
    printCentered("[ESC] BACK", statisticPositionY + statisticHeight + 3, terminalWidth);
}

/**
 * @brief Merender layar konfirmasi penghapusan seluruh data riwayat permainan.
 * 
 * @param fullRedraw Flag untuk merender ulang seluruh komponen layar.
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
    printCentered("This action cannot be undone.", startY + 4, terminalWidth);
    
    string yesText = "[Y] Yes (Delete All)";
    string gapText = "        ";
    string noText = "[N] No (Cancel)";
    
    int totalLen = yesText.length() + gapText.length() + noText.length();
    int startX = (terminalWidth - totalLen) / 2 + 1;
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
 * @brief Merender tampilan spesifik saat tidak ada data riwayat yang tersedia.
 * 
 * @param fullRedraw Flag untuk merender ulang komponen.
 */
void renderEmptyHistory(bool fullRedraw) {
    // Standalone render jika diperlukan (belum diimplementasikan)
}
