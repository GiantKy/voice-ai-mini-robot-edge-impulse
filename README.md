# 🤖 Voice AI Mini Robot với Edge Impulse

> **Đề tài:** Xây dựng robot mini điều khiển bằng giọng nói – ESP32 tích hợp mô hình TinyML (Edge Impulse C++ Library) để nhận dạng từ khóa trực tiếp trên thiết bị, sau đó điều khiển động cơ tương ứng mà **không cần kết nối Internet hay máy tính ngoài**.

---

## 👥 Thành viên nhóm

| STT | Họ và Tên | MSSV | Vai trò |
|-----|-----------|------|---------|
| 1   | Nguyễn Văn A | 21xxxxxxx | Nhóm trưởng – AI Model & Python |
| 2   | Trần Thị B   | 21xxxxxxx | Firmware ESP32 & Motor |
| 3   | Lê Văn C     | 21xxxxxxx | Thu thập dữ liệu & Tài liệu |

> ⚠️ **Lưu ý:** Thay thế thông tin thành viên thực tế vào bảng trên.

---

## 📋 Mô tả đề tài

Dự án xây dựng một **robot mini điều khiển bằng giọng nói** theo kiến trúc **on-device AI** — toàn bộ quá trình nhận dạng giọng nói diễn ra ngay trên ESP32:

1. **ESP32 thu âm** qua microphone (I2S / Analog).
2. **Chạy mô hình TinyML** được nhúng sẵn (C++ Library từ Edge Impulse) để nhận dạng từ khóa.
3. **Điều khiển động cơ** (qua Motor Driver) dựa trên kết quả phân loại.

### 🎙️ Các nhãn lệnh được nhận dạng

| Nhãn | Hành động robot |
|------|-----------------|
| `gia-ky` | LED / buzzer xác nhận nhận dạng thành công |
| `di` | Tiến về phía trước |
| `dung` | Dừng tất cả động cơ |
| `quay` | Quay tại chỗ |
| `noise` | Bỏ qua – tiếng ồn nền |

### 🔄 Workflow hệ thống (On-Device AI)

```
┌──────────────────────────────────────────────────────────────┐
│                         ESP32                                │
│                                                              │
│  🎤 Microphone (I2S INMP441 / MAX9814)                      │
│      │                                                       │
│      ▼                                                       │
│  Thu âm liên tục (sliding window 1s, stride 250ms)           │
│      │                                                       │
│      ▼                                                       │
│  Edge Impulse C++ Inferencing SDK                            │
│      │  - Trích đặc trưng MFE (Mel Filterbank Energy)       │
│      │  - Chạy mô hình TFLite (EON Compiled, INT8)          │
│      │  - Confidence threshold: 0.6                          │
│      │                                                       │
│      ▼                                                       │
│  Kết quả phân loại (5 nhãn):                                 │
│  "gia-ky" / "di" / "dung" / "quay" / "noise"               │
│      │                                                       │
│      ├── "di"    → Motor Driver → Tiến thẳng               │
│      ├── "dung"  → Motor Driver → Dừng                     │
│      ├── "quay"  → Motor Driver → Quay                     │
│      ├── "gia-ky"→ LED / buzzer xác nhận                   │
│      └── "noise" → Bỏ qua, không làm gì                   │
│                                                              │
│  GPIO → Motor Driver (L298N / L293D) → DC Motors            │
└──────────────────────────────────────────────────────────────┘
```

**Công nghệ sử dụng:**
- 🧠 **Edge Impulse Studio** – Huấn luyện mô hình nhận dạng từ khóa (Keyword Spotting)
- ⚡ **EON Compiler** – Tối ưu và biên dịch mô hình thành C++ chạy trên MCU
- 🔧 **ESP32** – Vi điều khiển chạy toàn bộ pipeline AI + điều khiển motor
- 🏎️ **TFLite INT8 (compiled)** – Mô hình được lượng tử hoá và biên dịch sẵn

---

## 📁 Mô tả cấu trúc thư mục

```
voice-ai-mini-robot-edge-impulse/
│
├── firmware/                        # Mã nguồn chạy trên ESP32
│   ├── src/                         # File .ino / .cpp chính (thu âm → AI inference → motor)
│   ├── include/                     # Header files (.h) của dự án
│   ├── platformio/                  # Cấu hình PlatformIO (platformio.ini)
│   └── lib/                         # ⚡ Edge Impulse C++ Library (export từ Studio)
│       ├── edge-impulse-sdk/        #    SDK inferencing (DSP, classifier, CMSIS...)
│       ├── model-parameters/        #    model_metadata.h, model_variables.h
│       ├── tflite-model/            #    Model TFLite đã biên dịch (EON Compiled INT8)
│       └── CMakeLists.txt           #    Build script cho CMake
│
├── edge-impulse/                    # Tất cả liên quan đến quá trình huấn luyện AI
│   ├── dataset/
│   │   ├── raw-audio/               # File âm thanh gốc (.wav) – 5 nhãn: gia-ky, di, dung, quay, noise
│   │   └── processed/               # Dữ liệu đã qua tiền xử lý
│   ├── model/                       # Tài liệu model (EDGE_IMPULSE_README.txt, link project)
│   └── notebooks/                   # Jupyter Notebook phân tích dữ liệu / kiểm thử
│
├── docs/                            # Tài liệu dự án
│   ├── diagrams/                    # Sơ đồ hệ thống, luồng dữ liệu
│   ├── images/                      # Ảnh chụp phần cứng, demo robot
│   └── reports/                     # Báo cáo đề tài (.pdf, .docx)
│
├── scripts/                         # Script tiện ích (thu âm mẫu, test serial, v.v.)
├── config/                          # Cấu hình chung (COM port, ngưỡng confidence, v.v.)
├── tests/                           # Bộ kiểm thử
│   ├── firmware/                    # Test firmware ESP32
│   ├── python/                      # Unit test Python script
│   └── integration/                 # Test luồng đầy đủ laptop → ESP32
│
├── .gitignore                       # Danh sách file/folder không push lên git
└── README.md                        # File này
```

---

## ⚙️ Hướng dẫn cài đặt và chạy

### Yêu cầu hệ thống

- **OS (máy tính để nạp firmware):** Windows 10/11, macOS 12+, hoặc Ubuntu 20.04+
- **Arduino IDE:** >= 2.x hoặc **PlatformIO** (khuyến nghị)
- **Edge Impulse CLI** *(chỉ dùng khi thu thập dữ liệu)*: `npm install -g edge-impulse-cli`
- **Phần cứng:** ESP32 + Microphone (I2S INMP441 hoặc MAX9814) + Motor Driver (L298N/L293D) + DC Motors

---

### 1️⃣ Clone repository

```bash
git clone https://github.com/GiantKy/voice-ai-mini-robot-edge-impulse.git
cd voice-ai-mini-robot-edge-impulse
```

---

### 2️⃣ Cấu hình PlatformIO

Mở file `firmware/platformio/platformio.ini` và chỉnh thông số board:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

; Thêm Edge Impulse library từ thư mục lib/
lib_extra_dirs = lib
```

> ⚠️ Toàn bộ mô hình AI đã được nhúng vào `firmware/lib/` — **không cần cài thêm thư viện nào** từ Library Manager.

---

### 3️⃣ Nạp Firmware lên ESP32

**Cách 1: PlatformIO (khuyến nghị)**
```bash
cd firmware
pio run --target upload
pio device monitor    # Xem log Serial để kiểm tra kết quả nhận dạng
```

**Cách 2: Arduino IDE**
1. Mở Arduino IDE → `File > Open` → chọn `firmware/src/main.ino`
2. Vào `Sketch > Include Library > Add .ZIP Library` → thêm từng thư mục trong `firmware/lib/`
3. Vào `Tools > Board` → chọn **ESP32 Dev Module**
4. Chọn đúng `Port` → nhấn **Upload**

**Sau khi upload**, mở Serial Monitor (115200 baud) — ESP32 sẽ in:
```
[EI] Listening...
[EI] Detected: di     (0.92)
[EI] Detected: dung   (0.87)
[EI] Detected: gia-ky (0.81)
```

---

### 4️⃣ Thu thập dữ liệu & Huấn luyện mô hình (Edge Impulse)

```bash
# Đăng nhập Edge Impulse
edge-impulse-daemon

# Thu âm trực tiếp và upload lên Edge Impulse Studio
edge-impulse-data-forwarder
```

Vào [studio.edgeimpulse.com](https://studio.edgeimpulse.com) để:
1. Tạo project → Upload file `.wav` từ `edge-impulse/dataset/raw-audio/`
2. Tạo 5 nhãn: **gia-ky**, **di**, **dung**, **quay**, **noise**
3. Thiết kế Impulse: `MFE` → `Neural Network Classifier`
4. Huấn luyện → Kiểm thử → Export dưới dạng **Linux (AARCH64) .eim** hoặc **Python SDK**
5. Lưu file `.eim` vào `edge-impulse/model/`

---

## 📦 Phụ thuộc (Dependencies)

### Firmware – ESP32
| Thư viện | Nguồn | Mô tả |
|---------|-------|-------|
| **edge-impulse-sdk** | `firmware/lib/` | SDK inferencing của Edge Impulse (DSP + TFLite) |
| **model-parameters** | `firmware/lib/` | Header tham số mô hình (metadata, labels, variables) |
| **tflite-model** | `firmware/lib/` | Model TFLite đã biên dịch bằng EON Compiler (INT8) |
| Arduino ESP32 core | Board Manager | Core lõi cho ESP32 (`espressif32`) |

> ✅ Tất cả thư viện AI đã có sẵn trong repo — **không cần tải thêm** từ Internet.

### Thông số mô hình
| Thông số | Giá trị |
|---------|---------|
| Project ID | 1003610 |
| Owner | GiaKy |
| Số nhãn | 5 (gia-ky, di, dung, quay, noise) |
| Tần số lấy mẫu | 16.000 Hz |
| Cửa sổ dữ liệu | 1.000 ms |
| Confidence threshold | 0.6 |
| Engine | TFLite INT8 (EON Compiled) |
| Arena size | 6.265 bytes |

---

## 🔗 Liên kết hữu ích

- 📖 [Edge Impulse Documentation](https://docs.edgeimpulse.com)
- 🐍 [Edge Impulse Python SDK](https://docs.edgeimpulse.com/docs/run-inference/linux/python-sdk)
- 🔧 [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- 🌐 [Repository GitHub](https://github.com/GiantKy/voice-ai-mini-robot-edge-impulse)

---

## 📄 Giấy phép

Dự án phục vụ mục đích học thuật – Học kỳ cuối năm học 2025–2026.

---

*© 2026 – Nhóm Voice AI Mini Robot | Trường Đại học ...*