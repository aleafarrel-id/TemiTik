/**
 * @file gameEngine.cpp
 * @brief Implementasi logika utama dan mekanika permainan.
 * 
 * Mengelola interaksi gameplay inti, kalkulasi dinamis posisi objek,
 * pemrosesan input dari pengguna secara real-time, perhitungan skor,
 * serta manajemen siklus hidup dan antrean elemen-elemen permainan.
 */

#include "gameEngine.h"
#include "visual.h"

// Alasan penggunaan <iostream>: Digunakan untuk merender teks kata yang jatuh ke terminal.
#include <iostream>
// Alasan penggunaan <windows.h>: Digunakan untuk GetTickCount64 dan Sleep guna mengontrol pergerakan kata secara asinkron tanpa memblokir input.
#include <windows.h>
// Alasan penggunaan <conio.h>: Digunakan untuk menangkap input keyboard secara real-time (_kbhit, _getch).
#include <conio.h>
// Alasan penggunaan <cstdlib> dan <ctime>: Digunakan untuk merandom letak posisi jatuhnya kata.
#include <cstdlib>
#include <ctime>

using namespace std;

// Konstanta Layout dan Batas Layar untuk Gameplay
const int BORDER_TOP_MARGIN = 2;       // Batas atas layar permainan (di bawah bingkai)
const int BORDER_BOTTOM_MARGIN = 5;    // Jarak dari bawah layar ke garis batas jatuhnya kata
const int BORDER_LEFT_MARGIN = 4;      // Batas aman kemunculan kata dari sisi kiri
const int TURRET_HEIGHT_OFFSET = 7;    // Tinggi moncong meriam laser relatif dari bawah terminal
const int TURRET_BASE_OFFSET = 5;      // Pangkal meriam laser menempel pada garis batas bawah

void calculateWordDrop(WordItem* word) {
    if (word->isActive) {
        word->yPosition++;
    }
}

bool validatePlayerInput(string input, WordItem* activeWord) {
    if (activeWord->text.length() >= input.length()) {
        return activeWord->text.substr(0, input.length()) == input;
    }
    return false;
}

void runGameLoop(PlayerState* playerState, GameState& currentState, Queue* wordQueue) {
    // Inisialisasi array kata aktif
    WordItem activeWords[MAX_ACTIVE_WORDS];
    for (int i = 0; i < MAX_ACTIVE_WORDS; i++) {
        activeWords[i].isActive = false;
    }

    int currentTermWidth, currentTermHeight;
    getTerminalSize(currentTermWidth, currentTermHeight);
    
    // Memberikan seed untuk generator angka acak
    srand((unsigned)time(0));
    
    int targetWordIndex = -1; // Indeks kata yang sedang di-auto-aim
    
    // Render awal bingkai permainan
    renderGameUI(playerState, true);
    
    // Menampilkan instruksi untuk memulai dan menunggu aksi pengguna
    setColor(32); // Hijau
    printCentered("Type anything to start", currentTermHeight / 3, currentTermWidth);
    resetColor();
    
    bool hasStarted = false;
    while (!hasStarted && currentState == Play) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 27) { // ESC saat game belum dimulai: langsung kembali ke Menu
                currentState = Menu;
                return;
            }
            hasStarted = true;
            
            // Menghapus tulisan "Type anything to start"
            moveCursorTo(2, currentTermHeight / 3);
            for(int k=0; k < currentTermWidth - 4; k++) cout << " ";
        } else {
            Sleep(ASYNC_INPUT_SLEEP_MS);
            
            // Memastikan antarmuka tidak rusak jika terminal diubah ukurannya saat menunggu
            int newTermWidth, newTermHeight;
            getTerminalSize(newTermWidth, newTermHeight);
            if (newTermWidth != currentTermWidth || newTermHeight != currentTermHeight) {
                currentTermWidth = newTermWidth;
                currentTermHeight = newTermHeight;
                renderGameUI(playerState, true);
                setColor(32);
                printCentered("Type anything to start", currentTermHeight / 3, currentTermWidth);
                resetColor();
            }
        }
    }
    
    // Set timer agar kata pertama langsung muncul saat permainan dimulai
    ULONGLONG lastDropTime = GetTickCount64() - WORD_SPAWN_INTERVAL_MS;
    ULONGLONG lastMoveTime = GetTickCount64();

    int lastTurretX = -1; // Menyimpan posisi X turret sebelumnya untuk dihapus dengan bersih

    while (currentState == Play) {
        // Deteksi resize terminal untuk mencegah kecurangan/bug visual
        int newTermWidth, newTermHeight;
        getTerminalSize(newTermWidth, newTermHeight);
        if (newTermWidth != currentTermWidth || newTermHeight != currentTermHeight) {
            currentTermWidth = newTermWidth;
            currentTermHeight = newTermHeight;
            renderGameUI(playerState, true); // Render ulang kerangka dasar
        }

        ULONGLONG currentTime = GetTickCount64();

        // Kecepatan jatuh (interval millisecond) berkurang seiring levelSpeed bertambah
        ULONGLONG moveInterval = MS_PER_SECOND / (playerState->levelSpeed > 0 ? playerState->levelSpeed : 1);
        if (moveInterval < MIN_DROP_INTERVAL_MS) moveInterval = MIN_DROP_INTERVAL_MS; // Batas maksimum kecepatan jatuh

        if (currentTime - lastMoveTime >= moveInterval) {
            lastMoveTime = currentTime;
            
            // Hapus render kata di posisi lama dengan mencetak spasi (alokasikan +4 untuk tanda indikator)
            for (int i = 0; i < MAX_ACTIVE_WORDS; i++) {
                if (activeWords[i].isActive) {
                    moveCursorTo(activeWords[i].xPosition - 2, activeWords[i].yPosition);
                    for (size_t c = 0; c < activeWords[i].text.length() + 4; c++) cout << " ";
                    
                    // Turunkan kata 1 baris ke bawah
                    calculateWordDrop(&activeWords[i]);
                    
                    // Periksa tabrakan dengan batas bawah
                    if (activeWords[i].yPosition >= currentTermHeight - BORDER_BOTTOM_MARGIN) {
                        activeWords[i].isActive = false;
                        playerState->currentHealth--;
                        
                        // Menghapus visual kata dari layar agar tidak menimpa garis bawah permanen
                        moveCursorTo(activeWords[i].xPosition - 2, activeWords[i].yPosition);
                        for (size_t c = 0; c < activeWords[i].text.length() + 4; c++) cout << " ";
                        
                        // Render ulang garis bawah secara manual yang rusak akibat crash
                        setColor(32); // Hijau (Warna bingkai)
                        moveCursorTo(2, currentTermHeight - BORDER_BOTTOM_MARGIN);
                        for (int k = 0; k < currentTermWidth - 2; k++) cout << "-";
                        resetColor();
                        
                        // Jika kata yang baru saja hancur adalah target yang sedang diketik, bebaskan target
                        if (i == targetWordIndex) {
                            targetWordIndex = -1;
                            playerState->currentInput = "";
                        }
                        
                        // Cek kondisi game over
                        if (playerState->currentHealth <= 0) {
                            currentState = End;
                        }
                    }
                }
            }
            
            // Render ulang kata di posisi barunya
            if (currentState == Play) {
                // Hapus turret lama agar tidak meninggalkan sisa sebelum menimpa dengan kata
                if (lastTurretX != -1) {
                    moveCursorTo(lastTurretX, currentTermHeight - TURRET_HEIGHT_OFFSET); cout << " ";
                    moveCursorTo(lastTurretX, currentTermHeight - TURRET_HEIGHT_OFFSET + 1); cout << " ";
                }
                
                for (int i = 0; i < MAX_ACTIVE_WORDS; i++) {
                    if (activeWords[i].isActive) {
                        // Mencegah kata tergambar di luar batas jika terminal diperkecil mendadak
                        if (activeWords[i].xPosition > currentTermWidth - activeWords[i].text.length() - BORDER_LEFT_MARGIN) {
                            activeWords[i].xPosition = currentTermWidth - activeWords[i].text.length() - BORDER_LEFT_MARGIN;
                        }
                        if (activeWords[i].xPosition < BORDER_LEFT_MARGIN) activeWords[i].xPosition = BORDER_LEFT_MARGIN;
                        
                        moveCursorTo(activeWords[i].xPosition - 2, activeWords[i].yPosition);
                        
                        // Logika Auto-Aim & Pewarnaan: Hijau jika benar, Merah jika salah
                        if (i == targetWordIndex && playerState->currentInput.length() > 0) {
                            setColor(33); // Warna Kuning untuk indikator bidikan
                            cout << "> ";
                            for (size_t c = 0; c < activeWords[i].text.length(); c++) {
                                if (c < playerState->currentInput.length()) {
                                    if (playerState->currentInput[c] == activeWords[i].text[c]) {
                                        setColor(32); // Warna Hijau (Benar)
                                    } else {
                                        setColor(31); // Warna Merah (Salah ketik)
                                    }
                                } else {
                                    resetColor(); // Kembalikan ke warna default
                                }
                                cout << activeWords[i].text[c];
                            }
                            setColor(33); // Warna Kuning
                            cout << " <";
                            resetColor();
                        } else {
                            // Berikan 2 spasi kosong sebagai bantalan pengganti indikator target
                            cout << "  " << activeWords[i].text << "  ";
                        }
                    }
                }
                // Memperbarui UI barisan bawah (Skor, Nyawa, Input) agar selalu berada paling depan
                renderGameUI(playerState, false);
                
                // Render ulang garis bawah dan turret penembak
                moveCursorTo(2, currentTermHeight - BORDER_BOTTOM_MARGIN);
                setColor(32); // Hijau
                for (int k = 0; k < currentTermWidth - 2; k++) cout << "-";
                resetColor();
                
                if (targetWordIndex != -1) {
                    int turretX = activeWords[targetWordIndex].xPosition + (activeWords[targetWordIndex].text.length() / 2);
                    lastTurretX = turretX; // Simpan posisi terbaru
                    
                    setColor(36); // Warna Cyan untuk moncong senjata
                    moveCursorTo(turretX, currentTermHeight - TURRET_HEIGHT_OFFSET);
                    cout << "^";
                    moveCursorTo(turretX, currentTermHeight - TURRET_HEIGHT_OFFSET + 1);
                    cout << "|";
                    moveCursorTo(turretX, currentTermHeight - TURRET_BASE_OFFSET);
                    cout << "|"; // Menimpa garis batas tepat di pangkal
                    resetColor();
                } else {
                    lastTurretX = -1;
                }
            }
        }

        // Keluar dari loop jika state sudah berubah (game over atau keluar ke Menu)
        if (currentState != Play) break;

        // Mekanisme memunculkan kata baru dari antrean secara berkala
        if (currentTime - lastDropTime >= WORD_SPAWN_INTERVAL_MS) {
            // Cari slot aktif yang kosong
            int freeSlot = -1;
            for (int i = 0; i < MAX_ACTIVE_WORDS; i++) {
                if (!activeWords[i].isActive) {
                    freeSlot = i;
                    break;
                }
            }
            
            if (freeSlot != -1 && wordQueue->count > 0) {
                // Peek antrean terdepan untuk menghitung lebar tanpa dequeue dulu
                string nextText = wordQueue->front->data.text;
                // Mengatur jangkauan nilai random kemunculan kata agar tidak menabrak tembok kanan
                int maxPosX = currentTermWidth - nextText.length() - (BORDER_LEFT_MARGIN * 2); 
                if (maxPosX < 1) maxPosX = 1;
                
                int newX = BORDER_LEFT_MARGIN;
                bool overlap = true;
                int attempts = 0;
                
                // Coba temukan koordinat X yang tidak bertumpuk dengan kata lain
                while (overlap && attempts < MAX_SPAWN_ATTEMPTS) {
                    newX = BORDER_LEFT_MARGIN + (rand() % maxPosX);
                    overlap = false;
                    for (int j = 0; j < MAX_ACTIVE_WORDS; j++) {
                        // Jika ada kata lain di area atas layar (Y < 6)
                        if (activeWords[j].isActive && activeWords[j].yPosition < 6) {
                            int leftA = newX - 1; 
                            int rightA = newX + nextText.length() + 1;
                            int leftB = activeWords[j].xPosition;
                            int rightB = activeWords[j].xPosition + activeWords[j].text.length();
                            
                            // Logika tabrakan AABB horizontal
                            if (!(rightA < leftB || leftA > rightB)) {
                                overlap = true;
                                break;
                            }
                        }
                    }
                    attempts++;
                }
                
                if (!overlap) {
                    // Posisi aman, lakukan Dequeue secara manual prosedural
                    lastDropTime = currentTime;
                    
                    QueueNode* temp = wordQueue->front;
                    activeWords[freeSlot] = temp->data;
                    
                    wordQueue->front = wordQueue->front->next;
                    if (wordQueue->front == nullptr) wordQueue->rear = nullptr;
                    wordQueue->count--;
                    delete temp;

                    activeWords[freeSlot].isActive = true;
                    activeWords[freeSlot].yPosition = BORDER_TOP_MARGIN; // Batas tepat di bawah bingkai atas
                    activeWords[freeSlot].xPosition = newX;
                } else {
                    // Layar atas penuh (terlalu berdekatan), tunda spawn sejenak agar tidak nabrak
                    lastDropTime = currentTime - (WORD_SPAWN_INTERVAL_MS - COLLISION_RETRY_DELAY_MS);
                }
            }
        }

        // Penanganan Input secara asinkron
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 27) { // ESC saat game aktif: masuk ke layar Pause
                currentState = Pause;

                // Tampilkan overlay pause di atas layar game yang sudah ada
                renderPauseScreen(true);

                // Inner loop: blokir update game selama pause aktif
                bool pauseResolved = false;
                while (!pauseResolved) {
                    if (_kbhit()) {
                        int pauseCh = _getch();
                        if (pauseCh == '\r' || pauseCh == '\n') {
                            // ENTER: lanjutkan permainan; gambar ulang layar game penuh
                            currentState = Play;
                            renderGameUI(playerState, true);
                            // Gambar ulang semua kata yang sedang aktif di posisi mereka
                            for (int i = 0; i < MAX_ACTIVE_WORDS; i++) {
                                if (activeWords[i].isActive) {
                                    moveCursorTo(activeWords[i].xPosition - 2, activeWords[i].yPosition);
                                    cout << "  " << activeWords[i].text << "  ";
                                }
                            }
                            // Perbarui timer agar kata tidak langsung jatuh setelah resume
                            lastMoveTime = GetTickCount64();
                            lastDropTime = GetTickCount64() - (WORD_SPAWN_INTERVAL_MS / 2);
                            pauseResolved = true;
                        } else if (pauseCh == 'q' || pauseCh == 'Q') {
                            // Q: batalkan sesi dan kembali ke Menu utama
                            currentState = Menu;
                            pauseResolved = true;
                        }
                    } else {
                        Sleep(ASYNC_INPUT_SLEEP_MS);
                    }
                }
            } else if (ch == '\t') { // TAB: Restart permainan secara instan
                // Hapus semua kata aktif dari layar
                for(int i=0; i<MAX_ACTIVE_WORDS; i++) {
                    if (activeWords[i].isActive) {
                        moveCursorTo(activeWords[i].xPosition - 2, activeWords[i].yPosition);
                        for (size_t c = 0; c < activeWords[i].text.length() + 4; c++) cout << " ";
                        activeWords[i].isActive = false;
                    }
                }
                if (lastTurretX != -1) {
                    moveCursorTo(lastTurretX, currentTermHeight - TURRET_HEIGHT_OFFSET); cout << " ";
                    moveCursorTo(lastTurretX, currentTermHeight - TURRET_HEIGHT_OFFSET + 1); cout << " ";
                    lastTurretX = -1;
                }
                // Reset statistik pemain
                playerState->currentHealth = STARTING_HEALTH;
                playerState->currentScore = 0;
                playerState->levelSpeed = INITIAL_DROP_SPEED;
                playerState->currentInput = "";
                targetWordIndex = -1;
                // Render ulang seluruh layar permainan
                renderGameUI(playerState, true);
                
                // Menampilkan kembali instruksi mulai
                setColor(32);
                printCentered("Type anything to start", currentTermHeight / 3, currentTermWidth);
                resetColor();
                
                // Kembali menunggu input dari pengguna untuk merestart
                hasStarted = false;
                while (!hasStarted && currentState == Play) {
                    if (_kbhit()) {
                        int startCh = _getch();
                        if (startCh == 27) { currentState = Menu; return; }
                        hasStarted = true;
                        moveCursorTo(2, currentTermHeight / 3);
                        for(int k=0; k < currentTermWidth - 4; k++) cout << " ";
                    } else {
                        Sleep(ASYNC_INPUT_SLEEP_MS);
                    }
                }
                lastDropTime = GetTickCount64() - WORD_SPAWN_INTERVAL_MS;
                lastMoveTime = GetTickCount64();
                continue; // Lanjutkan loop dengan status segar
            } else if (ch == '\b') { // Backspace: Menghapus 1 karakter input
                if (!playerState->currentInput.empty()) {
                    playerState->currentInput.pop_back();
                    if (playerState->currentInput.empty()) {
                        targetWordIndex = -1; // Bebaskan target jika input kosong
                    } else {
                        // Re-evaluasi target ke ancaman paling berbahaya (Y paling besar)
                        int bestTarget = -1;
                        int maxY = -1;
                        for (int i = 0; i < MAX_ACTIVE_WORDS; i++) {
                            if (activeWords[i].isActive && activeWords[i].text.find(playerState->currentInput) == 0) {
                                if (activeWords[i].yPosition > maxY) {
                                    maxY = activeWords[i].yPosition;
                                    bestTarget = i;
                                }
                            }
                        }
                        if (bestTarget != -1) targetWordIndex = bestTarget;
                    }
                }
            } else if (ch == '\r' || ch == '\n') {
                // Abaikan tombol ENTER secara eksplisit agar tidak masuk ke buffer input dan merusak garis kursor terminal
            } else if (isalnum(ch) || ispunct(ch) || ch == ' ') { 
                if (ch == ' ' && playerState->currentInput.empty()) {
                    // Abaikan spasi jika input masih kosong untuk mencegah bug accidental space
                } else {
                    // Rekam input yang valid
                    playerState->currentInput += (char)ch;
                    
                    // Logika Auto-Aim Berbasis Ancaman (Memprioritaskan Y Terbesar / Paling Bawah)
                    int bestTarget = -1;
                    int maxY = -1;
                    for (int i = 0; i < MAX_ACTIVE_WORDS; i++) {
                        if (activeWords[i].isActive && activeWords[i].text.find(playerState->currentInput) == 0) {
                            if (activeWords[i].yPosition > maxY) {
                                maxY = activeWords[i].yPosition;
                                bestTarget = i;
                            }
                        }
                    }
                    
                    // Update target jika ada kecocokan, biarkan pada target lama jika murni typo
                    if (bestTarget != -1) {
                        targetWordIndex = bestTarget;
                    }
                    
                    // Periksa apakah target yang sedang dikunci sudah diketik dengan tuntas
                    if (targetWordIndex != -1 && playerState->currentInput == activeWords[targetWordIndex].text) {
                        int targetX = activeWords[targetWordIndex].xPosition + (activeWords[targetWordIndex].text.length() / 2);
                        int startY = currentTermHeight - TURRET_BASE_OFFSET;
                        int endY = activeWords[targetWordIndex].yPosition;
                        
                        // Efek Animasi Laser Tembakan (Garis Vertikal)
                        setColor(36); // Warna Cyan untuk peluru laser
                        for (int y = startY; y > endY; y--) {
                            moveCursorTo(targetX, y);
                            cout << "|";
                        }
                        resetColor();
                        Sleep(LASER_ANIMATION_DELAY_MS); // Jeda sangat singkat untuk efek kilat
                        
                        // Bersihkan jalur laser
                        for (int y = startY; y > endY; y--) {
                            moveCursorTo(targetX, y);
                            cout << " ";
                        }
                        
                        // Hapus visual kata dari layar (Sukses)
                        moveCursorTo(activeWords[targetWordIndex].xPosition - 2, activeWords[targetWordIndex].yPosition);
                        for (size_t c = 0; c < activeWords[targetWordIndex].text.length() + 4; c++) cout << " ";
                        
                        activeWords[targetWordIndex].isActive = false;
                        playerState->currentScore += POINTS_PER_WORD;
                        
                        // Menambah kecepatan jatuh bertahap
                        playerState->levelSpeed = INITIAL_DROP_SPEED + (playerState->currentScore / SCORE_DIVISOR_FOR_SPEED);
                        
                        playerState->currentInput = "";
                        targetWordIndex = -1; // Bebaskan target
                    }
                }
            }
            // Segera perbarui layar bagian input pengguna
            renderGameUI(playerState, false); 
            
            // Render ulang warna kata secara instan untuk menghilangkan latensi visual (Zero Latency Fix)
            if (currentState == Play) {
                // Hapus turret lama saat terjadi perubahan ketikan
                if (lastTurretX != -1) {
                    moveCursorTo(lastTurretX, currentTermHeight - TURRET_HEIGHT_OFFSET); cout << " ";
                    moveCursorTo(lastTurretX, currentTermHeight - TURRET_HEIGHT_OFFSET + 1); cout << " ";
                }
                
                for (int i = 0; i < MAX_ACTIVE_WORDS; i++) {
                    if (activeWords[i].isActive) {
                        moveCursorTo(activeWords[i].xPosition - 2, activeWords[i].yPosition);
                        if (i == targetWordIndex && playerState->currentInput.length() > 0) {
                            setColor(33); // Warna Kuning untuk indikator
                            cout << "> ";
                            for (size_t c = 0; c < activeWords[i].text.length(); c++) {
                                if (c < playerState->currentInput.length()) {
                                    if (playerState->currentInput[c] == activeWords[i].text[c]) {
                                        setColor(32); // Hijau
                                    } else {
                                        setColor(31); // Merah
                                    }
                                } else {
                                    resetColor();
                                }
                                cout << activeWords[i].text[c];
                            }
                            setColor(33); // Warna Kuning
                            cout << " <";
                            resetColor();
                        } else {
                            cout << "  " << activeWords[i].text << "  ";
                        }
                    }
                }
                
                // Render ulang garis bawah dan turret (Sinkronisasi kilat dengan ketikan pengguna)
                moveCursorTo(2, currentTermHeight - BORDER_BOTTOM_MARGIN);
                setColor(32); // Hijau
                for (int k = 0; k < currentTermWidth - 2; k++) cout << "-";
                resetColor();
                
                if (targetWordIndex != -1) {
                    int turretX = activeWords[targetWordIndex].xPosition + (activeWords[targetWordIndex].text.length() / 2);
                    lastTurretX = turretX; // Simpan posisi terbaru
                    
                    setColor(36); // Warna Cyan untuk moncong senjata
                    moveCursorTo(turretX, currentTermHeight - TURRET_HEIGHT_OFFSET);
                    cout << "^";
                    moveCursorTo(turretX, currentTermHeight - TURRET_HEIGHT_OFFSET + 1);
                    cout << "|";
                    moveCursorTo(turretX, currentTermHeight - TURRET_BASE_OFFSET);
                    cout << "|"; // Menimpa garis batas
                    resetColor();
                } else {
                    lastTurretX = -1;
                }
            }
        }

        // Delay sangat singkat untuk menghindari lonjakan penggunaan CPU 100%
        Sleep(MAIN_LOOP_DELAY_MS); 
    }
}
