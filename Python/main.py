import requests
import time
import cv2
import numpy as np
from PIL import Image
from io import BytesIO

ESP32_IP = 'http://192.168.0.100'

net = cv2.dnn.readNet("yolov3.weights", "yolov3.cfg")
layer_names = net.getLayerNames()
output_layers = [layer_names[i - 1] for i in net.getUnconnectedOutLayers()]

def fetch_image():
    try:
        response = requests.get(f"{ESP32_IP}/capture")
        if response.status_code == 200:
            image = Image.open(BytesIO(response.content))
            return np.array(image)
        else:
            print("Failed to get image from ESP32. Status code:", response.status_code)
            return None
    except Exception as e:
        print("Error fetching image:", e)
        return None

def detect_vehicles(image):
    blob = cv2.dnn.blobFromImage(image, 0.00392, (416, 416), (0, 0, 0), True, crop=False)
    net.setInput(blob)
    outs = net.forward(output_layers)
    class_ids = []
    confidences = []
    boxes = []
    vehicle_count = 0
    height, width, _ = image.shape

    for out in outs:
        for detection in out:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]
            if confidence > 0.5:
                if class_id in [2, 5, 7]:
                    center_x = int(detection[0] * width)
                    center_y = int(detection[1] * height)
                    w = int(detection[2] * width)
                    h = int(detection[3] * height)
                    x = int(center_x - w / 2)
                    y = int(center_y - h / 2)
                    boxes.append([x, y, w, h])
                    confidences.append(float(confidence))
                    class_ids.append(class_id)

    indices = cv2.dnn.NMSBoxes(boxes, confidences, 0.5, 0.4)
    if len(indices) > 0:
        vehicle_count = len(indices)

    return vehicle_count

def send_vehicle_count(vehicle_count):
    try:
        response = requests.post(f"{ESP32_IP}/vehicle_count", data=f"count={vehicle_count}")
        if response.status_code == 200:
            print(f"Sent vehicle count: {vehicle_count} to ESP32")
        else:
            print("Failed to send vehicle count. Status code:", response.status_code)
    except Exception as e:
        print("Error sending vehicle count:", e)

def main():
    while True:
        print("Fetching image from ESP32...")
        image = fetch_image()
        if image is not None:
            vehicle_count = detect_vehicles(image)
            print(f"Number of vehicles detected: {vehicle_count}")
            send_vehicle_count(vehicle_count)
        
        time.sleep(10)

if __name__ == "__main__":
    main()
