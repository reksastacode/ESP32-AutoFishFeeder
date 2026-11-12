# Wiring Breadboard — Pakan Ikan Otomatis

Rencana aktif: **breadboard biasa**. Dokumen PCB lubang disimpan sebagai arsip.

## 1. Peta pin

| Fungsi | GPIO |
|---|---|
| I2C SDA | GPIO21 |
| I2C SCL | GPIO22 |
| DS18B20 DATA | GPIO4 |
| TDS AOUT | GPIO34 |
| Servo signal | GPIO18 |
| Buzzer | GPIO25 |
| Tombol | GPIO27 |

## 2. Pembagian rel

| Rel | Isi | Sumber |
|---|---|---|
| Rel atas | **5V** + **GND servo** | Pin 5V dan satu pin GND ESP32 |
| Rel bawah | **3V3** + **GND sensor** | Pin 3V3 dan pin GND ESP32 lainnya |

- Beri label fisik **5V** dan **3V3**.
- Banyak breadboard 830 titik punya rel terbelah di tengah. Bridge kiri-kanan dengan jumper.
- GND servo dan GND sensor bertemu di pin GND ESP32 (star ground).
- Jangan menyambung rel 5V ke rel 3V3.

## 3. Jalur komponen

| Komponen | VCC / + | GND / − | Sinyal |
|---|---|---|---|
| OLED SSD1306 | rel 3V3 | rel GND sensor | SDA→GPIO21, SCL→GPIO22 |
| VL53L0X | rel 3V3 | rel GND sensor | SDA→GPIO21, SCL→GPIO22 |
| DS18B20 | terminal pin 1 → rel 3V3 | terminal pin 3 → rel GND sensor | terminal pin 2 → GPIO4 |
| TDS Meter | rel 3V3 | rel GND sensor | AOUT→GPIO34 |
| Servo SG90 | rel 5V | rel GND servo | S→GPIO18 |
| Buzzer pasif | — | rel GND sensor | +→GPIO25 |
| Tombol | — | rel GND sensor | kaki 1→GPIO27 |
| Kapasitor 1000 µF | +→rel 5V | −→rel GND servo | — |

Catatan:

- Buzzer magnetic: tambah resistor **220 Ω seri**.
- Tombol 4 kaki: pakai dua kaki **diagonal**.
- Servo memakai male header 3 pin: **S / + / −**.
- Kabel probe DS18B20: merah=VCC, hitam=GND, kuning/putih=DATA. Verifikasi produk.
- TDS VCC default **3,3 V**; AOUT tidak boleh >3,3 V.
- VL53L0X hanya **3,3 V**.

## 4. Topologi pull-up DS18B20

```text
ESP32                          Terminal block                 Probe DS18B20

3V3 ────────────────┬─────────  Pin 1 (VCC) ──────────────── merah
                    │
                  [4,7k]  ← pull-up
                    │
GPIO4 ──────────────┴─────────  Pin 2 (DATA) ─────────────── kuning/putih
GND ──────────────────────────  Pin 3 (GND) ──────────────── hitam
```

- Resistor 4,7 kΩ antara **VCC dan DATA**, bukan seri dan bukan ke GND.
- Cukup satu resistor untuk satu sensor.
- Terminal block pitch 3,5/5,08 mm biasanya tidak pas di breadboard; solder kabel jumper ke tiap pin terminal.

## 5. Topologi I2C OLED + VL53L0X

```text
ESP32 3V3 ────┬──── OLED VCC
              └──── VL53L0X VIN

ESP32 GND ────┬──── OLED GND
              └──── VL53L0X GND

ESP32 GPIO21 ─┬──── OLED SDA
              └──── VL53L0X SDA

ESP32 GPIO22 ─┬──── OLED SCL
              └──── VL53L0X SCL
```

- OLED: `0x3C` atau `0x3D`.
- VL53L0X: `0x29`.
- XSHUT (kalau ada) → 3V3; jangan ke GND.
- GPIO/INT → NC.

## 6. Layout kasar

```text
Zona kiri        : OLED + VL53L0X (I2C)
Zona tengah      : ESP32, USB menghadap keluar
Zona kanan       : DS18B20 + TDS
Zona depan       : tombol + buzzer
Kanan atas       : servo + kapasitor 1000 µF, dekat rel 5V
```

- Jalur I2C dibuat pendek.
- Kabel AOUT TDS dijauhkan dari servo dan buzzer.
- Modul remote memakai kabel female-to-male.

## 7. Urutan pasang dan tes

1. ESP32 + rel 5V/GND servo + rel 3V3/GND sensor.
2. OLED → menu `i`, harus muncul `0x3C`/`0x3D`.
3. VL53L0X → `i` harus muncul `0x29`; tekan `v` untuk baca jarak.
4. DS18B20 → tekan `d`, cek suhu wajar.
5. TDS → tekan `t`, cek mV berubah saat probe di air.
6. Servo + kapasitor → tekan `s`, cek PWM 50 Hz.
7. Tombol + buzzer → tekan `b`, lalu tahan tombol >800 ms.

## 8. Rawan

- Rel daya terbelah — bridge.
- Kontak breadboard longgar — servo bisa jitter/reset.
- Servo hanya dari 5V, bukan 3V3.
- Kapasitor 1000 µF: stripe = negatif.
- TDS default 3,3V.
- VL53L0X 3,3V saja; XSHUT jangan ke GND.
- Tombol pakai kaki diagonal.
- Kalau servo membuat ESP32 reset, ambil 5V servo langsung dari kabel adaptor dengan GND tetap common.
