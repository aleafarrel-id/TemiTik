# History Search — Sistem Pencarian Numerik

Dokumen ini menjelaskan desain, algoritma, dan keputusan arsitektur dari fitur pencarian (**Search**) pada [[Dokumentasi_TemiTik#6. Algoritma Sorting & Searching Wajib (history.cpp)|modul History]].

## Kontrak Input

Sesuai spesifikasi [[PRD#f. Searching|PRD bagian 3.f]], input pencarian **hanya menerima karakter digit (0–9)**. Alasan:

- Field yang dicocokkan (`score` dan `playTimeInSeconds`) adalah **murni numerik**.
- Input non-numerik tidak bermakna untuk pencocokan substring angka, dan hanya akan mengotori riwayat query.
- Pembatasan ini diterapkan di dua lapisan:
  1. **`main.cpp`** — hanya menerima `isdigit(ch)` dari keyboard.
  2. **`history.cpp`** — validasi defensif di dalam `filterHistoryRecords` menolak query non-numerik dan mengembalikan `0`.

## Format Tampilan Waktu

Sejak pembaruan ini, waktu di tabel History ditampilkan dalam format **`Xm Ys`** (bukan hanya detik mentah):

| Detik tersimpan | Tampilan di tabel |
|---|---|
| 0 | `0s` |
| 45 | `45s` |
| 90 | `1m 30s` |
| 150 | `2m 30s` |
| 3600 | `60m 0s` |

Konversi dilakukan oleh helper internal `formatTime(int totalSeconds)` di `visual.cpp`, dipakai secara konsisten di **History Menu**, **History Stats**, dan **End Screen**.

## Algoritma Dua Tahap

```
Input Query (digits only)
        │
        ▼
┌─────────────────────────────┐
│  Binary Search (O log n)    │  ← Mencari nilai eksak di kolom Score
│  Array sudah di-sort        │
│  Simpan foundIndex jika ada │
└────────────┬────────────────┘
             │ (foundIndex diakui; partial search mencakup kasusnya)
             ▼
┌──────────────────────────────────────────────────────────────────┐
│  Sequential Partial Match (O n × |query|)                        │
│  Loop manual tanpa string::find (pure manual loop)               │
│  Cocokkan query sebagai substring di EMPAT representasi:         │
│    1. scoreStr   = to_string(record.score)                       │
│    2. timeStr    = to_string(record.playTimeInSeconds)  ← raw    │
│    3. minuteStr  = to_string(record.playTimeInSeconds / 60)      │
│    4. secStr     = to_string(record.playTimeInSeconds % 60)      │
│  Jika foundInScore || foundInTime → masuk dest[]                 │
└──────────────────────────────────────────────────────────────────┘
```

Tiga representasi waktu (raw, menit, detik) memastikan pencarian konsisten dengan **apa yang terlihat di layar**. Contoh:

| User mengetik | Menemukan record | Karena |
|---|---|---|
| `2` | Record 150 detik (2m 30s) | minuteStr = "2" cocok |
| `30` | Record 150 detik (2m 30s) | secStr = "30" cocok |
| `150` | Record 150 detik | timeStr = "150" cocok |

> Binary Search dijalankan secara nyata (bukan dummy) dan menghasilkan `foundIndex` yang valid.
> Partial Search memastikan hasil mencakup seluruh kolom numerik sesuai PRD.

## Edge Cases yang Ditangani

| Edge Case | Penanganan |
|---|---|
| Query kosong | `filterHistoryRecords` mengembalikan salinan semua data (bypass filter) |
| Query non-numerik | `main.cpp` memblokir input; `history.cpp` mengembalikan `0` sebagai defense |
| ENTER ditekan saat search aktif | Diabaikan (tidak memicu transisi ke `HistoryStats`) |
| Query berubah (digit tambah/hapus) | `currentPage` dan `cursorIndex` di-reset ke `0` (mencegah out-of-bound) |
| Clear history saat search aktif | `searchQuery` dikosongkan, `isSearchActive` dimatikan, halaman dan kursor di-reset |
| `sourceCount == 0` | Loop partial match tidak berjalan, `destCount` tetap `0`, aman |
| Record 0 detik | `formatTime(0)` mengembalikan `"0s"` (cabang `minutes == 0`) |

## Alur Input di `main.cpp`

```cpp
if (historyState.isSearchActive) {
    int prevQueryLen = (int)historyState.searchQuery.length();

    if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
        historyState.isSearchActive = false;           // Tutup mode; filter tetap aktif
    } else if (ch == KEY_ENTER_WIN || ch == KEY_ENTER_NIX) {
        // Tidak ada aksi; mencegah transisi tidak sengaja ke HistoryStats
    } else if (ch == '\b') {
        if (!historyState.searchQuery.empty())
            historyState.searchQuery.pop_back();       // Hapus digit terakhir
    } else if (isdigit(ch)) {
        historyState.searchQuery += (char)ch;          // Tambah digit
    }

    // Reset halaman jika query berubah
    if ((int)historyState.searchQuery.length() != prevQueryLen) {
        historyState.currentPage = 0;
        historyState.cursorIndex = 0;
    }
}
```

## Hint UI di `visual.cpp`

Saat mode pencarian aktif, placeholder hilang dan kursor `|` muncul di ujung angka:

```
Search: 2|
```

Saat mode pencarian tidak aktif (tapi query masih tersimpan), nilai tetap tampil tanpa kursor:

```
Search: 2
```

Saat field kosong dan mode tidak aktif, placeholder magenta ditampilkan:

```
Search by number...
```

## Helper `formatTime` di `visual.cpp`

Fungsi internal `static string formatTime(int totalSeconds)` adalah satu-satunya titik konversi waktu ke teks. Semua layar menggunakan fungsi ini, **bukan** kalkulasi duplikat masing-masing:

```cpp
static string formatTime(int totalSeconds) {
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    if (minutes == 0) return to_string(seconds) + "s";
    return to_string(minutes) + "m " + to_string(seconds) + "s";
}
```

Dideklarasikan `static` agar tidak bocor ke unit kompilasi lain (enkapsulasi file-level).

## Keterkaitan Modul

- [[Dokumentasi_TemiTik]] — Dokumentasi utama
- [[PRD]] — Sumber spesifikasi asal
- [[PauseScreen]] — Dokumentasi fitur Pause
