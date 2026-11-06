# Test Bring-Up — Pakan Ikan Otomatis

Sketch untuk mengetes komponen yang sudah terpasang di breadboard:

- ESP32 Dev Module
- OLED SSD1306 0.96" (I2C)
- DS18B20 waterproof module
- TDS Meter V1.0 (analog)
- Servo SG90
- Tombol
- Buzzer pasif
- Kapasitor 1000 µF pada jalur servo

**VL53L0X belum diuji** karena belum disolder. Nanti tinggal tambah sketch terpisah atau tambahkan library VL53L0X.

## 1. Library yang perlu dipasang

Di Arduino IDE → Library Manager:

| Library | Keterangan |
|---|---|
| `Adafruit SSD1306` | plus dependency `Adafruit GFX Library` dan `Adafruit BusIO` |
| `OneWire` | untuk DS18B20 |
| `DallasTemperature` | untuk DS18B20 |
| `ESP32Servo` | untuk SG90 |

Board package: **ESP32 by Espressif Systems**. Sketch ini sudah disesuaikan untuk **core 2.0.x maupun core 3.x**: API LEDC lama (`ledcSetup`/`ledcAttachPin`) dan API baru (`ledcAttach`) dipilih otomatis lewat preprocessor. Sudah divalidasi compile dengan **core 3.3.12**.

Kalau memakai core 3.x, pastikan library `ESP32Servo` juga versi terbaru agar kompatibel.

## 2. Setting Arduino IDE

- Board: **ESP32 Dev Module**
- Upload Speed: 921600 (kalau gagal, turunkan ke 115200)
- Flash Frequency: 80 MHz
- PSRAM: Disabled
- Partition Scheme: Default
- Port: port ESP32 kamu
- Serial Monitor: **115200 baud**

## 3. Wiring

| Komponen | Pin komponen | Ke ESP32/rail |
|---|---|---|
| OLED | VCC | 3.3 V |
| OLED | GND | GND |
| OLED | SDA | GPIO21 |
| OLED | SCL | GPIO22 |
| DS18B20 | VCC | 3.3 V |
| DS18B20 | GND | GND |
| DS18B20 | DATA | GPIO4 |
| TDS | VCC | 3.3 V (tes awal) |
| TDS | GND | GND |
| TDS | AOUT | GPIO34 |
| SG90 | Merah | 5 V |
| SG90 | Cokelat/hitam | GND |
| SG90 | Oranye/kuning | GPIO18 |
| Tombol | Kaki 1 | GPIO27 |
| Tombol | Kaki 2 | GND |
| Buzzer | + | GPIO25 |
| Buzzer | − | GND |
| Kapasitor 1000 µF | + | 5 V dekat servo |
| Kapasitor | − | GND dekat servo |

Catatan:

- Tombol 4 kaki: pakai dua kaki diagonal. Satu ke GPIO27, satu ke GND.
- Buzzer magnetic: tambahkan resistor 220 Ω seri. Buzzer piezo biasanya bisa langsung.
- Jangan power servo dari 3.3 V.
- Jangan hubungkan AOUT TDS ke 5 V.
- Kalau DS18B20 module tidak punya pull-up, pasang dua resistor 10 kΩ paralel (= 5 kΩ) antara DATA dan 3.3 V.

## 4. Upload dan buka Serial Monitor

1. Sambungkan ESP32 ke adaptor 5 V 3 A via USB.
2. Upload sketch.
3. Buka Serial Monitor 115200.
4. Tekan tombol `EN`/`RST` di ESP32 kalau perlu.

Output awal yang diharapkan:

```text
==========================================
  TEST BRING-UP PAKAN IKAN OTOMATIS
  ESP32 + OLED + DS18B20 + TDS + SG90
  Tombol + buzzer
==========================================
VL53L0X belum diuji di sketch ini.

Scan I2C...
  Ditemukan 0x3C
  Total: 1 perangkat.
OLED: OK di 0x3C.
DS18B20: 1 device terdeteksi.
Servo: belum di-attach (tekan 's' untuk tes).
...
```

Alamat OLED bisa 0x3C atau 0x3D. Sketch mencoba keduanya. Servo tidak bergerak saat boot; gerakan hanya terjadi setelah perintah `s`, `f`, atau `x`.

## 5. Perintah Serial

Ketik huruf lalu Enter:

| Perintah | Fungsi |
|---|---|
| `h` | tampilkan help |
| `i` | scan ulang I2C |
| `d` | baca DS18B20 sekarang |
| `t` | baca TDS sekarang |
| `b` | tes buzzer 3 beep |
| `s` | servo 0° → 90° → 0° (aman untuk mekanisme) |
| `x` | servo 0° → 180° → 0° (lepas mekanisme dulu) |
| `f` | feed test: buzzer ±2 detik + servo 0° → 90° → 0° |
| `m` | print semua status |
| `o` | refresh OLED |

## 6. Kalibrasi sudut servo

Di bagian atas sketch:

```cpp
const int SERVO_HOME_DEG = 0;
const int SERVO_FEED_DEG = 90;
```

- `SERVO_HOME_DEG` adalah posisi rail saat menerima pakan dari corong.
- `SERVO_FEED_DEG` adalah posisi rail saat pakan jatuh ke akuarium.
- Kalau gerakan rail terlalu pendek/panjang, ubah `SERVO_FEED_DEG`.
- Pastikan rail tidak menabrak dinding corong di kedua posisi.
- Mulai dari `s` (0 → 90 → 0). Jangan pakai `x` saat mekanisme terpasang.

## 7. Urutan tes yang disarankan

1. **OLED**: cek alamat I2C, pastikan tampilan muncul.
2. **DS18B20**: tekan `d`; cek suhu wajar. Kalau `--` atau -127, cek wiring/pull-up.
3. **TDS**: tekan `t`; masukkan probe ke air. Cek nilai mV berubah. Jangan berharap ppm akurat tanpa kalibrasi.
4. **Tombol**: tekan dan tahan >800 ms; Serial harus menampilkan long press dan feed test.
5. **Buzzer**: tekan `b`.
6. **Servo**: tekan `s`. Servo tidak bergerak saat boot; gerakan pertama baru terjadi setelah `s`. Kalau mekanisme terpasang, jangan tekan `x`.
7. **Feed test**: tekan `f`; pastikan buzzer berbunyi sebelum servo bergerak.
8. **Power test**: jalankan `f` beberapa kali; pastikan ESP32 tidak reset.

## 8. Troubleshooting

### OLED tidak terdeteksi
- Cek VCC 3.3 V dan GND.
- Cek SDA/SCL tidak tertukar.
- Coba alamat 0x3D (sketch sudah mencoba).
- Pastikan modul OLED memang I2C, bukan SPI.

### DS18B20 membaca -127 °C
- Cek VCC, GND, DATA.
- Cek pull-up 4.7 kΩ di module. Kalau tidak ada, dua 10 kΩ paralel.
- Coba kabel lebih pendek.

### TDS selalu 0 atau mentok
- Cek AOUT ke GPIO34.
- Cek VCC/GND module.
- Pastikan probe tercelup, bukan board module.
- Coba pindah VCC TDS ke 5 V **hanya jika** output module memang 0–2.3 V. Jangan sampai AOUT >3.3 V.
- Tanpa larutan kalibrasi, nilai ppm hanya estimasi.

### Servo tidak bergerak sama sekali
- Tekan `s`, lalu cek Serial Monitor. Harus muncul:
  ```text
  Servo: signal GPIO18, attach channel=0, attached=YA
  Servo: PWM freq=50 Hz (harus 50 Hz)
  ```
- Kalau `attached=TIDAK`, cek kabel signal servo ke **GPIO18**.
- Kalau `PWM freq` bukan 50 Hz (misalnya 2000 Hz), berarti channel LEDC bentrok dengan buzzer. Sketch terbaru sudah memakai channel 15 untuk buzzer dan melepasnya setelah beep; upload ulang sketch terbaru.
- Kalau sudah 50 Hz tetapi servo tetap diam:
  - Cek urutan kabel servo: **cokelat/hitam = GND**, **merah = 5 V**, **oranye/kuning = signal GPIO18**.
  - Pastikan kabel merah ke **5 V**, bukan 3.3 V.
  - Pastikan GND servo dan GND ESP32 tersambung (common ground).
  - Cek polaritas kapasitor 1000 µF. Kalau terbalik, lepas daya segera.
  - Coba catu servo dari adaptor 5 V 3 A, bukan dari port USB komputer.
  - Dengan daya mati, coba putar horn servo dengan tangan. Kalau macet, gear/mekanisme bermasalah.

### Servo jitter / ESP32 reset
- Cek ground servo dan ESP32 common.
- Cek polaritas kapasitor 1000 µF.
- Pastikan kapasitor dekat konektor servo.
- Cek kabel USB/adaptor; coba kabel lebih tebal/pendek.
- Jalankan `s` (gerak pelan), bukan `x`.
- Kalau tetap reset, coba catu servo dari adaptor 5 V terpisah (kalau ada) dengan ground common.

### Buzzer tidak bunyi
- Cek polaritas.
- Coba tambah resistor 220 Ω seri kalau buzzer magnetic.
- Pastikan buzzer pasif; kalau active, `ledcWriteTone` mungkin tetap bunyi, tetapi karakter berbeda.
- Cek jumper ke GPIO25.

### Tombol selalu terbaca TEKAN
- Kemungkinan memakai dua kaki yang sepasang internal. Pakai dua kaki diagonal.
- Cek satu kaki ke GPIO27, satu ke GND.

## 9. Catatan

- Sketch ini belum menguji Wi-Fi, Telegram, atau jadwal.
- VL53L0X akan ditambahkan setelah disolder.
- Servo tidak di-attach saat boot supaya tidak ada gerakan tak terduga. Servo baru bergerak saat perintah `s`, `f`, atau `x`.
- Jangan menaruh ESP32, OLED, module TDS, atau module DS18B20 di dalam air. Hanya probe TDS dan probe DS18B20 yang boleh masuk air.
- Kalau servo berdengung keras atau mekanisme macet, lepas daya servo segera.
