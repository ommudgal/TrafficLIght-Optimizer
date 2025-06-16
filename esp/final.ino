#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

const char* ssid = "MAPL";
const char* password = "ommudgal";

#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

httpd_handle_t stream_httpd = NULL;
int vehicleCount = 0;

static esp_err_t capture_image_handler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

  esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);

  esp_camera_fb_return(fb);

  return res;
}

static esp_err_t vehicle_count_handler(httpd_req_t *req) {
  char content[100];
  int content_len = req->content_len;
  
  if (content_len > sizeof(content) - 1) {
    content_len = sizeof(content) - 1;
  }

  int ret = httpd_req_recv(req, content, content_len);
  if (ret <= 0) {
    return ESP_FAIL;
  }

  content[ret] = '\0';

  String vehicle_count_str(content);
  int count_pos = vehicle_count_str.indexOf("count=");
  if (count_pos != -1) {
    String count = vehicle_count_str.substring(count_pos + 6);
    vehicleCount = count.toInt();
    String ipAddress = WiFi.localIP().toString();
    String dataToSend = String(vehicleCount) + "," + ipAddress;
    Serial.println(dataToSend);
  }

  httpd_resp_send(req, "Vehicle count received", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t capture_uri = {
    .uri       = "/capture",
    .method    = HTTP_GET,
    .handler   = capture_image_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t vehicle_count_uri = {
    .uri       = "/vehicle_count",
    .method    = HTTP_POST,
    .handler   = vehicle_count_handler,
    .user_ctx  = NULL
  };

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &capture_uri);
    httpd_register_uri_handler(stream_httpd, &vehicle_count_uri);
  }
}

void clearCameraResources() {
  esp_camera_deinit();
  Serial.println("Camera resources cleared");
}

void cameraTask(void *pvParameters) {
  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      vTaskDelay(500 / portTICK_PERIOD_MS);
      continue;
    }

    esp_camera_fb_return(fb);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void sendDefaultValuesToArduino() {
  String ipAddress = WiFi.localIP().toString();
  String dataToSend = String(vehicleCount) + "," + ipAddress;
  Serial.println(dataToSend);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting to WiFi...");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  sendDefaultValuesToArduino();

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_HD;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  delay(1000);
  sendDefaultValuesToArduino();
  Serial.println("Camera initialized successfully");

  startCameraServer();

  xTaskCreate(cameraTask, "Camera Task", 8192, NULL, 1, NULL);
}

void loop() {
  delay(5000);
}
