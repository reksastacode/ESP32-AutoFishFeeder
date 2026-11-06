# Dokumentasi Rancangan Proyek IoT

# Pakan Ikan Otomatis dengan Kendali Melalui Bot Telegram

> **Status dokumen:** Rancangan awal proyek  
> **Mikrokontroler:** ESP32 Dev Module  
> **Platform kendali jarak jauh:** Telegram Bot  
> **Media pemantauan lokal:** OLED 0.96 inci

## 1. Gambaran Umum Proyek

Proyek **Pakan Ikan Otomatis dengan Kendali Melalui Bot Telegram** merupakan sistem berbasis Internet of Things (IoT) yang dirancang untuk membantu proses pemberian pakan ikan sekaligus memantau beberapa kondisi akuarium. Sistem menggunakan ESP32 Dev Module sebagai pusat pengendali yang menghubungkan sensor, mekanisme pemberi pakan, layar OLED, tombol manual, buzzer, dan bot Telegram.

Pakan disimpan di dalam wadah penampung. Sebuah mekanisme berupa **rail atau rel berlubang** digerakkan oleh servo. Ketika rail bergerak melewati bagian bawah corong yang memiliki lubang, sejumlah pakan akan masuk ke lubang pada rail dan terbawa untuk dikeluarkan. Dengan demikian, gerakan servo digunakan untuk mengatur proses pengambilan dan penyaluran pakan.

Pengguna dapat memantau kondisi akuarium secara langsung melalui OLED yang dipasang di dekat akuarium. Informasi yang sama juga dapat diminta dari jarak jauh melalui perintah bot Telegram. Pemberian pakan dapat dipicu melalui tombol fisik maupun perintah Telegram.

## 2. Tujuan Proyek

1. Membuat sistem pemberian pakan ikan yang dapat dijalankan melalui mekanisme servo.
2. Memungkinkan pemberian pakan secara manual dari tombol yang terpasang pada perangkat.
3. Memungkinkan pengguna meminta pemberian pakan dari jarak jauh melalui bot Telegram.
4. Memantau suhu air dan nilai TDS sebagai informasi tambahan mengenai kondisi air akuarium.
5. Memantau perkiraan jumlah pakan yang tersisa di dalam wadah penampung.
6. Menampilkan informasi pemantauan secara lokal melalui OLED dan secara jarak jauh melalui Telegram.
7. Memberikan peringatan suara sebelum mekanisme servo mulai bergerak.

## 3. Komponen dan Fungsinya

| Komponen | Fungsi dalam sistem | Catatan rancangan |
|---|---|---|
| ESP32 Dev Module | Mikrokontroler utama yang membaca sensor, mengendalikan servo dan buzzer, memperbarui OLED, serta berkomunikasi dengan Telegram melalui Wi-Fi. | Menjadi pusat pemrosesan dan pengendalian seluruh perangkat. |
| Sensor TDS Meter | Mengukur perkiraan total zat pad terlarut (*Total Dissolved Solids*) dalam air. yang nantinya bisa mendeteksi bahwa air didalam akuarium harus segera diganti | **TDS bukan sensor kekeruhan.** Sensor ini tidak mengukur tingkat keruh secara langsung. |
| Sensor VL53L0X | Mengukur jarak dari sensor ke permukaan pakan di dalam wadah. | Nilai jarak digunakan untuk memperkirakan tingkat atau sisa isi pakan setelah proses kalibrasi. |
| Sensor DS18B20 | Mengukur suhu air akuarium. | Sebaiknya menggunakan probe DS18B20 tahan air yang sesuai untuk penggunaan di akuarium. |
| Servo motor | Menggerakkan rail berlubang yang melewati bagian bawah corong pakan untuk mengambil dan mengeluarkan pakan. | Sudut dan pola gerak servo perlu disesuaikan dengan bentuk mekanisme serta ukuran pakan. |
| OLED 0.96 inci | Menampilkan informasi pemantauan secara langsung di dekat akuarium. | Umumnya menggunakan komunikasi I2C; alamat dan tegangan kerja perlu diperiksa pada modul. |
| Tombol | Memicu pemberian pakan secara manual langsung dari perangkat. | Dapat menggunakan konfigurasi `INPUT_PULLUP` pada ESP32. |
| Buzzer pasif | Memberikan bunyi peringatan sebelum servo bergerak. | Buzzer pasif memerlukan sinyal berfrekuensi/PWM untuk menghasilkan nada. |
| Kapasitor elektrolit 1000 µF | Membantu meredam penurunan tegangan sesaat ketika servo mulai bergerak. | Dipasang paralel pada jalur suplai 5 V dan GND di dekat servo dengan polaritas yang benar. Kapasitor bukan pengganti catu daya yang memadai. |

### 3.1 Catatan penting mengenai sensor TDS

Pada rancangan awal, sensor TDS disebut digunakan untuk melihat kekeruhan air. Secara teknis, **TDS Meter mengukur perkiraan jumlah zat pad terlarut dalam air**, biasanya berdasarkan konduktivitas listrik. Nilai TDS tidak menunjukkan kekeruhan atau banyaknya partikel tersuspensi secara langsung.

Apabila tujuan utama proyek adalah mengetahui apakah air terlihat keruh, komponen yang lebih sesuai adalah **sensor turbidity**. Sensor TDS tetap dapat digunakan apabila proyek juga ingin memantau perubahan kadar zat terlarut sebagai salah satu indikator kualitas air. Karena itu, tampilan dan laporan sistem sebaiknya menyebut hasil sensor ini sebagai **nilai TDS**, bukan tingkat kekeruhan.

### 3.2 Cara kerja sensor VL53L0X untuk membaca sisa pakan

VL53L0X merupakan sensor jarak berbasis *Time-of-Flight* (ToF). Sensor dipasang di bagian atas wadah pakan dan diarahkan ke permukaan pakan. Ketika permukaan pakan semakin rendah, jarak yang terbaca akan semakin besar. ESP32 dapat mengubah hasil pengukuran jarak tersebut menjadi perkiraan tingkat isi wadah.

Agar hasilnya dapat ditampilkan sebagai persentase atau status seperti **Penuh**, **Sedang**, dan **Hampir Habis**, wadah perlu dikalibrasi pada kondisi penuh dan kosong. Bentuk permukaan pakan yang tidak rata, debu, posisi sensor, serta pantulan dari bagian dalam wadah dapat memengaruhi pembacaan.

## 4. Arsitektur Sistem

ESP32 menjadi pusat kendali dan terhubung dengan tiga kelompok utama:

1. **Input:** sensor TDS, VL53L0X, DS18B20, dan tombol manual.
2. **Output:** servo, buzzer, dan OLED.
3. **Komunikasi:** Wi-Fi untuk mengirim dan menerima pesan melalui bot Telegram.

Gambaran hubungan sistem:

```text
                 ┌──────────────────────┐
                 │      BOT TELEGRAM    │
                 │  /status   /feed     │
                 └──────────┬───────────┘
                            │ Wi-Fi
                            │
┌──────────────┐    ┌────────▼─────────┐    ┌──────────────┐
│ Sensor TDS   ├───►│                  ├───►│ OLED 0.96"   │
└──────────────┘    │                  │    └──────────────┘
┌──────────────┐    │       ESP32      │    ┌──────────────┐
│ VL53L0X      ├───►│   Dev Module     ├───►│ Servo motor  │
└──────────────┘    │                  │    └──────┬───────┘
┌──────────────┐    │                  │           │
│ DS18B20      ├───►│                  │    Mekanisme rail
└──────────────┘    │                  │    dan corong pakan
┌──────────────┐    │                  │
│ Tombol       ├───►│                  ├───► Buzzer pasif
└──────────────┘    └──────────────────┘
```

Diagram tersebut merupakan gambaran konseptual, bukan diagram pengawatan. Jalur daya, pin, dan rangkaian pendukung tetap perlu ditentukan sesuai spesifikasi setiap modul.

## 5. Mekanisme Pemberian Pakan

Mekanisme pemberian pakan dirancang menggunakan wadah penampung, corong dengan lubang di bagian bawah, rail berlubang, dan servo motor.

Urutan kerja mekanisme:

1. Pakan ikan disimpan di dalam wadah penampung.
2. Pakan bergerak menuju corong dan berkumpul di sekitar lubang keluaran.
3. Dalam posisi awal, rail berada pada posisi yang telah ditentukan agar lubangnya dapat menerima sejumlah pakan dari corong.
4. Ketika perintah pemberian pakan diterima, buzzer berbunyi sebagai tanda bahwa mekanisme akan bergerak.
5. Setelah jeda peringatan, servo menggerakkan rail melewati lubang pada bagian bawah corong.
6. Pakan yang berada di lubang rail terbawa menuju titik keluaran dan jatuh ke akuarium.
7. Servo mengembalikan rail ke posisi awal agar mekanisme siap digunakan kembali.

Jumlah pakan yang dikeluarkan dipengaruhi oleh ukuran lubang, bentuk rail, ukuran dan jenis pakan, serta jarak atau sudut gerakan servo. Karena itu, jumlah pakan tidak dapat ditentukan hanya dari sudut servo. Mekanisme perlu diuji dan dikalibrasi menggunakan jenis pakan yang benar-benar akan digunakan.

## 6. Alur Kerja Sistem Secara Keseluruhan

### 6.1 Saat perangkat dinyalakan

1. ESP32 melakukan inisialisasi pin, sensor, OLED, servo, dan buzzer.
2. OLED menampilkan informasi awal atau status inisialisasi.
3. ESP32 mencoba terhubung ke jaringan Wi-Fi.
4. Setelah terhubung, ESP32 mulai berkomunikasi dengan bot Telegram.
5. Sistem membaca data sensor secara berkala dan memperbarui tampilan OLED.

### 6.2 Pemantauan kondisi akuarium

1. DS18B20 membaca suhu air.
2. Sensor TDS membaca nilai TDS air.
3. VL53L0X membaca jarak permukaan pakan dari posisi sensor.
4. ESP32 mengolah hasil pembacaan dan menentukan status sisa pakan berdasarkan hasil kalibrasi.
5. OLED diperbarui secara berkala agar informasi terbaru dapat dilihat langsung.
6. Saat pengguna mengirim perintah status melalui Telegram, ESP32 mengirimkan hasil pembacaan sensor terbaru.

### 6.3 Pemberian pakan melalui tombol fisik

1. Pengguna menekan tombol pada perangkat.
2. ESP32 mendeteksi penekanan tombol dan memeriksa apakah mekanisme sedang tidak digunakan.
3. Buzzer memberikan peringatan sebelum servo bergerak.
4. Servo menggerakkan rail untuk mengeluarkan pakan.
5. Servo kembali ke posisi awal.
6. Sistem memperbarui status dan, apabila diatur, mengirim pemberitahuan bahwa pemberian pakan telah dijalankan.

### 6.4 Pemberian pakan melalui Telegram

1. Pengguna mengirim perintah pemberian pakan melalui bot Telegram.
2. ESP32 menerima dan memeriksa perintah tersebut.
3. Sistem memastikan mekanisme tidak sedang menjalankan proses pemberian pakan.
4. Buzzer berbunyi sebagai peringatan.
5. Servo menjalankan mekanisme rail dan mengembalikannya ke posisi awal.
6. Bot Telegram mengirim balasan bahwa proses telah dijalankan atau memberi tahu apabila perintah tidak dapat diproses.

## 7. Rancangan Perintah Bot Telegram

Berikut contoh perintah yang dapat disediakan. Nama perintah dan format balasan masih dapat disesuaikan saat pemrograman.

| Perintah | Fungsi | Contoh informasi balasan |
|---|---|---|
| `/start` | Memulai interaksi dengan bot dan menampilkan pengenalan singkat. | Nama sistem dan daftar perintah. |
| `/help` | Menampilkan panduan penggunaan bot. | Daftar perintah yang tersedia. |
| `/status` | Meminta kondisi akuarium dan perangkat secara real-time. | Suhu air, nilai TDS, perkiraan sisa pakan, dan status koneksi/perangkat. |
| `/feed` | Meminta sistem menjalankan pemberian pakan. | Status proses pemberian pakan berhasil dijalankan atau ditolak karena sistem sedang sibuk. |

**Catatan:** `/status` sebaiknya mengambil pembacaan sensor terbaru, bukan hanya mengirim data lama yang tersimpan. Untuk menghindari pemberian pakan berulang akibat pesan ganda atau tombol yang tertahan, program perlu memiliki pengunci proses (*busy flag*) dan jeda minimum (*cooldown*) yang sesuai.
akan ada perintah untuk melakukan penjadwalan dimana setiap rentang waktu tertentu atau dijam tertentu, perangkat akan otomatis memberi makan ikan sesuai yang sudah di input di bot telegram

### Contoh format balasan `/status`

```text
STATUS AKUARIUM

Suhu Air       : 27.5 °C
Nilai TDS      : 180 ppm
Sisa Pakan     : 65% (Sedang)
Koneksi Wi-Fi  : Terhubung
Status Sistem  : Siap

Gunakan /feed untuk memberi pakan.
```

Angka di atas hanya contoh tampilan, bukan hasil pengukuran nyata atau rekomendasi kondisi ideal untuk semua jenis ikan.

## 8. Informasi pada OLED

OLED 0.96 inci digunakan untuk menampilkan kondisi perangkat secara lokal. Informasi yang disarankan antara lain:

- Suhu air dalam satuan derajat Celsius.
- Nilai TDS dalam satuan ppm, sesuai hasil pembacaan dan kalibrasi modul.
- Perkiraan sisa pakan dalam persentase atau kategori.
- Status Wi-Fi atau Telegram.
- Status mekanisme, misalnya **Siap**, **Memberi pakan**, atau **Periksa perangkat**.

Contoh susunan tampilan:

```text
+------------------------+
|     MONITOR AQUARIUM   |
|------------------------|
| Suhu  : 27.5 C         |
| TDS   : 180 ppm        |
| Pakan : 65%            |
| Wi-Fi : CONNECTED      |
| Status: READY          |
+------------------------+
```

Tampilan sebenarnya bergantung pada ukuran layar, pustaka OLED, dan tata letak program.

## 9. Rekomendasi Awal Pengawatan

Tabel berikut merupakan **usulan alokasi pin**, bukan ketentuan wajib. Periksa kembali pin yang tersedia pada varian ESP32 Dev Module yang digunakan dan spesifikasi modul sebelum menyambungkan rangkaian.

| Perangkat | Sinyal | Usulan pin ESP32 | Catatan |
|---|---|---:|---|
| OLED 0.96 inci | SDA | GPIO 21 | Jalur I2C bersama. |
| OLED 0.96 inci | SCL | GPIO 22 | Jalur I2C bersama. |
| VL53L0X | SDA | GPIO 21 | Dapat berbagi bus I2C dengan OLED jika alamat tidak bertabrakan. |
| VL53L0X | SCL | GPIO 22 | Periksa alamat dan tegangan modul. |
| DS18B20 | DATA | GPIO 4 | Umumnya memerlukan resistor *pull-up* sekitar 4,7 kΩ antara DATA dan 3,3 V. Ikuti datasheet/probe yang digunakan. |
| TDS Meter | Analog OUT | GPIO 34 | GPIO 34 merupakan input-only. Pastikan tegangan keluaran analog tidak melebihi batas ADC ESP32. |
| Servo | Signal | GPIO 18 | Catu daya servo dibahas pada bagian catatan daya. |
| Buzzer pasif | Signal/PWM | GPIO 25 | Gunakan rangkaian penggerak/transistor bila arus buzzer melebihi kemampuan GPIO. |
| Tombol | Salah satu kaki | GPIO 27 | Kaki lainnya ke GND jika menggunakan `INPUT_PULLUP`. |

OLED dan VL53L0X dapat menggunakan jalur I2C yang sama selama alamat perangkat tidak bertabrakan dan level tegangannya kompatibel. Sensor TDS yang menggunakan keluaran analog perlu diperiksa secara khusus: **jangan memasukkan tegangan di atas batas pin ADC ESP32**. Gunakan pembagi tegangan atau rangkaian penyesuai level apabila diperlukan.

## 10. Catatan Catu Daya dan Perlindungan Rangkaian

Servo dapat menarik arus cukup besar, terutama ketika mulai bergerak atau menahan beban. Lonjakan kebutuhan arus ini dapat menyebabkan tegangan turun dan berpotensi membuat ESP32 restart atau pembacaan sensor terganggu.

Pada rancangan awal, servo direncanakan menggunakan jalur 5 V dan ditambahkan kapasitor elektrolit 1000 µF. Kapasitor tersebut dapat membantu menyediakan arus sesaat dan mengurangi penurunan tegangan lokal, tetapi **tidak menjamin sumber 5 V mampu memenuhi kebutuhan arus servo**.

Rekomendasi rancangan daya:

1. Lebih aman menggunakan catu daya 5 V teregulasi yang terpisah dan memadai untuk servo, sesuai kebutuhan arus servo yang dipilih.
2. Satukan GND catu daya servo dengan GND ESP32 agar sinyal kendali servo memiliki referensi yang sama.
3. Pasang kapasitor 1000 µF secara paralel antara 5 V dan GND sedekat mungkin dengan konektor servo. Perhatikan polaritas: kaki positif ke 5 V dan kaki negatif ke GND.
4. Periksa rating tegangan kapasitor dan pastikan sesuai untuk jalur 5 V.
5. Jangan menganggap pin 5 V pada board ESP32 atau port USB selalu mampu menyuplai arus servo. Kemampuan suplai bergantung pada board, sumber USB, kabel, dan beban lainnya.
6. Pastikan tegangan sinyal, catu daya, dan arus kerja setiap modul sesuai datasheet.

## 11. Hal yang Perlu Diperhatikan dalam Implementasi

### 11.1 Kalibrasi dan konsistensi pakan

- Uji beberapa kali jumlah pakan yang keluar untuk setiap siklus servo.
- Gunakan jenis dan ukuran pakan yang sama ketika melakukan kalibrasi.
- Pastikan rail tidak macet akibat pakan yang menggumpal atau tersangkut.
- Atur posisi awal dan akhir servo dengan batas gerak yang aman agar mekanisme tidak menekan bagian wadah secara berlebihan.

### 11.2 Pembacaan sensor

- Lakukan kalibrasi sensor TDS menggunakan larutan standar yang sesuai dengan modul.
- Jika digunakan untuk pemantauan air, pertimbangkan kompensasi suhu karena pembacaan TDS dapat dipengaruhi suhu.
- Kalibrasi VL53L0X pada kondisi wadah penuh dan kosong.
- Pastikan probe DS18B20 yang digunakan memang tahan air dan sambungannya terlindung dari kelembapan.
- Tampilkan status kesalahan apabila sensor tidak terdeteksi atau menghasilkan pembacaan yang tidak masuk akal.

### 11.3 Keamanan kendali

- Batasi akses bot Telegram, misalnya dengan memeriksa *chat ID* pengguna yang diizinkan.
- Jangan membagikan token bot Telegram di repositori publik atau dokumentasi yang dapat diakses umum.
- Cegah proses pemberian pakan berjalan bersamaan ketika tombol dan Telegram mengirim perintah pada waktu hampir bersamaan.
- Sediakan jeda minimum antar-pemberian pakan agar perintah berulang tidak langsung menggerakkan servo berkali-kali.
- Pastikan bagian elektronik terlindung dari cipratan air dan ditempatkan pada posisi yang aman di sekitar akuarium.

## 12. Pengembangan Lanjutan

Setelah fungsi dasar berhasil, proyek dapat dikembangkan dengan beberapa fitur tambahan:

1. **Jadwal pemberian pakan otomatis:** ESP32 dapat menjalankan jadwal berdasarkan waktu yang diperoleh melalui jaringan, misalnya NTP, selama koneksi internet tersedia.
2. **Peringatan pakan hampir habis:** bot Telegram mengirim notifikasi ketika pembacaan VL53L0X menunjukkan wadah hampir kosong.
3. **Riwayat pemberian pakan:** menyimpan waktu pemberian pakan dan sumber perintahnya, yaitu tombol atau Telegram.
4. **Notifikasi kondisi sensor:** mengirim peringatan ketika sensor tidak terbaca atau nilai berada di luar rentang yang telah ditetapkan.
5. **Pengaturan jumlah pakan:** menyediakan beberapa pola gerak servo yang telah dikalibrasi, misalnya porsi kecil dan porsi normal.
6. **Sensor turbidity:** menambahkan sensor turbidity apabila pemantauan kekeruhan air memang menjadi kebutuhan proyek.

## 13. Kesimpulan

Pakan Ikan Otomatis dengan Kendali Melalui Bot Telegram dirancang sebagai perangkat IoT yang menggabungkan pemberian pakan, pemantauan kondisi akuarium, dan kendali jarak jauh. ESP32 Dev Module berperan sebagai pusat kendali yang menerima data dari sensor TDS, VL53L0X, dan DS18B20, lalu mengolahnya untuk ditampilkan pada OLED maupun dikirimkan melalui bot Telegram.

Proses pemberian pakan dilakukan menggunakan servo yang menggerakkan rail berlubang pada bagian bawah corong. Pemberian pakan dapat dimulai melalui tombol fisik atau perintah Telegram, dengan buzzer sebagai indikator sebelum servo bergerak. Sensor VL53L0X digunakan untuk memperkirakan sisa pakan, sedangkan DS18B20 memantau suhu air dan TDS Meter memantau perkiraan jumlah zat pad terlarut.

Keberhasilan implementasi sangat bergantung pada kalibrasi sensor, ketepatan desain mekanisme rail dan corong, kestabilan komunikasi Wi-Fi, serta rancangan catu daya yang sesuai. Khusus untuk sensor TDS, hasil pengukurannya tidak boleh disebut sebagai tingkat kekeruhan karena pengukuran kekeruhan memerlukan sensor turbidity.

---

## 14. Ringkasan Spesifikasi Rancangan

| Aspek | Rancangan |
|---|---|
| Nama proyek | Pakan Ikan Otomatis dengan Kendali Melalui Bot Telegram |
| Konsep | IoT untuk pemberian pakan dan pemantauan akuarium |
| Mikrokontroler | ESP32 Dev Module |
| Sensor kualitas air | TDS Meter (nilai TDS, bukan kekeruhan langsung) |
| Sensor suhu | DS18B20 |
| Sensor sisa pakan | VL53L0X |
| Aktuator | Servo motor dengan mekanisme rail berlubang dan corong |
| Display lokal | OLED 0.96 inci |
| Kendali manual | Tombol fisik |
| Kendali jarak jauh | Bot Telegram melalui Wi-Fi |
| Indikator | Buzzer pasif sebelum servo bergerak |
| Komponen pendukung daya | Kapasitor elektrolit 1000 µF pada jalur servo, dengan catu daya yang memadai |
| Perintah utama Telegram | `/status` dan `/feed` |
