# 🤖 Voice AI Mini Robot với Edge Impulse

> **Đề tài:** Xây dựng robot mini điều khiển bằng giọng nói – Micro laptop chạy mô hình AI (Edge Impulse) nhận dạng lệnh thoại, gửi lệnh qua USB Serial đến ESP32 để điều khiển động cơ.

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

Dự án xây dựng một **robot mini điều khiển bằng giọng nói** theo kiến trúc hai tầng:

1. **Tầng AI (Micro laptop):** Thu âm từ microphone → chạy mô hình Edge Impulse bằng Python → nhận dạng từ khóa → gửi chuỗi lệnh qua cổng **USB Serial**.
2. **Tầng chấp hành (ESP32):** Đọc lệnh từ Serial → điều khiển **động cơ (motor)** tương ứng.

### 🎙️ Các nhãn lệnh được nhận dạng

| Nhãn | Lệnh gửi qua USB | Hành động robot |
|------|-----------------|-----------------|
| `gia ky` | `gk` | Giữ nguyên / chào (tín hiệu nhận dạng thành công) |
| `di` | `di` | Tiến về phía trước |
| `dung` | `dung` | Dừng tất cả động cơ |
| `quay` | `quay` | Quay tại chỗ |
| `noise` | *(bỏ qua)* | Tiếng ồn nền – không gửi lệnh |

### 🔄 Workflow hệ thống

```
┌─────────────────────────────────────────────────────────┐
│                    MICRO LAPTOP                         │
│                                                         │
│  🎤 Microphone                                          │
│      │                                                  │
│      ▼                                                  │
│  Python Script (main.py)                                │
│      │  - Thu âm liên tục (sliding window)              │
│      │  - Trích đặc trưng MFCC                         │
│      │  - Chạy mô hình Edge Impulse (.eim)              │
│      │  - Ngưỡng tin cậy (confidence threshold)         │
│      │                                                  │
│      ▼                                                  │
│  Kết quả nhận dạng:                                     │
│  "gia ky" / "di" / "dung" / "quay" / "noise"           │
│      │                                                  │
│      │  Gửi chuỗi lệnh qua USB Serial (pyserial)       │
└──────┼──────────────────────────────────────────────────┘
       │  USB Cable (Serial / COM Port)
       ▼
┌─────────────────────────────────────────────────────────┐
│                      ESP32                              │
│                                                         │
│  Serial.read() → parse lệnh                            │
│      │                                                  │
│      ├── "di"   → Motor tiến                           │
│      ├── "dung" → Motor dừng                           │
│      ├── "quay" → Motor quay                           │
│      └── "gk"   → LED / buzzer xác nhận               │
│                                                         │
│  GPIO → Motor Driver (L298N / L293D) → DC Motors       │
└─────────────────────────────────────────────────────────┘
```

**Công nghệ sử dụng:**
- 🧠 **Edge Impulse** – Huấn luyện mô hình nhận dạng từ khóa (Keyword Spotting)
- 🐍 **Python** – Chạy mô hình AI, giao tiếp Serial trên laptop
- 🔧 **ESP32** – Vi điều khiển nhận lệnh và điều khiển motor
- 🔌 **USB Serial** – Kênh giao tiếp giữa laptop và ESP32

---

## 📁 Mô tả cấu trúc thư mục

```
voice-ai-mini-robot-edge-impulse/
│
├── firmware/                        # Mã nguồn chạy trên ESP32
│   ├── src/                         # File .ino / .cpp chính (đọc Serial → điều khiển motor)
│   ├── include/                     # Header files (.h)
│   ├── lib/                         # Thư viện ngoài
│   └── platformio/                  # Cấu hình PlatformIO (platformio.ini)
│
├── edge-impulse/                    # Tất cả liên quan đến AI Model
│   ├── dataset/
│   │   ├── raw-audio/               # File âm thanh gốc (.wav) – 5 nhãn: gia-ky, di, dung, quay, noise
│   │   └── processed/               # Dữ liệu đã qua tiền xử lý
│   ├── model/                       # Model export từ Edge Impulse (.eim cho Linux/Mac, .zip)
│   └── notebooks/                   # Jupyter Notebook phân tích dữ liệu / kiểm thử mô hình
│
├── python/                          # Script Python chạy trên Micro laptop
│   ├── main.py                      # Script chính: thu âm → AI → gửi Serial
│   ├── classifier.py                # Wrapper gọi mô hình Edge Impulse (.eim)
│   ├── serial_sender.py             # Module giao tiếp USB Serial (pyserial)
│   └── requirements.txt             # Danh sách thư viện Python cần cài
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

- **OS (Laptop):** Windows 10/11, macOS 12+, hoặc Ubuntu 20.04+
- **Python:** >= 3.9
- **Arduino IDE:** >= 2.x hoặc PlatformIO (để nạp firmware ESP32)
- **Edge Impulse CLI:** `npm install -g edge-impulse-cli`
- **Phần cứng:** Micro laptop + ESP32 + Motor Driver (L298N/L293D) + DC Motors + Cáp USB

---

### 1️⃣ Clone repository

```bash
git clone https://github.com/GiantKy/voice-ai-mini-robot-edge-impulse.git
cd voice-ai-mini-robot-edge-impulse
```

---

### 2️⃣ Cài đặt Python (chạy AI trên laptop)

```bash
cd python
pip install -r requirements.txt
```

**Cấu hình COM Port** – mở `config/config.yaml` và chỉnh:
```yaml
serial:
  port: "COM3"        # Windows: COMx | Linux: /dev/ttyUSB0 | Mac: /dev/cu.usbserial-x
  baudrate: 115200

model:
  path: "../edge-impulse/model/model.eim"
  confidence_threshold: 0.7   # Chỉ gửi lệnh khi độ tin cậy >= 70%
```

**Chạy chương trình AI:**
```bash
python main.py
```

> 🎤 Chương trình sẽ liên tục lắng nghe microphone. Khi phát hiện từ khóa với độ tin cậy đủ cao, chuỗi lệnh tương ứng được gửi tự động qua USB Serial đến ESP32.

---

### 3️⃣ Nạp Firmware lên ESP32

**Cách 1: Arduino IDE**
1. Mở Arduino IDE → `File > Open` → chọn `firmware/src/main.ino`
2. Vào `Tools > Board` → chọn **ESP32 Dev Module**
3. Vào `Tools > Port` → chọn đúng COM Port của ESP32
4. Nhấn **Upload**

**Cách 2: PlatformIO**
```bash
cd firmware
pio run --target upload
```

**Lệnh Serial mà ESP32 nhận và xử lý:**
```
"di"   → Chạy motor tiến
"dung" → Dừng tất cả motor
"quay" → Quay tại chỗ
"gk"   → Tín hiệu xác nhận (LED/buzzer)
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

### Python – Laptop (AI + Serial)
| Package | Phiên bản | Mô tả |
|---------|-----------|-------|
| edge-impulse-linux | latest | Chạy mô hình `.eim` trên laptop |
| pyserial | >= 3.5 | Giao tiếp USB Serial với ESP32 |
| pyaudio | >= 0.2.13 | Thu âm từ microphone |
| numpy | >= 1.24 | Xử lý mảng số |
| librosa | >= 0.10 | Phân tích âm thanh (tùy chọn) |

### Firmware – ESP32 (Arduino)
| Thư viện | Phiên bản | Mô tả |
|---------|-----------|-------|
| Arduino ESP32 core | >= 2.x | Core cho ESP32 |
| *(không cần thư viện AI)* | – | AI chạy hoàn toàn trên laptop |

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