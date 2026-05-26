# 🤖 Voice AI Mini Robot với Edge Impulse

> **Đề tài:** Xây dựng robot mini tích hợp nhận dạng giọng nói tại biên (on-device) sử dụng nền tảng Edge Impulse và vi điều khiển nhúng.

---

## 👥 Thành viên nhóm

| STT | Họ và Tên | MSSV | Vai trò |
|-----|-----------|------|---------|
| 1   | Nguyễn Văn A | 21xxxxxxx | Nhóm trưởng – Firmware & AI Model |
| 2   | Trần Thị B   | 21xxxxxxx | Backend Server & API |
| 3   | Lê Văn C     | 21xxxxxxx | Web Dashboard & Tài liệu |

> ⚠️ **Lưu ý:** Thay thế thông tin thành viên thực tế vào bảng trên.

---

## 📋 Mô tả đề tài

Dự án xây dựng một robot mini có khả năng **nhận diện giọng nói ngoại tuyến (offline)** trực tiếp trên thiết bị nhúng mà không cần kết nối Internet. Hệ thống sử dụng nền tảng **Edge Impulse** để huấn luyện mô hình học máy nhận dạng từ khóa (keyword spotting), sau đó triển khai (deploy) lên vi điều khiển (Arduino / ESP32). Robot phản hồi các lệnh thoại cơ bản như *"di chuyển"*, *"dừng lại"*, *"bật đèn"*, v.v.

**Công nghệ sử dụng:**
- 🧠 **Edge Impulse** – Huấn luyện và xuất mô hình TinyML
- 🔧 **Arduino / ESP32** – Vi điều khiển chạy firmware nhúng
- 🐍 **Python / Node.js** – Server thu thập và giám sát dữ liệu
- 🌐 **Web Dashboard** – Giao diện theo dõi trạng thái robot theo thời gian thực

---

## 📁 Mô tả cấu trúc thư mục

```
voice-ai-mini-robot-edge-impulse/
│
├── firmware/                        # Mã nguồn chạy trên vi điều khiển (Arduino/ESP32)
│   ├── src/                         # File .ino / .cpp chính
│   ├── include/                     # Header files (.h)
│   ├── lib/                         # Thư viện ngoài (tải về thủ công)
│   └── platformio/                  # Cấu hình PlatformIO (platformio.ini)
│
├── edge-impulse/                    # Tất cả liên quan đến AI Model
│   ├── dataset/
│   │   ├── raw-audio/               # File âm thanh gốc (.wav) dùng để training
│   │   └── processed/               # Dữ liệu đã qua tiền xử lý
│   ├── model/                       # Model đã export từ Edge Impulse (.zip, .h)
│   └── notebooks/                   # Jupyter Notebook phân tích dữ liệu
│
├── server/                          # Backend Server (Node.js / Python Flask)
│   ├── src/
│   │   ├── routes/                  # Định nghĩa các API endpoint
│   │   ├── controllers/             # Logic xử lý nghiệp vụ
│   │   ├── models/                  # Schema database (nếu có)
│   │   └── middleware/              # Auth, logging, error handling
│   ├── config/                      # File cấu hình (db, env mẫu)
│   └── logs/                        # Log server (không push lên git)
│
├── web/                             # Giao diện Web Dashboard
│   ├── public/
│   │   └── assets/
│   │       ├── images/              # Hình ảnh giao diện
│   │       └── audio/               # File âm thanh demo
│   └── src/
│       ├── components/              # Các UI component tái sử dụng
│       ├── pages/                   # Các trang chính
│       ├── styles/                  # CSS / SCSS
│       └── utils/                   # Hàm tiện ích
│
├── docs/                            # Tài liệu dự án
│   ├── diagrams/                    # Sơ đồ hệ thống, luồng dữ liệu
│   ├── images/                      # Ảnh chụp phần cứng, demo
│   └── reports/                     # Báo cáo đề tài (.pdf, .docx)
│
├── scripts/                         # Script tiện ích (flash firmware, thu âm, v.v.)
├── config/                          # Cấu hình chung toàn dự án
├── tests/                           # Bộ kiểm thử
│   ├── firmware/                    # Test cho firmware
│   ├── server/                      # Unit test server
│   └── integration/                 # Integration test
│
├── .gitignore                       # Danh sách file/folder không push lên git
└── README.md                        # File này
```

---

## ⚙️ Hướng dẫn cài đặt và chạy

### Yêu cầu hệ thống

- **OS:** Windows 10/11, macOS 12+, hoặc Ubuntu 20.04+
- **Node.js:** >= 18.x
- **Python:** >= 3.9
- **Arduino IDE:** >= 2.x hoặc PlatformIO
- **Edge Impulse CLI:** `npm install -g edge-impulse-cli`

---

### 1️⃣ Clone repository

```bash
git clone https://github.com/USERNAME/voice-ai-mini-robot-edge-impulse.git
cd voice-ai-mini-robot-edge-impulse
```

---

### 2️⃣ Cài đặt và chạy Server

```bash
cd server
npm install          # hoặc: pip install -r requirements.txt (nếu dùng Python)
cp config/.env.example config/.env   # Sao chép file cấu hình mẫu
npm start            # Khởi động server tại http://localhost:3000
```

---

### 3️⃣ Chạy Web Dashboard

```bash
cd web
npm install
npm run dev          # Khởi động tại http://localhost:5173
```

---

### 4️⃣ Nạp Firmware lên vi điều khiển

**Cách 1: Arduino IDE**
1. Mở Arduino IDE → `File > Open` → chọn `firmware/src/main.ino`
2. Vào `Tools > Board` → chọn đúng board (Arduino Nano 33 BLE / ESP32)
3. Chọn đúng `Port` rồi nhấn **Upload**

**Cách 2: PlatformIO**
```bash
cd firmware
pio run --target upload
```

---

### 5️⃣ Thu thập dữ liệu & Huấn luyện mô hình (Edge Impulse)

```bash
# Đăng nhập Edge Impulse
edge-impulse-daemon

# Thu âm trực tiếp từ thiết bị
edge-impulse-data-forwarder
```

Sau đó vào [studio.edgeimpulse.com](https://studio.edgeimpulse.com) để:
1. Tạo project → Upload dữ liệu từ `edge-impulse/dataset/raw-audio/`
2. Thiết kế Impulse → Huấn luyện mô hình
3. Export → **Arduino Library** → giải nén vào `firmware/lib/`

---

## 📦 Phụ thuộc (Dependencies)

### Firmware (Arduino / PlatformIO)
| Thư viện | Phiên bản | Mô tả |
|---------|-----------|-------|
| Edge Impulse SDK | latest | Chạy mô hình TinyML trên thiết bị |
| Arduino_LSM9DS1 | >= 1.1.1 | Cảm biến IMU (nếu dùng Nano 33 BLE) |
| WiFiNINA | >= 1.8.14 | Kết nối WiFi |

### Server (Node.js)
| Package | Phiên bản | Mô tả |
|---------|-----------|-------|
| express | ^4.18.2 | Web framework |
| socket.io | ^4.7.2 | Giao tiếp thời gian thực |
| dotenv | ^16.3.1 | Quản lý biến môi trường |
| cors | ^2.8.5 | Cross-Origin Resource Sharing |

### Web Dashboard
| Package | Phiên bản | Mô tả |
|---------|-----------|-------|
| vite | ^5.x | Build tool |
| socket.io-client | ^4.7.2 | Kết nối Socket.IO từ frontend |
| chart.js | ^4.4.x | Vẽ biểu đồ dữ liệu cảm biến |

### Python (Edge Impulse / Scripts)
| Package | Phiên bản | Mô tả |
|---------|-----------|-------|
| edge-impulse-cli | latest | CLI thu thập dữ liệu |
| numpy | >= 1.24 | Xử lý mảng số |
| matplotlib | >= 3.7 | Vẽ biểu đồ phân tích |
| librosa | >= 0.10 | Phân tích âm thanh |

---

## 🔗 Liên kết hữu ích

- 📖 [Edge Impulse Documentation](https://docs.edgeimpulse.com)
- 🔧 [Arduino Nano 33 BLE Pinout](https://docs.arduino.cc/hardware/nano-33-ble)
- 🌐 [Repository GitHub](https://github.com/USERNAME/voice-ai-mini-robot-edge-impulse)

---

## 📄 Giấy phép

Dự án phục vụ mục đích học thuật – Học kỳ cuối năm học 2025–2026.

---

*© 2026 – Nhóm Voice AI Mini Robot | Trường Đại học ...*