# CatRewardFeeder-ESP32CAM
智能猫咪饮水奖励系统：用 ESP32-CAM 识别猫是否在喝水，自动打开奖励仓（MG996R 舵机）

## 功能
- 摄像头安装在水盆正上方
- 实时检测猫头位置与朝向
- 若判定为“正在喝水”，舵机旋转打开奖励仓（如零食盒）
- 低功耗设计（可配合 PIR 唤醒）

## 硬件清单
- ESP32-CAM（AI-Thinker 版）
- MG996R 舵机
- 外部 6V 电源（4节AA电池或6V 2A电源）
- 杜邦线、面包板

## 接线图
+------------------+       +------------------+
|   ESP32-CAM      |       |   MG996R 舵机     |
|                  |       |                  |
|  5V (VIN)  <-----+-------+  外部 6V (+)     |
|  GND       <-----+-------+  外部 6V (-)     |
|  GPIO 32   ------+------>+  Signal (橙线)   |
+------------------+       +------------------+

> ⚠️ 必须共地！舵机不可由 ESP32 供电！

## 使用步骤
1. 将你的 PyTorch 模型 (`catDetect.pth`) 转为 TensorFlow Lite（见 `model_conversion/`）
2. 生成 `model.h`
3. 安装 Arduino 库：
   - ESP32 by Espressif
   - ESP32Servo
   - TensorFlow Lite for Microcontrollers
4. 上传 `CatRewardFeeder.ino`
5. ⚠️ 需要安装 TensorFlow Lite for Microcontrollers 库（通过 Arduino Library Manager 搜索安装）
