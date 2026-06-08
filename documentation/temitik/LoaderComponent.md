# Loader Component

## Ringkasan
Komponen Loader (`loader.cpp` dan `loader.h`) bertanggung jawab untuk membaca data teks dari penyimpanan eksternal (terutama `data/wordBank.txt`) dan memuatnya ke dalam memori permainan.

## Struktur Data yang Digunakan
- **[[Queue]]**: Digunakan untuk menampung kata-kata yang telah dibaca dari file eksternal sebelum diproses oleh [[GameEngine]].
- **[[QueueNode]]**: Simpul untuk menyimpan teks dari setiap baris data yang akan dialokasikan secara manual ke dalam heap.
- **[[WordItem]]**: Struktur yang menyimpan kata berserta posisi dan statusnya, disematkan pada tiap node dalam Queue.

## Implementasi Prosedural
Mengikuti persyaratan pada [[PRD]], tidak ada pustaka STL seperti `<queue>` atau `<vector>` yang digunakan. Pemuatan data membaca file baris demi baris menggunakan `std::ifstream`, lalu mengalokasikan node secara dinamis menggunakan `new` dan merangkainya menggunakan operasi pointer dasar untuk membentuk struktur data *Linked List Queue*.

### Fungsi Utama
- `loadWordsFromFile(string filePath, Queue* targetQueue)`: Fungsi I/O utama yang memuat data kata secara sekuensial dan memanggil `enqueueWord()` secara internal. Menangani skenario di mana antrean perlu di-reset dengan mengatur `front` dan `rear` menjadi `nullptr`.
- `loadHistoryRecords(string filePath, ScoreRecord* records)`: Membaca file `historyData.txt` secara terstruktur dengan separator spasi, menyisipkannya langsung ke Array `ScoreRecord`.
## Penanganan Pengecualian dan Keamanan
- Digunakan pemeriksaan `if (!fileStream.is_open())` untuk mengantisipasi file yang tidak ditemukan atau tidak dapat diakses.
- Pengecekan string kosong `!currentLine.empty()` dilakukan untuk mencegah baris kosong ikut dimuat ke dalam antrean permainan. 
- Penggunaan library `#include <fstream>` dan `#include <string>` telah diberikan penjelasan memadai dalam komentar baris sesuai standar koding yang ditetapkan.
