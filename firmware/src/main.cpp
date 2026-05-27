/**
 * ====================================================
 *  Voice AI Mini Robot - ESP32 Firmware
 * ====================================================
 *  Chức năng:
 *   1. Thu âm liên tục qua I2S microphone (INMP441)
 *   2. Chạy Edge Impulse inference (MFCC + TFLite INT8)
 *   3. Nhận dạng 5 lệnh: Di, Dung, GiaKy, Quay, Noise
 *   4. Điều khiển motor DC qua L298N
 *
 *  Edge Impulse Project: 1003610 (GiaKy)
 *  Model: EON Compiled, TFLite INT8, 16kHz, 1s window
 * ====================================================
 */

#include <Arduino.h>
#include <driver/i2s.h>

/* Edge Impulse SDK */
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"

/* Cấu hình pin */
#include "config.h"

// ===================== GLOBALS =====================

// Audio buffer cho 1 slice (250ms = 4000 samples)
static int16_t slice_buffer[EI_CLASSIFIER_SLICE_SIZE];
static volatile bool slice_ready = false;

// Cooldown để tránh spam lệnh
static unsigned long last_command_time = 0;

// Debug mode
static const bool debug_nn = false;

// ===================== I2S MICROPHONE =====================

/**
 * Khởi tạo I2S để đọc dữ liệu từ INMP441
 */
static void i2s_init()
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = EI_CLASSIFIER_FREQUENCY,         // 16000 Hz
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,   // INMP441 output 32-bit
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num   = I2S_SCK_PIN,
        .ws_io_num    = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num  = I2S_SD_PIN
    };

    esp_err_t err;
    err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[ERROR] I2S driver install failed: %d\n", err);
    }
    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("[ERROR] I2S set pin failed: %d\n", err);
    }
    i2s_zero_dma_buffer(I2S_PORT);

    Serial.println("[I2S] Microphone initialized OK");
}

// ===================== MOTOR CONTROL =====================

static void motor_init()
{
    // Motor A - PWM channel 0
    ledcSetup(0, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    ledcAttachPin(MOTOR_ENA_PIN, 0);
    pinMode(MOTOR_IN1_PIN, OUTPUT);
    pinMode(MOTOR_IN2_PIN, OUTPUT);

    // Motor B - PWM channel 1
    ledcSetup(1, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    ledcAttachPin(MOTOR_ENB_PIN, 1);
    pinMode(MOTOR_IN3_PIN, OUTPUT);
    pinMode(MOTOR_IN4_PIN, OUTPUT);

    Serial.println("[MOTOR] Initialized OK");
}

static void motor_stop()
{
    digitalWrite(MOTOR_IN1_PIN, LOW);
    digitalWrite(MOTOR_IN2_PIN, LOW);
    digitalWrite(MOTOR_IN3_PIN, LOW);
    digitalWrite(MOTOR_IN4_PIN, LOW);
    ledcWrite(0, 0);
    ledcWrite(1, 0);
}

static void motor_forward(uint8_t speed = MOTOR_SPEED_DEFAULT)
{
    digitalWrite(MOTOR_IN1_PIN, HIGH);
    digitalWrite(MOTOR_IN2_PIN, LOW);
    digitalWrite(MOTOR_IN3_PIN, HIGH);
    digitalWrite(MOTOR_IN4_PIN, LOW);
    ledcWrite(0, speed);
    ledcWrite(1, speed);
}

static void motor_rotate(uint8_t speed = MOTOR_SPEED_DEFAULT)
{
    // Quay tại chỗ: Motor A tiến, Motor B lùi
    digitalWrite(MOTOR_IN1_PIN, HIGH);
    digitalWrite(MOTOR_IN2_PIN, LOW);
    digitalWrite(MOTOR_IN3_PIN, LOW);
    digitalWrite(MOTOR_IN4_PIN, HIGH);
    ledcWrite(0, speed);
    ledcWrite(1, speed);
}

// ===================== BUZZER =====================

static void buzzer_beep(int count, int freq_hz, int duration_ms)
{
    // Dùng PWM channel 2 cho buzzer
    ledcSetup(2, freq_hz, 8);
    ledcAttachPin(BUZZER_PIN, 2);

    for (int i = 0; i < count; i++) {
        ledcWrite(2, 128);     // 50% duty cycle
        delay(duration_ms);
        ledcWrite(2, 0);
        if (i < count - 1) delay(100);
    }

    ledcDetachPin(BUZZER_PIN);
}

// ===================== AUDIO CAPTURE TASK =====================

/**
 * Task chạy trên Core 0: đọc I2S và fill vào slice_buffer
 * Mỗi slice = EI_CLASSIFIER_SLICE_SIZE samples (4000 @ 16kHz = 250ms)
 */
static void audio_capture_task(void *arg)
{
    const size_t samples_needed = EI_CLASSIFIER_SLICE_SIZE;
    int32_t raw_sample;  // INMP441 trả về 32-bit
    size_t bytes_read;

    while (true) {
        size_t samples_read = 0;

        while (samples_read < samples_needed) {
            i2s_read(I2S_PORT,
                     &raw_sample,
                     sizeof(int32_t),
                     &bytes_read,
                     portMAX_DELAY);

            if (bytes_read == sizeof(int32_t)) {
                // INMP441: dữ liệu 24-bit nằm ở MSB, shift xuống 16-bit
                slice_buffer[samples_read] = (int16_t)(raw_sample >> 16);
                samples_read++;
            }
        }

        // Báo hiệu slice đã sẵn sàng
        slice_ready = true;

        // Chờ main loop xử lý xong
        while (slice_ready) {
            vTaskDelay(1);
        }
    }
}

// ===================== EDGE IMPULSE CALLBACK =====================

/**
 * Callback cho Edge Impulse SDK: đọc dữ liệu audio từ slice_buffer
 */
static int get_audio_signal_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&slice_buffer[offset], out_ptr, length);
    return 0;
}

// ===================== COMMAND EXECUTION =====================

/**
 * Thực thi lệnh dựa trên kết quả nhận dạng
 */
static void execute_command(const char *label, float confidence)
{
    // Cooldown: tránh thực thi liên tục
    if (millis() - last_command_time < COMMAND_COOLDOWN_MS) {
        return;
    }
    last_command_time = millis();

    Serial.printf("[CMD] >>> %s (%.0f%%)\n", label, confidence * 100);

    if (strcmp(label, "Di") == 0) {
        Serial.println("[CMD] -> Tiến về phía trước");
        motor_forward();
        digitalWrite(LED_PIN, HIGH);
    }
    else if (strcmp(label, "Dung") == 0) {
        Serial.println("[CMD] -> Dừng lại");
        motor_stop();
        digitalWrite(LED_PIN, LOW);
    }
    else if (strcmp(label, "Quay") == 0) {
        Serial.println("[CMD] -> Quay tại chỗ");
        motor_rotate();
        digitalWrite(LED_PIN, HIGH);
    }
    else if (strcmp(label, "GiaKy") == 0) {
        Serial.println("[CMD] -> Xác nhận giọng nói (GiaKy)");
        buzzer_beep(3, 2000, 100);
        // Nhấp nháy LED
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(150);
            digitalWrite(LED_PIN, LOW);
            delay(150);
        }
    }
    // "Noise" -> bỏ qua, không làm gì
}

// ===================== SETUP =====================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("╔══════════════════════════════════════╗");
    Serial.println("║   Voice AI Mini Robot - ESP32        ║");
    Serial.println("║   Edge Impulse TinyML Inference      ║");
    Serial.println("╚══════════════════════════════════════╝");
    Serial.println();

    // LED & Buzzer
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Motor
    motor_init();
    motor_stop();

    // I2S Microphone
    i2s_init();

    // Thông tin model
    Serial.printf("[EI] Project:    %s\n", EI_CLASSIFIER_PROJECT_NAME);
    Serial.printf("[EI] Owner:      %s\n", EI_CLASSIFIER_PROJECT_OWNER);
    Serial.printf("[EI] Labels:     %d\n", EI_CLASSIFIER_LABEL_COUNT);
    Serial.printf("[EI] Frequency:  %d Hz\n", EI_CLASSIFIER_FREQUENCY);
    Serial.printf("[EI] Window:     %d ms\n",
                  (int)(EI_CLASSIFIER_RAW_SAMPLE_COUNT * 1000 / EI_CLASSIFIER_FREQUENCY));
    Serial.printf("[EI] Slice:      %d samples (%d ms)\n",
                  EI_CLASSIFIER_SLICE_SIZE,
                  (int)(EI_CLASSIFIER_SLICE_SIZE * 1000 / EI_CLASSIFIER_FREQUENCY));
    Serial.printf("[EI] Threshold:  %.0f%%\n", EI_CLASSIFIER_THRESHOLD * 100);
    Serial.printf("[EI] Arena:      %d bytes\n", EI_CLASSIFIER_TFLITE_LARGEST_ARENA_SIZE);
    Serial.println();

    // Khởi chạy task thu âm trên Core 0
    // (Core 1 dành cho loop() chạy inference)
    xTaskCreatePinnedToCore(
        audio_capture_task,     // Function
        "audio_capture",        // Name
        4096,                   // Stack size
        NULL,                   // Parameter
        1,                      // Priority
        NULL,                   // Task handle
        0                       // Core 0
    );

    // Beep báo hiệu sẵn sàng
    buzzer_beep(1, 1000, 200);
    Serial.println("[EI] ✓ Listening...\n");
}

// ===================== MAIN LOOP =====================

void loop()
{
    // Chờ slice audio mới từ Core 0
    if (!slice_ready) {
        delay(1);
        return;
    }

    // Tạo signal cho Edge Impulse
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = &get_audio_signal_data;

    // Chạy inference (continuous mode = sliding window)
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR err = run_classifier_continuous(&signal, &result, debug_nn);

    // Đánh dấu đã xử lý xong, cho phép thu âm tiếp
    slice_ready = false;

    if (err != EI_IMPULSE_OK) {
        Serial.printf("[ERROR] Classifier failed: %d\n", err);
        return;
    }

    // Tìm nhãn có confidence cao nhất
    float max_val = 0;
    int   max_idx = -1;

    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (result.classification[i].value > max_val) {
            max_val = result.classification[i].value;
            max_idx = i;
        }
    }

    // Thực thi nếu vượt ngưỡng và không phải Noise
    if (max_idx >= 0 && max_val >= EI_CLASSIFIER_THRESHOLD) {
        const char *label = result.classification[max_idx].label;
        if (strcmp(label, "Noise") != 0) {
            execute_command(label, max_val);
        }
    }

    // In thông tin timing (mỗi 20 lần để tránh spam Serial)
    static int print_count = 0;
    if (++print_count >= 20) {
        print_count = 0;
        Serial.printf("[Timing] DSP: %d ms | Inference: %d ms | ",
                      result.timing.dsp, result.timing.classification);
        for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            Serial.printf("%s: %.0f%% ",
                          result.classification[i].label,
                          result.classification[i].value * 100);
        }
        Serial.println();
    }
}
