#ifndef _CONFIG_H_
#define _CONFIG_H_

// ============================================
// Pin Configuration - Voice AI Mini Robot
// Chỉnh sửa theo sơ đồ nối dây thực tế
// ============================================

// ---------- I2S Microphone (INMP441) ----------
#define I2S_SCK_PIN         26      // Serial Clock (BCLK)
#define I2S_WS_PIN          25      // Word Select (LRCLK)
#define I2S_SD_PIN          33      // Serial Data (DOUT)
#define I2S_PORT            I2S_NUM_0

// ---------- Motor Driver (L298N) ----------
// Motor A (bánh trái)
#define MOTOR_IN1_PIN       12
#define MOTOR_IN2_PIN       14
#define MOTOR_ENA_PIN       32      // PWM tốc độ

// Motor B (bánh phải)
#define MOTOR_IN3_PIN       27
#define MOTOR_IN4_PIN       13
#define MOTOR_ENB_PIN       15      // PWM tốc độ

// ---------- LED & Buzzer ----------
#define LED_PIN             5       // LED ngoài (không dùng LED tích hợp GPIO2)
#define BUZZER_PIN          4

// ---------- Motor PWM ----------
#define MOTOR_PWM_FREQ      5000    // 5 kHz
#define MOTOR_PWM_RESOLUTION 8      // 8-bit (0-255)
#define MOTOR_SPEED_DEFAULT 200     // Tốc độ mặc định (0-255)

// ---------- Inference ----------
#define COMMAND_COOLDOWN_MS 1000    // Chờ giữa 2 lệnh (ms)

#endif // _CONFIG_H_
