/**
 * @file loader.cpp
 * @brief Implementasi sistem pemuatan sumber daya (I/O).
 * 
 * Mengatur antarmuka interaksi dengan penyimpanan eksternal, mengekstrak,
 * memvalidasi, serta menyalurkan himpunan data awal ke dalam struktur data
 * operasional permainan.
 */

#include "loader.h"

// Alasan penggunaan <iostream>: Digunakan untuk standar input/output (sementara untuk stub/debugging).
#include <iostream>

// Alasan penggunaan <fstream>: Digunakan untuk operasi pembacaan file teks eksternal secara sekuensial. Tidak melanggar aturan karena hanya membaca aliran data, bukan menyediakan struktur data atau algoritma instan.
#include <fstream>

// Alasan penggunaan <string>: Diperlukan untuk menyimpan baris teks dari file secara dinamis tanpa batas statis yang berisiko buffer overflow.
#include <string>

using namespace std;

/**
 * @brief Menambahkan satu elemen kata ke bagian belakang antrean.
 * 
 * @param q Pointer ke antrean tujuan.
 * @param text Teks dari kata yang akan dimasukkan.
 */
static void enqueueWord(Queue* q, const string& rawText) {
    string text = rawText;
    if (!text.empty() && text.back() == '\r') {
        text.pop_back();
    }
    
    // Alokasi memori untuk node baru pada heap
    QueueNode* newNode = new QueueNode;
    newNode->data.text = text;
    newNode->data.xPosition = 0;
    newNode->data.yPosition = 0;
    newNode->data.isActive = false;
    newNode->next = nullptr;

    // Memeriksa apakah antrean dalam keadaan kosong
    if (q->rear == nullptr) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        // Menyambungkan node baru di akhir antrean
        q->rear->next = newNode;
        q->rear = newNode;
    }
    
    // Memperbarui jumlah total elemen
    q->count++;
}

bool loadWordsFromFile(string filePath, Queue* targetQueue) {
    // Mereset atau menginisialisasi nilai awal antrean
    targetQueue->front = nullptr;
    targetQueue->rear = nullptr;
    targetQueue->count = 0;

    // Membuka aliran file untuk pembacaan dengan pencarian jalur fallback
    ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        fileStream.open("../" + filePath);
        if (!fileStream.is_open()) {
            fileStream.open("../../" + filePath);
            if (!fileStream.is_open()) {
                return false;
            }
        }
    }

    string currentLine;
    bool isFirstLine = true;
    
    // Membaca karakter dari file baris per baris hingga akhir
    while (getline(fileStream, currentLine)) {
        if (isFirstLine) {
            // Hapus UTF-8 BOM jika file disimpan dengan encoding tersebut
            if (currentLine.size() >= 3 && 
                (unsigned char)currentLine[0] == 0xEF && 
                (unsigned char)currentLine[1] == 0xBB && 
                (unsigned char)currentLine[2] == 0xBF) {
                currentLine = currentLine.substr(3);
            }
            isFirstLine = false;
        }
        
        // Mengabaikan baris kosong untuk menghindari data tidak valid
        if (!currentLine.empty()) {
            enqueueWord(targetQueue, currentLine);
        }
    }

    // Menutup aliran file untuk membebaskan sumber daya
    fileStream.close();
    
    return true;
}
