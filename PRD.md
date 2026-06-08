# Project Requirements Document (PRD)

- **Nama Proyek:** TemiTik (Terminal Mengetik)
- **Paradigma:** Prosedural C++ (Non-OOP)
- **Tujuan:** Final Project Mata Kuliah Struktur Data
- **Prinsip Desain:** DRY (Don't Repeat Yourself), KISS (Keep It Simple, Stupid), Clean Code

## 1. Ringkasan Proyek

TemiTik adalah sebuah permainan ketik kata (typing game) berbasis Terminal User Interface (TUI) yang terinspirasi dari antarmuka modern CLI. Permainan ini menguji kecepatan dan ketepatan pengguna dalam mengetik kata-kata yang jatuh dari atas layar sebelum mencapai batas bawah (atau sebelum health/nyawa habis). Proyek ini dirancang secara modular menggunakan paradigma prosedural dan diwajibkan untuk mengimplementasikan serangkaian konsep Struktur Data.

## 2. Standar Penamaan (Clean Code Conventions)

Untuk memastikan kode mudah dibaca, dikelola, dan tidak bias, proyek ini secara ketat menerapkan aturan Clean Code untuk penamaan:

- **Bahasa Kode:** Semua nama `struct`, fungsi, dan variabel menggunakan Bahasa Inggris (standar industri).
- **Struct & Enum (PascalCase):** Contoh: `WordItem`, `ScoreRecord`, `PlayerState`, `GameState`.
- **Fungsi (camelCase - Kata Kerja + Kata Benda):** Fungsi harus merepresentasikan tepat satu tindakan. Contoh: `loadWordsFromFile()`, `renderMainMenu()`, `calculateScore()`. Tidak boleh ada fungsi seperti `doGameStuff()`.
- **Variabel (camelCase - Deskriptif):** Nama variabel tidak boleh disingkat jika menghilangkan makna. Contoh: gunakan `currentScore` alih-alih `cs`, `activeWord` alih-alih `aw`.
- **Konstanta (UPPER_SNAKE_CASE):** Contoh: `MAX_SCREEN_WIDTH`, `STARTING_HEALTH`.

## 3. Struktur Direktori Proyek

Karena proyek ini menggunakan CMake, struktur folder akan diatur secara rapi dan terpisah antara source code, header, dan berkas aset/data.

```text
TemiTik/
│
├── CMakeLists.txt           # File konfigurasi CMake untuk build otomatis
├── data/                    # Folder khusus file teks/resource statis
│   ├── wordBank.txt         # Kumpulan kata untuk permainan
│   └── historyData.txt      # Tempat penyimpanan riwayat skor pemain
│
├── include/                 # Folder khusus file Header (.h)
│   ├── dataStructs.h        # Pusat deklarasi Struct dan Enum
│   ├── gameEngine.h         
│   ├── visual.h             
│   ├── loader.h             
│   └── history.h            
│
└── src/                     # Folder khusus file implementasi (.cpp)
    ├── main.cpp             # Entry point dan Game Loop (State Machine)
    ├── gameEngine.cpp       # Logika utama permainan (gerak, ketikan, skor)
    ├── visual.cpp           # Rendering UI terminal (box, warna, layout)
    ├── loader.cpp           # Sistem I/O untuk memuat wordBank.txt
    └── history.cpp          # Sistem I/O history, sorting, dan searching
```

## 4. Pemetaan Syarat Struktur Data Wajib

Proyek ini mengintegrasikan seluruh syarat dari mata kuliah Struktur Data secara natural sesuai dengan blueprint visual yang dirancang.

### a. Struct
Digunakan sebagai tipe data bentukan untuk mengelompokkan variabel (di `dataStructs.h`).
- `struct WordItem`: Menyimpan properti kata yang sedang aktif (`string text`, `int xPosition`, `int yPosition`).
- `struct ScoreRecord`: Menyimpan data pemain setelah game over (`int score`, `int playTimeInSeconds`). Skor dihitung berdasarkan poin (setiap kata berhasil diketik = `POINTS_PER_WORD`, default 10 poin). Dibuat simpel tanpa nama pemain sesuai rancangan TUI.
- `struct PlayerState`: Menyimpan status terkini pemain (`int currentHealth`, `int currentScore`, `int levelSpeed`). Health awal adalah `STARTING_HEALTH` (3 nyawa). Kecepatan (`levelSpeed`) bertambah sedikit setiap kali pemain berhasil mengetik satu kata, bukan berdasarkan waktu yang berjalan.

### b. Array
- Digunakan di dalam `loader.cpp` / `gameEngine.cpp` sebagai buffer sementara saat memindahkan data dari file ke antrean.
- Digunakan di `history.cpp` (misal: `ScoreRecord historyRecords[100]`) untuk menampung riwayat data sebelum disortir dan ditampilkan dalam bentuk tabel ke layar TUI.

### c. Pointer
Digunakan secara ekstensif untuk efisiensi memori (Pass-by-Pointer) dan modifikasi nilai variabel di luar ruang lingkup (scope) fungsi tersebut.
- Contoh: `void updatePlayerScore(PlayerState* state, int pointsToAdd)` atau `void dequeueNextWord(Queue* wordQueue, WordItem* targetWord)`.
- Pointer juga akan digunakan untuk mengelola antrean (`Queue`) jika diimplementasikan menggunakan Linked List manual.

### d. Stack / Queue (Dipilih: Queue)
- **Implementasi:** Antrean Kata (`wordQueue`).
- **Logika:** Di dalam `gameEngine.cpp`, kata-kata yang dimuat dari `wordBank.txt` akan dimasukkan ke dalam `Queue`. Saat sebuah kata muncul di layar (jatuh), kata tersebut di-dequeue (`dequeueNextWord`). Jika kata tersebut berhasil diketik, pemain mendapat `POINTS_PER_WORD` poin dan kecepatan jatuh kata (`levelSpeed`) bertambah sedikit. Jika kata gagal (mengenai batas bawah), health berkurang satu. Kata berikutnya akan kembali di-dequeue. Prinsip First-In-First-Out (FIFO) ini adalah inti dari mekanik gameplay.

### e. Sorting
- **Implementasi:** Mengurutkan Tabel Riwayat (History Menu).
- **Logika:** Di `history.cpp`, ketika pengguna membuka layar **HISTORY MENU**, program akan memanggil fungsi `sortRecordsAscending()` atau `sortRecordsDescending()` (menggunakan Selection Sort atau Bubble Sort) untuk mengurutkan Array of Struct `ScoreRecord`.

### f. Searching
- **Implementasi:** Mencari Riwayat Berdasarkan Angka (Skor atau Waktu).
- **Logika:** Fitur Searching diaplikasikan dengan cara pengguna menekan tombol Search (`[S]`) lalu memasukkan angka pencarian. Sistem secara otomatis mencocokkan angka tersebut terhadap seluruh field numerik (skor dan waktu) dari setiap record tanpa pengaturan manual. Contoh: jika pengguna mencari angka `2`, maka semua record yang mengandung angka `2` baik di kolom skor maupun waktu akan ditampilkan. Secara internal, data di-sort terlebih dahulu lalu digunakan algoritma Binary Search untuk pencarian eksak pada kolom yang sedang aktif di-sort, serta dilengkapi filter pencocokan parsial untuk menangani pencarian lintas-kolom.

## 5. Arsitektur File & Fungsionalitas

### `main.cpp`
- Hanya memiliki `int main()`.
- Berisi State Machine (`int currentState`) yang mendelegasikan tugas ke modul spesifik (0 = Menu, 1 = Play, 2 = End, 3 = History, 4 = History Stats, 5 = Credits, 6 = Clear History Confirmation).

### `visual.h` & `visual.cpp`
- Fungsi spesifik manipulasi layar: `void clearScreen()`, `void moveCursorTo(int x, int y)`.
- Fungsi menggambar elemen TUI: `void drawBorder(int startX, int startY, int width, int height)`, `void renderHealthBar(int currentHealth)`, `void renderGameUI(PlayerState* state)`, `void renderClearHistoryConfirmation()`.

### `loader.h` & `loader.cpp`
- Fungsi spesifik File I/O baca: `bool loadWordsFromFile(string filePath, Queue* targetQueue)`.

### `gameEngine.h` & `gameEngine.cpp`
- Otak logika permainan: `void runGameLoop()`.
- Mengatur fisika dan input: `void calculateWordDrop(WordItem* word)`, `bool validatePlayerInput(string input, WordItem* activeWord)`.

### `history.h` & `history.cpp`
- Manajemen penyimpanan: `void saveRecordToFile(ScoreRecord newRecord)`, `void clearAllHistoryRecords()`.
- Manajemen data layar: `void renderHistoryTable(ScoreRecord* records, int count, int page)`.

## 6. Alur Navigasi (Berdasarkan Blueprint Flowchart)

Navigasi diimplementasikan menggunakan deteksi keyboard hit asinkron.

### START MENU
- **ENTER** -> Pindah ke GAME PLAY.
- **H** -> Pindah ke HISTORY MENU.
- **Q** -> EXIT (Keluar Aplikasi).

### GAME PLAY
- Gameplay berjalan terus (kata turun).
- **TAB** -> Restart game dari awal secara langsung (`resetGameState()`).
- **ESC** -> Keluar tanpa menyimpan (langsung START MENU).
- Jika `currentHealth == 0` -> Pindah ke END SCREEN.

### END SCREEN (GAME OVER)
- **ENTER** -> Simpan history skor & waktu secara otomatis (`saveRecordToFile()`), lalu pindah ke START MENU.
- **C** -> Pindah ke CREDITS SCREEN.

### HISTORY MENU (Tabel Data)
- Menampilkan daftar riwayat (Sorted). Default adalah DESC.
- **A & D** -> Untuk mengubah urutan menjadi ASC (`sortRecordsAscending()`) atau DESC (`sortRecordsDescending()`).
- **Up / Down** -> Navigasi kursor baris (`>`).
- **ENTER** pada baris terpilih -> Pindah ke HISTORY STATS (Detail baris tersebut).
- **N & P** -> Untuk Next dan Previous halaman (Pagination).
- **C** -> Pindah ke CLEAR HISTORY CONFIRMATION.
- **ESC** -> Kembali ke START MENU.

### CLEAR HISTORY CONFIRMATION
- Menampilkan konfirmasi peringatan penghapusan seluruh riwayat secara permanen.
- **Y** -> Menghapus seluruh riwayat permainan (`clearAllHistoryRecords()`) dan kembali ke HISTORY MENU.
- **N** -> Batal dan kembali ke HISTORY MENU.

### HISTORY STATS
- Tampilan detail 1 riwayat skor dan waktu.
- **ESC** -> Kembali ke HISTORY MENU.

### CREDITS SCREEN
- Menampilkan nama kreator.
- **ESC** -> Kembali ke END SCREEN.

---

> [!IMPORTANT]
> **Catatan Mutlak:** Dokumen ini merupakan acuan mutlak selama proses penulisan dan pengembangan kode.
