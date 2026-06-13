# Pause Screen — Sistem Jeda Permainan

Dokumen ini menjelaskan desain, alur implementasi, dan keputusan arsitektur dari fitur layar Pause yang ditambahkan ke [[Dokumentasi_TemiTik]].

## Motivasi

Sebelum fitur ini, menekan ESC saat bermain langsung memindahkan pemain ke Main Menu tanpa konfirmasi. Progress permainan (posisi kata, skor, nyawa) hilang sepenuhnya. Fitur Pause memberikan titik istirahat yang aman: game beku, pemain bisa memilih untuk lanjut atau benar-benar keluar.

## Alur State

```
GAME PLAY (Play)
    │
    │  [ESC]
    ▼
┌─────────────────────────────┐
│         PAUSE SCREEN        │
│                             │
│   [ENTER] Continue          │ ──── kembali ke Play (semua kata tetap di posisi masing-masing)
│   [Q]     Quit to Menu      │ ──── pindah ke Menu (progress hilang)
└─────────────────────────────┘
```

> ESC di layar "Type anything to start" (sebelum game benar-benar dimulai) **tetap langsung ke Menu** — wajar karena belum ada progress yang perlu dilindungi.

## Keputusan Arsitektur

### Pause dihandle di dalam `gameEngine.cpp` (bukan di `main.cpp`)

Pilihan ini disengaja agar **state `Pause` tidak bocor ke state machine `main.cpp`**. Seluruh siklus pause–resume terjadi sebagai **inner blocking loop** di dalam `runGameLoop`:

```
while (currentState == Play) {          ← loop utama game
    ...
    if (ESC ditekan) {
        currentState = Pause;
        renderPauseScreen(true);

        while (!pauseResolved) {        ← inner loop — game beku di sini
            if (ENTER) {
                currentState = Play;
                render ulang layar game;
                sesuaikan timer;
                pauseResolved = true;
            } else if (Q) {
                currentState = Menu;
                pauseResolved = true;
            }
        }
    }
    ...
}
```

Keuntungan pendekatan ini:
- `main.cpp` tidak perlu mengetahui state `Pause` sama sekali.
- Semua variabel lokal `runGameLoop` (`activeWords[]`, `lastTurretX`, `targetWordIndex`) tetap hidup di stack selama pause, sehingga resume benar-benar melanjutkan dari kondisi yang sama.
- Timer (`lastMoveTime`, `lastDropTime`) disesuaikan ulang setelah resume agar kata tidak tiba-tiba melompat jauh ke bawah akibat waktu pause yang terakumulasi.

## Komponen yang Diubah

| File | Perubahan |
|---|---|
| `dataStructs.h` | Tambah `Pause = 7` ke `enum GameState` |
| `gameEngine.cpp` | ESC di loop aktif → inner pause loop; ESC di layar awal tetap ke Menu |
| `visual.cpp` | Implementasi `renderPauseScreen()` — overlay tanpa clearScreen |
| `visual.h` | Deklarasi `renderPauseScreen()` dengan Doxygen lengkap |

## Desain Visual Overlay

`renderPauseScreen` **tidak memanggil `clearScreen()`**. Ini disengaja:
- Tampilan game yang sedang berjalan tetap terlihat di belakang kotak dialog.
- Pemain bisa melihat kondisi layar sebelum memutuskan apakah akan lanjut atau keluar.

```
┌────────────────────────────────────┐
│          || PAUSED ||              │  ← border cyan
│                                    │
│       Game is paused.              │
│                                    │
│    [ENTER] Continue                │  ← hijau
│    [Q]     Quit to Menu            │  ← merah
│                                    │
│                                    │
└────────────────────────────────────┘
```

## Perilaku Resume

Saat `ENTER` ditekan di layar Pause:
1. `currentState` dikembalikan ke `Play`.
2. `renderGameUI(playerState, true)` menggambar ulang seluruh bingkai game.
3. Semua kata aktif digambar ulang di koordinat `yPosition` mereka yang sudah tersimpan.
4. `lastMoveTime` di-reset ke waktu sekarang → kata tidak langsung jatuh.
5. `lastDropTime` disetel ke `now - WORD_SPAWN_INTERVAL_MS/2` → kata berikutnya muncul dalam ~1,5 detik (bukan langsung).

## Keterkaitan Modul

- [[Dokumentasi_TemiTik]] — Dokumentasi utama
- [[PRD]] — Alur navigasi acuan (seksi 6: GAME PLAY)
