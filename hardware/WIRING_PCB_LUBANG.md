# Wiring PCB Lubang — Pakan Ikan Otomatis

> **Arsip:** rencana aktif sekarang memakai breadboard. Lihat [`WIRING_BREADBOARD.md`](WIRING_BREADBOARD.md).

Dokumen ini untuk versi yang sudah disolder di **PCB lubang (perfboard)**.
Semua modul memakai header agar bisa dilepas-pasang.

Skema visual: [`wiring_pcb_lubang.svg`](wiring_pcb_lubang.svg) — preview PNG: [`wiring_pcb_lubang.png`](wiring_pcb_lubang.png)

---

## 1. Peta pin final

| Fungsi | Pin ESP32 | Terhubung ke | Pin modul | Catatan |
|---|---|---|---|---|
| I2C SDA | GPIO21 | OLED + VL53L0X | SDA | Bus I2C, 3.3 V |
| I2C SCL | GPIO22 | OLED + VL53L0X | SCL | Bus I2C, 3.3 V |
| DS18B20 DATA | GPIO4 | DS18B20 | DATA / DQ | Butuh pull-up; modul biasanya sudah ada |
| TDS analog | GPIO34 | TDS Meter V1.0 | AOUT | GPIO34 input-only |
| Servo signal | GPIO18 | SG90 | Oranye/kuning | Sinyal 3.3 V |
| Buzzer | GPIO25 | Buzzer pasif | + | − ke GND |
| Tombol | GPIO27 | Tombol | Kaki 1 | Kaki 2 ke GND, `INPUT_PULLUP` |
| 3.3 V | 3V3 | OLED, DS18B20, VL53L0X, TDS | VCC/VIN | TDS default 3.3 V |
| 5 V | 5V | Servo, kapasitor | Merah, + | Hanya untuk servo; jangan ke sensor 3.3 V |
| GND | GND | Semua modul | GND | Common ground |

### Ringkasan koneksi

```text
ESP32 3V3  ─────────────── OLED VCC
                           DS18B20 VCC
                           VL53L0X VIN
                           TDS VCC (default 3.3 V)

ESP32 GND  ─────────────── semua GND
                           servo cokelat/hitam
                           kapasitor 1000 µF (−)

ESP32 5V   ─────────────── servo merah
                           kapasitor 1000 µF (+)

ESP32 GPIO21 (SDA) ─────── OLED SDA
                           VL53L0X SDA

ESP32 GPIO22 (SCL) ─────── OLED SCL
                           VL53L0X SCL

ESP32 GPIO4  ───────────── DS18B20 DATA
ESP32 GPIO34 ◄──────────── TDS AOUT
ESP32 GPIO18 ───────────── servo signal (oranye/kuning)
ESP32 GPIO25 ───────────── buzzer +
ESP32 GPIO27 ───────────── tombol kaki 1

Tombol kaki 2 ──────────── GND
```

---

## 2. Skema visual

Buka file [`wiring_pcb_lubang.svg`](wiring_pcb_lubang.svg) di browser.
Skema memakai **label net**:

- `+5V` = jalur 5 V servo
- `+3V3` = jalur 3.3 V sensor
- `GND` = ground bersama
- `SDA` / `SCL` = bus I2C
- `DS_DATA`, `TDS_AOUT`, `SERVO_SIG`, `BUZZER`, `BUTTON` = sinyal masing-masing

Contoh bentuk teksnya:

```text
┌─────────────────────┐
│ ESP32 Dev Module    │
│                     │
│ 5V   ─────── +5V    │──── servo merah + kapasitor +
│ 3V3  ─────── +3V3   │──── OLED VCC, DS18B20 VCC,
│                     │     VL53L0X VIN, TDS VCC
│ GND  ─────── GND    │──── semua GND
│ GPIO21 ───── SDA    │──── OLED SDA, VL53L0X SDA
│ GPIO22 ───── SCL    │──── OLED SCL, VL53L0X SCL
│ GPIO4  ───── DS_DATA│──── DS18B20 DATA
│ GPIO34 ◄──── TDS_AOUT──── TDS AOUT
│ GPIO18 ───── SERVO_SIG─── servo signal
│ GPIO25 ───── BUZZER │──── buzzer +
│ GPIO27 ───── BUTTON │──── tombol kaki 1
└─────────────────────┘
```

---

## 3. Header dan konektor

Prinsip yang disarankan:

- **Female header di PCB** untuk semua modul yang punya pin male.
- **Male header 3 pin di PCB** khusus servo, karena konektor servo female.
- Untuk modul yang letaknya jauh (OLED, VL53L0X, tombol, buzzer), gunakan kabel **female-to-male** dari modul ke female header di PCB.

| Komponen | Header di PCB | Kabel/adaptor | Catatan |
|---|---|---|---|
| ESP32 Dev Module | 2× female header, 15 pin (board 30 pin) atau 19 pin (board 38 pin) | — | Hitung pin board dulu |
| OLED SSD1306 | 1× female 4 pin | Langsung atau female-to-male jika remote | Urutan pin ikuti label modul |
| VL53L0X | 1× female 4 pin | Female-to-male jika sensor remote | 3.3 V saja |
| DS18B20 module | 1× female 3 pin | Langsung / female-to-male | Probe tidak disolder ke PCB |
| TDS Meter V1.0 | 1× female 3 pin | Langsung / female-to-male | Probe masuk ke module |
| Servo SG90 | 1× male 3 pin | Servo plug langsung | Urutan S / + / − |
| Tombol | 1× female 2 pin | Kabel 2 pin dari tombol | Pakai dua kaki diagonal |
| Buzzer | 1× female 2 pin | Langsung / kabel 2 pin | Cek polaritas |
| Kapasitor 1000 µF | — | Solder langsung | Dekat header servo |

### Urutan pin header servo

Buat male header 3 pin dengan urutan yang jelas:

```text
Pin 1 : S  (signal)  → GPIO18  → kabel oranye/kuning
Pin 2 : +  (5 V)     → 5V      → kabel merah
Pin 3 : −  (GND)     → GND     → kabel cokelat/hitam
```

Beri label `S`, `+`, `−` di PCB. Jangan hanya mengandalkan posisi, karena konektor servo bisa dipasang terbalik.

### Urutan pin modul

**Selalu baca label di PCB modul**, karena urutan fisik bisa berbeda:

- OLED: biasanya `GND VCC SCL SDA` atau `VCC GND SCL SDA`
- DS18B20: `VCC GND DATA` atau `+ − S`
- TDS: `VCC GND AOUT` atau `+ − A`
- VL53L0X: `VIN GND SDA SCL` atau `VCC GND SDA SCL`

Jangan menyambung berdasarkan urutan kabel saja.

---

## 4. Jalur daya dan ground

### 5 V

- Sumber: adaptor **5 V 3 A** via USB ke ESP32.
- Ambil dari pin **5V** ESP32 menuju header servo.
- Untuk satu SG90, jalur ini biasanya cukup (arus puncak < 1 A). Kalau servo bergerak lalu ESP32 reset atau OLED mati, ambil 5 V servo **langsung dari kabel adaptor** (splice USB) dengan GND tetap common. Jangan menambah adaptor kedua.
- Gunakan kabel **20-22 AWG** untuk 5 V dan GND servo.
- Kapasitor **1000 µF** dipasang paralel tepat di header servo:
  - `+` ke node 5 V servo
  - `−` ke node GND servo
- Jangan memberi daya servo dari pin **3V3**.

### 3.3 V

- Ambil dari pin **3V3** ESP32.
- Untuk OLED, DS18B20, VL53L0X, dan TDS (default).
- Gunakan kabel 24 AWG.
- Jangan menghubungkan 3.3 V ke 5 V.

### Ground

- Semua GND harus common.
- Buat **star ground** di dekat pin GND ESP32:
  - Servo GND kembali langsung ke titik ini.
  - Kapasitor 1000 µF (−) ke titik ini.
  - Sensor GND ke bus ground sensor, lalu ke titik ini.
- Jangan melewatkan arus servo melalui jalur ground sensor.

### TDS VCC: 3.3 V atau 5 V?

Default: **3.3 V**.

Modul TDS Meter V1.0 (DFRobot) biasanya punya output AOUT 0–2,3 V, sehingga aman untuk ADC ESP32 walaupun VCC 5 V. Tetapi kalau modul clone tidak jelas, tetap pakai 3.3 V.

Kalau ingin bisa memilih, tambahkan **selector 3 pin + jumper**:

```text
+3V3 ──o 1
        o 2 ──── TDS VCC
+5V  ──o 3
```

- Jumper 1–2 = 3.3 V (default)
- Jumper 2–3 = 5 V (hanya setelah yakin AOUT ≤ 3.3 V)

Jangan pindah ke 5 V tanpa memastikan output AOUT aman.

---

## 5. Tata letak fisik yang disarankan

```text
┌──────────────────────────────────────────────────────┐
│  USB ◄── ESP32                                     │
│  ┌──────────────┐                                  │
│  │   ESP32      │   ┌──────┐        ┌──────┐       │
│  │              │   │ OLED │        │ VL53 │       │
│  └──────────────┘   └──────┘        └──────┘       │
│                                                    │
│   ┌────────┐        ┌────────┐                     │
│   │ DS18B20│        │  TDS   │                     │
│   └────────┘        └────────┘                     │
│                                                    │
│   ┌────┐   ┌────┐   ┌─────────┐   ┌──────────┐    │
│   │BTN │   │BZ  │   │ SERVO   │   │ 1000 µF  │    │
│   └────┘   └────┘   │ S + −   │   │  +   −   │    │
│                     └─────────┘   └──────────┘    │
└──────────────────────────────────────────────────────┘
```

Saran:

- ESP32 di tepi board, port USB menghadap keluar.
- Header servo di tepi berlawanan, dekat kapasitor 1000 µF.
- OLED dan VL53L0X di satu sisi; DS18B20 dan TDS di sisi lain.
- Tombol dan buzzer di tepi depan agar mudah diakses.
- Jalur I2C (SDA/SCL) dibuat pendek dan sejajar.
- Jalur TDS AOUT dijauhkan dari kabel servo dan buzzer.
- Kabel probe TDS/DS18B20 diberi strain relief (lem tembak atau zip tie).
- Beri label pada setiap header.

### Modul remote

Kalau OLED, VL53L0X, tombol, atau buzzer dipasang di panel enclosure:

- Gunakan kabel 4 pin / 3 pin / 2 pin dengan header di PCB.
- I2C OLED + VL53L0X sebaiknya total kabel < 30–50 cm.
- VL53L0X hanya 3.3 V; jangan bawa 5 V lewat kabel sensor.
- Untuk VL53L0X, kabel 4 pin: `VIN GND SDA SCL`; tambahkan GND di antara SDA/SCL jika memungkinkan.
- Servo yang jauh dari board: kapasitor 1000 µF idealnya diletakkan di ujung kabel servo, bukan hanya di PCB. Kalau servo dekat board, cukup di header servo.

---

## 6. Catatan per modul

### OLED SSD1306

- VCC 3.3 V, GND, SDA GPIO21, SCL GPIO22.
- Alamat biasanya `0x3C`; sebagian `0x3D`.
- Urutan pin berbeda-beda; baca label.
- Jangan pasang 5 V kecuali modul jelas punya regulator/level shifter.

### DS18B20

- VCC 3.3 V, GND, DATA GPIO4.
- Module yang ada resistor di board biasanya sudah pull-up.
- Kalau tidak terdeteksi, tambah **5 kΩ** antara DATA dan 3.3 V.
  - Dua resistor 10 kΩ diparalel = 5 kΩ.
- Probe boleh masuk air; module dan sambungan kabel jangan kena air.

### TDS Meter V1.0

- VCC 3.3 V (default), GND, AOUT GPIO34.
- Probe masuk air; board module jangan kena air.
- AOUT hanya boleh 0–3.3 V.
- Tanpa larutan kalibrasi, ppm hanya estimasi.
- Jauhkan kabel probe dari servo/buzzer.

### VL53L0X

- VIN 3.3 V, GND, SDA GPIO21, SCL GPIO22.
- Alamat `0x29`; tidak bentrok dengan OLED.
- Jika breakout punya `XSHUT`, hubungkan ke 3.3 V (atau ikuti board; biasanya sudah pull-up). `GPIO` dibiarkan.
- Sensor menghadap ke bawah ke corong. FOV sekitar 25°.
- Corong 4.5 cm bisa memantulkan dinding saat level rendah. Baffle hitam 6–8 mm ID, panjang 25–30 mm membantu.
- Kabel I2C sensor sebaiknya pendek (< 15 cm).

### Servo SG90

- Merah 5 V, cokelat/hitam GND, oranye/kuning signal GPIO18.
- Jangan dari 3.3 V.
- Kapasitor 1000 µF dekat header servo.
- Jangan menahan beban terus-menerus; `detach()` setelah gerak.
- Kalau servo berdengung/macet, lepas daya.

### Tombol

- 4 kaki tactile: pakai dua kaki **diagonal**.
- Satu kaki ke GPIO27, satu ke GND.
- Tidak perlu resistor eksternal (`INPUT_PULLUP`).
- Untuk remote, solder kabel ke dua kaki diagonal, ujung lain ke header 2 pin.

### Buzzer pasif

- `+` ke GPIO25, `−` ke GND.
- Passive buzzer butuh PWM; sketch memakai 2 kHz.
- Kalau buzzer magnetic/arus besar, tambah resistor **220 Ω seri**.
- Kalau piezo, biasanya bisa langsung.
- Jangan hubungkan buzzer ke 5 V.

---

## 7. Urutan solder yang aman

Sebelum mulai:

- Ukur jumlah pin dan jarak baris header ESP32 (30 pin / 38 pin, 0.9" atau 1.0").
- Tandai posisi USB, header servo, dan arah hadap OLED/VL53L0X di PCB.
- Solder **dua pin sudut** header ESP32 dulu, pasang board, cek masuk/lepas, baru solder sisanya.
- Jangan menyolder modul sensor langsung; solder header-nya saja.
- Siapkan kabel jumper **female-to-male** untuk modul yang dipasang remote.

1. **Solder header ESP32** (female) di PCB. Pasang ESP32, cek alignment, baru solder semua pin.
2. **Solder header modul**: OLED, VL53L0X, DS18B20, TDS, servo, tombol, buzzer.
3. **Solder jalur 5 V servo** dengan kabel 20–22 AWG.
4. **Solder jalur GND** dan star ground.
5. **Solder jalur 3.3 V**.
6. **Solder kapasitor 1000 µF** tepat di header servo. Perhatikan polaritas: stripe = negatif.
7. **Solder jalur sinyal**: SDA/SCL, DS18B20, TDS AOUT, servo signal, buzzer, tombol.
8. **Cek visual** dengan kaca pembesar: tidak ada solder bridge, tidak ada kabel terjepit.
9. Kalau bisa pinjam multimeter, cek kontinuitas:
   - 5 V tidak short ke GND
   - 3.3 V tidak short ke GND
   - 5 V tidak nyambung ke 3.3 V
10. **Tes bertahap**:
    - ESP32 saja + USB → hidup
    - OLED saja → I2C scan
    - tambah DS18B20 → baca suhu
    - tambah TDS → baca mV
    - tambah VL53L0X → alamat 0x29
    - tambah servo + kapasitor → tes `s`
    - tambah tombol + buzzer → tes `b` dan long-press

---

## 8. Tes setelah soldering

Gunakan sketch bring-up yang sama, lalu:

| Tes | Perintah | Hasil yang diharapkan |
|---|---|---|
| I2C | `i` | `0x3C`/`0x3D` (OLED) dan `0x29` (VL53L0X) |
| OLED | `o` | Tampilan refresh |
| DS18B20 | `d` | Suhu wajar |
| TDS | `t` | mV berubah saat probe di air |
| Buzzer | `b` | 3 beep |
| Servo | `s` | 0° → 90° → 0°, PWM 50 Hz |
| Tombol | tahan >800 ms | Feed test |
| VL53L0X | sketch terpisah | Jarak terbaca |

VL53L0X belum ada di sketch bring-up saat ini. Setelah disolder, tambahkan library `Adafruit_VL53L0X` dan buat sketch tes VL53L0X terpisah, atau perluas sketch bring-up.

---

## 9. Peringatan

- Jangan membalik VCC dan GND pada modul.
- Jangan memberi 5 V ke VL53L0X, OLED, atau DS18B20 kecuali modul jelas punya regulator/level shifter.
- AOUT TDS tidak boleh melebihi 3.3 V.
- Servo hanya dari 5 V, bukan 3.3 V.
- Polaritas kapasitor 1000 µF jangan terbalik.
- Jangan menaruh module TDS/DS18B20 di dalam air; hanya probe.
- Periksa solder bridge sebelum memberi daya.
- Setelah daya dimatikan, kapasitor masih menyimpan muatan; hindari korsleting.
