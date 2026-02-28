#include "cat_model.h"
#include "model.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// 全局变量
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// 内存池（根据模型大小调整）
constexpr int kTensorArenaSize = 128 * 1024;  // 128KB
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

void init_cat_model() {
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("模型版本不匹配");
    return;
  }

  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("内存分配失败");
    return;
  }

  input = interpreter->input(0);
  output = interpreter->output(0);
}

float run_inference(const uint8_t* rgb565_buf, int width, int height) {
  // 转换 RGB565 → 模型输入格式（假设是 RGB888 归一化到 [0,1]）
  float* input_data = input->data.f;
  for (int i = 0; i < width * height; i++) {
    uint16_t pixel = ((uint16_t*)rgb565_buf)[i];
    uint8_t r = (pixel >> 11) & 0x1F;
    uint8_t g = (pixel >> 5) & 0x3F;
    uint8_t b = pixel & 0x1F;
    // 归一化到 [0,1]
    input_data[i*3 + 0] = r / 31.0f;
    input_data[i*3 + 1] = g / 63.0f;
    input_data[i*3 + 2] = b / 31.0f;
  }

  // 运行推理
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    error_reporter->Report("推理失败");
    return 0.0f;
  }

  // 假设输出是单个概率值（如 is_drinking）
  return output->data.f[0];
}
