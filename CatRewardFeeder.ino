// ==============================
// 智能猫咪奖励饮水机 - ESP32-CAM 版
// 功能：检测猫头朝向 → 打开奖励仓
// ==============================

#include "esp_camera.h"
#include <ESP32Servo.h>
#include "model.h"          // 你的模型
#include "cat_model.h"      // 推理接口

// 舵机配置
Servo rewardServo;
const int SERVO_PIN = 32;
const int SERVO_OPEN = 90;   // 打开角度
const int SERVO_CLOSE = 0;   // 关闭角度

// 摄像头配置（OV2640）
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// 全局变量
bool isDrinking = false;

void setup() {
  Serial.begin(115200);
  Serial.println("=== 猫咪奖励饮水机启动 ===");

  // 初始化舵机
  rewardServo.setPeriodHertz(50);
  rewardServo.attach(SERVO_PIN, 500, 2400);
  rewardServo.write(SERVO_CLOSE);

  // 初始化摄像头
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size = FRAMESIZE_QQVGA; // 160x120
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("摄像头初始化失败: 0x%x", err);
    return;
  }

  // 初始化 AI 模型
  init_cat_model();

  Serial.println("系统就绪！");
}

void loop() {
  // 获取一帧图像
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("获取图像失败");
    delay(1000);
    return;
  }

  // 运行推理（输入：RGB565 图像，160x120）
  float confidence = run_inference(fb->buf, fb->width, fb->height);

  // 判断：置信度 > 0.8 且持续 2 帧
  static int drink_count = 0;
  if (confidence > 0.8) {
    drink_count++;
    if (drink_count >= 2 && !isDrinking) {
      isDrinking = true;
      Serial.println("✅ 检测到猫咪喝水！打开奖励仓...");
      rewardServo.write(SERVO_OPEN);
      delay(3000); // 保持3秒
      rewardServo.write(SERVO_CLOSE);
      isDrinking = false;
      drink_count = 0;
    }
  } else {
    drink_count = 0;
  }

  // 释放帧缓冲
  esp_camera_fb_return(fb);
  delay(1000); // 每秒检测一次（省电）
}
