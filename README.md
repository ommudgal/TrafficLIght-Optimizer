# Traffic Light Optimizer

A smart traffic light control system using YOLOv3 object detection, ESP32-CAM, and Arduino UNO. The system detects vehicles in real-time and dynamically adjusts traffic light timings to optimize traffic flow.

## Components

- **ESP32-CAM**: Captures images and serves them over HTTP.
- **Python Script**: Runs YOLOv3 to detect vehicles in images from ESP32-CAM and sends vehicle count back to ESP32.
- **Arduino UNO**: Receives vehicle count and ESP32 IP, controls traffic lights (LEDs) and displays info on an LCD.

## Directory Structure

```
TrafficLIght-Optimizer/
├── Python/
│   ├── main.py         # Python script for vehicle detection and communication
│   ├── yolov3.cfg      # YOLOv3 configuration file
│   ├── yolov3.weights  # YOLOv3 weights file (not included, download separately)
│   └── coco.names      # COCO class labels
├── Uno/
│   └── UNO.ino         # Arduino UNO code for traffic light control
├── esp/
│   └── final.ino       # ESP32-CAM firmware for image capture and HTTP server
```

## Setup

### 1. ESP32-CAM

- Flash `final.ino` to your ESP32-CAM.
- Update WiFi credentials in the code.
- Connect ESP32-CAM to your network.
- The ESP32 will serve images at `http://<ESP32_IP>/capture` and accept vehicle counts at `/vehicle_count`.

### 2. Python Environment

- Install dependencies:
  ```
  pip install opencv-python numpy pillow requests
  ```
- Download `yolov3.weights` from the official YOLO website and place it in the `Python/` directory.
- Edit `ESP32_IP` in `main.py` to match your ESP32's IP address.
- Run the script:
  ```
  python main.py
  ```

### 3. Arduino UNO

- Upload `UNO.ino` to your Arduino UNO.
- Connect LEDs to pins A3 (Red), A4 (Yellow), A5 (Green).
- Connect a 16x2 LCD as per the pin definitions in the code.
- Connect Arduino UNO to ESP32-CAM via serial.

## Arduino UNO Pin Definitions

**LEDs:**
- Red LED: `A3`
- Yellow LED: `A4`
- Green LED: `A5`

**16x2 LCD:**
- RS: `12`
- EN: `11`
- D4: `5`
- D5: `4`
- D6: `3`
- D7: `2`

Refer to `Uno/UNO.ino` for exact wiring.

## How It Works

1. ESP32-CAM captures images and serves them over HTTP.
2. Python script fetches images, detects vehicles using YOLOv3, and sends the count to ESP32.
3. ESP32 forwards the vehicle count and its IP to Arduino UNO via serial.
4. Arduino UNO adjusts traffic light timings based on vehicle count and displays info on the LCD.

## Notes

- Ensure all devices are on the same WiFi network.
- Adjust pin numbers and timings as needed for your hardware setup.
- The YOLOv3 weights file is large and must be downloaded separately.

## License

For educational and research purposes only.
