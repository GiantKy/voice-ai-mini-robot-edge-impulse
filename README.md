# 🤖 Voice AI Mini Robot với Edge Impulse

> **Đề tài:** Xây dựng robot mini điều khiển bằng giọng nói – ESP32 tích hợp mô hình TinyML (Edge Impulse C++ Library) để nhận dạng từ khóa trực tiếp trên thiết bị, sau đó điều khiển động cơ tương ứng mà **không cần kết nối Internet hay máy tính ngoài**.

---

## 👥 Thành viên nhóm

| STT | Họ và Tên | MSSV |
|-----|-----------|------|
| 1   | Thái Gia Kỳ | N23DCCI038 |

---

## 📋 Mô tả đề tài

Dự án xây dựng một **robot mini điều khiển bằng giọng nói** theo kiến trúc **on-device AI** — toàn bộ quá trình nhận dạng giọng nói diễn ra ngay trên ESP32:

1. **ESP32 thu âm** qua microphone I2S (INMP441).
2. **Chạy mô hình TinyML** được nhúng sẵn (C++ Library từ Edge Impulse) để nhận dạng từ khóa.
3. **Điều khiển động cơ** (qua Motor Driver L298N) dựa trên kết quả phân loại.

### 🎙️ Các lệnh giọng nói được nhận dạng

| Lệnh | Hành động robot | Ví dụ output |
|------|-----------------|--------------|
| `Di` | Tiến về phía trước | `[CMD] >>> Di (95%) -> Tiến về phía trước` |
| `Dung` | Dừng tất cả động cơ | `[CMD] >>> Dung (87%) -> Dừng lại` |
| `Quay` | Quay tại chỗ | `[CMD] >>> Quay (98%) -> Quay tại chỗ` |
| `GiaKy` | Buzzer beep + LED nhấp nháy (xác nhận) | `[CMD] >>> GiaKy (96%) -> Xác nhận giọng nói` |
| `Noise` | Bỏ qua – tiếng ồn nền | *(không hiện CMD)* |

### 🔄 Workflow hệ thống (On-Device AI)

```
┌─────────────────────────────────────────────────────────────┐
│                         ESP32 (Dual Core)                   │
│                                                             │
│  [Core 0] — Audio Capture                                   │
│  🎤 INMP441 (I2S) → 16kHz, 32-bit → shift 16-bit            │    
│      → Slice buffer (4000 samples = 250ms)                  │
│                                                             │
│  [Core 1] — AI Inference + Motor Control                    │
│  Slice buffer → MFCC (13 cepstral, 32 filters)              │
│      → TFLite INT8 (EON Compiled) → 5 nhãn                  │
│      │                                                      │
│      ├── "Di"    → Motor Driver → Tiến thẳng                │
│      ├── "Dung"  → Motor Driver → Dừng                      │
│      ├── "Quay"  → Motor Driver → Quay tại chỗ              │
│      ├── "GiaKy" → Buzzer beep + LED nhấp nháy              │
│      └── "Noise" → Bỏ qua                                   │
│                                                             │
│  GPIO → Motor Driver (L298N) → 2x DC Motors                 │
└─────────────────────────────────────────────────────────────┘
```

**Công nghệ sử dụng:**
- 🧠 **Edge Impulse Studio** – Huấn luyện mô hình nhận dạng từ khóa (Keyword Spotting)
- ⚡ **EON Compiler** – Tối ưu và biên dịch mô hình thành C++ chạy trên MCU
- 🔧 **ESP32** – Vi điều khiển dual-core chạy toàn bộ pipeline AI + điều khiển motor
- 🏎️ **TFLite INT8 (compiled)** – Mô hình được lượng tử hoá và biên dịch sẵn
- 📟 **PlatformIO** – Build system cho embedded development

---

## 📁 Cấu trúc thư mục

```
voice-ai-mini-robot-edge-impulse/
│
├── firmware/                        # 🔧 Mã nguồn chạy trên ESP32
│   ├── src/
│   │   └── main.cpp                # File chính: I2S capture + EI inference + motor control
│   ├── include/
│   │   └── config.h                # Cấu hình pin (I2S, Motor, LED, Buzzer)
│   ├── platformio.ini              # Cấu hình PlatformIO build
│   └── lib/                        # ⚡ Edge Impulse C++ Library (export từ Studio)
│       ├── edge-impulse-sdk/       #    SDK inferencing (DSP, TFLite, CMSIS, ESP-NN)
│       ├── model-parameters/       #    model_metadata.h, model_variables.h
│       ├── tflite-model/           #    Model TFLite đã biên dịch (EON Compiled INT8)
│       └── CMakeLists.txt          #    Build script cho CMake
│
├── edge-impulse/                    # 📊 Liên quan đến huấn luyện AI
│   ├── dataset/
│   │   ├── training/              # 321 samples huấn luyện (.json)
│   │   ├── testing/               # 79 samples kiểm thử (.json)
│   │   ├── info.labels            # Metadata labels
│   │   └── README.txt             # Hướng dẫn import dataset
│   └── model/                      # Tài liệu model (EDGE_IMPULSE_README.txt)
│
├── .gitignore                       # Danh sách file không push lên git
└── README.md                        # File này
```

---

## 🔩 Phần cứng cần thiết

| Linh kiện | Mô tả | Ghi chú |
|-----------|--------|---------|
| **ESP32 Dev Module** | Vi điều khiển chính | Dual Core 240MHz, 320KB RAM, 4MB Flash |
| **INMP441** | Microphone I2S | Digital, chất lượng cao cho voice recognition |
| **L298N** | Motor Driver | Điều khiển 2 động cơ DC |
| **2x DC Motor** | Động cơ bánh xe | 3-6V |
| **LED** | Đèn trạng thái | Dùng LED tích hợp GPIO2 |
| **Buzzer** | Còi báo hiệu | Active buzzer 3.3V |
| **Nguồn** | Pin / adapter | 7-12V cho L298N, 3.3V cho ESP32 |

### 🔌 Sơ đồ nối dây (mặc định)

```
INMP441          ESP32              L298N
────────         ─────              ─────
VDD ──────────── 3.3V
GND ──────────── GND
SCK ──────────── GPIO 26
WS  ──────────── GPIO 25
SD  ──────────── GPIO 33
L/R ──────────── GND (kênh trái)

                 GPIO 12 ─────────── IN1  (Motor A)
                 GPIO 14 ─────────── IN2  (Motor A)
                 GPIO 32 ─────────── ENA  (PWM Motor A)
                 GPIO 27 ─────────── IN3  (Motor B)
                 GPIO 13 ─────────── IN4  (Motor B)
                 GPIO 15 ─────────── ENB  (PWM Motor B)

                 GPIO 2  ─────────── LED (tích hợp)
                 GPIO 4  ─────────── Buzzer
```

> 💡 Chỉnh sửa pin trong file `firmware/include/config.h` nếu nối dây khác.

---

## ⚙️ Hướng dẫn cài đặt và chạy

### Yêu cầu

- **OS:** Windows 10/11, macOS, hoặc Linux
- **Python:** >= 3.8
- **PlatformIO Core:** Cài qua `pip install platformio`
- **Phần cứng:** ESP32 + INMP441 + L298N + DC Motors

### 1️⃣ Clone repository

```bash
git clone https://github.com/GiantKy/voice-ai-mini-robot-edge-impulse.git
cd voice-ai-mini-robot-edge-impulse
```

### 2️⃣ Cài đặt PlatformIO

```bash
pip install platformio
```

> Hoặc cài **PlatformIO IDE** extension trong VS Code.

### 3️⃣ Chỉnh cấu hình pin (nếu cần)

Mở `firmware/include/config.h` và sửa pin theo sơ đồ nối dây thực tế:

```c
// I2S Microphone (INMP441)
#define I2S_SCK_PIN     26
#define I2S_WS_PIN      25
#define I2S_SD_PIN      33

// Motor Driver (L298N)
#define MOTOR_IN1_PIN   12
#define MOTOR_IN2_PIN   14
#define MOTOR_ENA_PIN   32
#define MOTOR_IN3_PIN   27
#define MOTOR_IN4_PIN   13
#define MOTOR_ENB_PIN   15

// LED & Buzzer
#define LED_PIN         2
#define BUZZER_PIN      4
```

### 4️⃣ Build firmware

```bash
cd firmware
pio run
```

Kết quả build thành công:
```
RAM:   [=         ]  10.1% (used 32,932 bytes from 327,680 bytes)
Flash: [=         ]  13.1% (used 411,585 bytes from 3,145,728 bytes)
======================== [SUCCESS] ========================
```

### 5️⃣ Nạp firmware vào ESP32

Cắm ESP32 vào máy tính qua USB, sau đó:

```bash
pio run --target upload
```

> ⚠️ Nếu upload thất bại, giữ nút **BOOT** trên ESP32 trong khi upload.

### 6️⃣ Mở Serial Monitor

```bash
pio device monitor --baud 115200
```

Kết quả khi chạy thành công:
```
╔══════════════════════════════════════╗
║   Voice AI Mini Robot - ESP32        ║
║   Edge Impulse TinyML Inference      ║
╚══════════════════════════════════════╝

[MOTOR] Initialized OK
[I2S] Microphone initialized OK
[EI] Project:    Điều khiển robot mini bằng lệnh giọng nói
[EI] Owner:      GiaKy
[EI] Labels:     5
[EI] Frequency:  16000 Hz
[EI] Window:     1000 ms
[EI] Slice:      4000 samples (250 ms)
[EI] Threshold:  60%
[EI] Arena:      6265 bytes

[EI] ✓ Listening...

[Timing] DSP: 23 ms | Inference: 4 ms | Di: 4% Dung: 8% GiaKy: 0% Noise: 88% Quay: 0%
```

### 7️⃣ Sử dụng

Nói các lệnh vào microphone INMP441:

| Nói | Robot sẽ | Serial Monitor hiện |
|-----|----------|---------------------|
| **"đi"** | Tiến về phía trước | `[CMD] >>> Di (95%) -> Tiến về phía trước` |
| **"dừng"** | Dừng lại | `[CMD] >>> Dung (87%) -> Dừng lại` |
| **"quay"** | Quay tại chỗ | `[CMD] >>> Quay (98%) -> Quay tại chỗ` |
| **"gia ky"** | Buzzer beep 3 lần + LED nhấp nháy | `[CMD] >>> GiaKy (96%) -> Xác nhận giọng nói` |
| *(im lặng)* | Không làm gì | `Noise: 95%` |

> 🎯 Lệnh chỉ được thực thi khi **confidence ≥ 60%** và có **cooldown 1 giây** giữa các lệnh.

---

## 📦 Phụ thuộc (Dependencies)

### Firmware – ESP32
| Thư viện | Nguồn | Mô tả |
|---------|-------|-------|
| **edge-impulse-sdk** | `firmware/lib/` | SDK inferencing (DSP + TFLite + CMSIS-NN + ESP-NN) |
| **model-parameters** | `firmware/lib/` | Header tham số mô hình (metadata, labels, variables) |
| **tflite-model** | `firmware/lib/` | Model TFLite đã biên dịch bằng EON Compiler (INT8) |
| Arduino ESP32 core | PlatformIO | Core lõi cho ESP32 (`espressif32@6.9.0`) |

> ✅ Tất cả thư viện AI đã có sẵn trong repo — **không cần tải thêm** từ Internet.

### Thông số mô hình AI

| Thông số | Giá trị |
|---------|---------|
| Project ID | 1003610 |
| Owner | GiaKy |
| Số nhãn | 5 (Di, Dung, GiaKy, Quay, Noise) |
| Tần số lấy mẫu | 16,000 Hz |
| Cửa sổ dữ liệu | 1,000 ms |
| Sliding window | 4 slices × 250 ms |
| DSP | MFCC (13 cepstral, 32 filters, 256 FFT) |
| Confidence threshold | 0.6 (60%) |
| Engine | TFLite INT8 (EON Compiled) |
| Arena size | 6,265 bytes |

### Hiệu năng trên ESP32

| Metric | Giá trị |
|--------|---------|
| DSP (MFCC extraction) | ~23 ms |
| Inference (Neural Network) | ~4 ms |
| Tổng latency | ~27 ms |
| RAM sử dụng | 10.1% (32 KB / 320 KB) |
| Flash sử dụng | 13.1% (411 KB / 3 MB) |

---

## 🔧 Kiến trúc firmware

Firmware sử dụng **dual-core ESP32**:

| Core | Chức năng | Priority |
|------|-----------|----------|
| **Core 0** | Thu âm I2S liên tục (250ms/slice) | Task priority 1 |
| **Core 1** | Edge Impulse inference + motor control | Main loop |

```
main.cpp
├── i2s_init()              — Khởi tạo I2S cho INMP441
├── motor_init()            — Khởi tạo PWM cho L298N
├── audio_capture_task()    — [Core 0] Thu âm liên tục
├── get_audio_signal_data() — Callback cho Edge Impulse SDK
├── execute_command()       — Ánh xạ label → hành động motor
├── setup()                 — Khởi tạo tất cả + tạo task Core 0
└── loop()                  — [Core 1] Chạy inference liên tục
```

---

## ❓ Xử lý lỗi thường gặp

| Lỗi | Nguyên nhân | Giải pháp |
|-----|-------------|-----------|
| `COM port not found` | Chưa cài driver USB | Cài driver **CP2102** hoặc **CH340** |
| `Access denied COM3` | Port đang bị chiếm | Đóng Serial Monitor / Arduino IDE khác |
| `Upload failed` | ESP32 không vào boot mode | Giữ nút **BOOT** khi upload |
| `Build lâu (~5 phút)` | Lần đầu compile SDK | Bình thường, lần sau nhanh hơn |
| `Noise 100%` liên tục | Mic không hoạt động | Kiểm tra nối dây INMP441 |

---

## 🔗 Liên kết hữu ích

- 📖 [Edge Impulse Documentation](https://docs.edgeimpulse.com)
- 🎯 [Edge Impulse Project #1003610](https://studio.edgeimpulse.com/studio/1003610)
- 🔧 [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- 📟 [PlatformIO Documentation](https://docs.platformio.org)
- 🌐 [Repository GitHub](https://github.com/GiantKy/voice-ai-mini-robot-edge-impulse)

---

## 📄 Giấy phép

Dự án phục vụ mục đích học thuật – Học kỳ cuối năm học 2025–2026.

---

*© 2026 – Voice AI Mini Robot | Học viện Công Nghệ Bưu Chính Viễn Thông cơ sở TP.HCM*
