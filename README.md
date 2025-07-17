# 🎵 IKEA OBEGRÄNSAD LED Panel - FFT Audio Visualizer (ESP32)

Real-time audio frequency visualizer for the IKEA OBEGRÄNSAD 16x16 LED matrix panel using an ESP32 and I2S microphone (e.g., INMP441). This project uses FFT (Fast Fourier Transform) to analyze audio input and display a smooth frequency spectrum across the LED panel.

---

## 📸 Demo


---

## 🛠 Features

- 🧠 **ESP32-based** control
- 🎤 **I2S Microphone input** (mono)
- 🔍 **FFT Audio Processing** (arduinoFFT library)
- 🎚️ **Smoothing** and decibel scaling
- 🔲 Horizontal bar display using 256-bit LED buffer
- 💡 Adjustable brightness and PWM frequency

---

## ⚙️ Hardware Required

| Component                | Description                                 |
|-------------------------|---------------------------------------------|
| ESP32 Dev Board         | e.g., WROOM-32                               |
| IKEA OBEGRÄNSAD Panel   | 16x16 LED matrix (custom wiring)             |
| INMP441 or SPH0645 Mic  | I2S digital MEMS microphone                  |
| Power Supply            | 5V 2A recommended                            |
| Jumper Wires            | For connections                              |

---

## 🧾 Pin Configuration

### 🔌 ESP32 to Panel

| Panel Signal | ESP32 Pin |
|--------------|-----------|
| `EN`         | GPIO 26   |
| `DI`         | GPIO 27   |
| `CLK`        | GPIO 14   |
| `CLA`        | GPIO 12   |

### 🎤 I2S Microphone

| I2S Mic Pin | ESP32 Pin |
|-------------|-----------|
| `WS`        | GPIO 25   |
| `SD`        | GPIO 33   |
| `SCK`       | GPIO 22   |

---

## 📦 Installation (via PlatformIO)

1. **Clone the repository**
   ```bash
   git clone https://github.com/ShivaMakes/obegransad.git
   cd obegransad
