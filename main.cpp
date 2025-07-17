#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>

// === Panel Pins ===
#define TT     5
#define P_EN   26
#define P_DI   27
#define P_CLK  14
#define P_CLA  12

// === Mic Pins ===
#define I2S_WS  25
#define I2S_SD  33
#define I2S_SCK 22

// === Audio FFT Settings ===
#define SAMPLE_SIZE 512
#define SAMPLE_RATE 16000

double vReal[SAMPLE_SIZE];
double vImag[SAMPLE_SIZE];
ArduinoFFT<double> FFT;

// === LED Panel Setup ===
uint8_t p_buf[256];
const int lut[16][16] = {
  {23,22,21,20,19,18,17,16, 7,6,5,4,3,2,1,0},
  {24,25,26,27,28,29,30,31, 8,9,10,11,12,13,14,15},
  {39,38,37,36,35,34,33,32,55,54,53,52,51,50,49,48},
  {40,41,42,43,44,45,46,47,56,57,58,59,60,61,62,63},
  {87,86,85,84,83,82,81,80,71,70,69,68,67,66,65,64},
  {88,89,90,91,92,93,94,95,72,73,74,75,76,77,78,79},
  {103,102,101,100,99,98,97,96,119,118,117,116,115,114,113,112},
  {104,105,106,107,108,109,110,111,120,121,122,123,124,125,126,127},
  {151,150,149,148,147,146,145,144,135,134,133,132,131,130,129,128},
  {152,153,154,155,156,157,158,159,136,137,138,139,140,141,142,143},
  {167,166,165,164,163,162,161,160,183,182,181,180,179,178,177,176},
  {168,169,170,171,172,173,174,175,184,185,186,187,188,189,190,191},
  {215,214,213,212,211,210,209,208,199,198,197,196,195,194,193,192},
  {216,217,218,219,220,221,222,223,200,201,202,203,204,205,206,207},
  {231,230,229,228,227,226,225,224,247,246,245,244,243,242,241,240},
  {232,233,234,235,236,237,238,239,248,249,250,251,252,253,254,255}
};

const int PWM_CHANNEL = 0;
const int PWM_FREQ = 5000;
const int PWM_RES = 8;

const uint16_t bin_limits[17] = {1, 40, 50, 60, 65, 70,	75,	80, 95,	105, 115,	125,	135,	145,	155,	165,	169
};
float smoothed[16] = {0};
float alpha = 0.4;

void p_init() {
  pinMode(P_EN, OUTPUT);
  pinMode(P_DI, OUTPUT);
  pinMode(P_CLK, OUTPUT);
  pinMode(P_CLA, OUTPUT);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(P_EN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 250);
}

void p_clear() {
  memset(p_buf, 0, sizeof(p_buf));
}

void p_drawPixel(int8_t x, int8_t y, uint8_t c) {
  if (x < 0 || x > 15 || y < 0 || y > 15) return;
  p_buf[lut[y][x]] = c;
}

void p_scan(uint8_t cm) {
  ledcWrite(PWM_CHANNEL, 255);
  delayMicroseconds(TT);
  for (int i = 0, w = 0; i < 256; i++) {
    digitalWrite(P_DI, (cm & p_buf[w++]) ? HIGH : LOW);
    digitalWrite(P_CLK, HIGH);
    digitalWrite(P_CLK, LOW);
  }
  digitalWrite(P_CLA, HIGH);
  digitalWrite(P_CLA, LOW);
  ledcWrite(PWM_CHANNEL, 250);
}

void setupI2SMic() {
  i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = SAMPLE_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}
void setup() {
  Serial.begin(115200);
  p_init();
  setupI2SMic();
  Serial.println("FFT Mic Panel Ready");
}
void loop() {
  int16_t rawData[SAMPLE_SIZE];
  size_t bytesRead;
  i2s_read(I2S_NUM_0, &rawData, sizeof(rawData), &bytesRead, portMAX_DELAY);
  int sampleCount = bytesRead / sizeof(int16_t);

  for (int i = 0; i < sampleCount; i++) {
    vReal[i] = (double)rawData[i];
    vImag[i] = 0.0;
  }

  FFT.windowing(vReal, SAMPLE_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLE_SIZE, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLE_SIZE);

  p_clear();

  for (uint8_t y = 0; y < 16; y++) {
    double sum = 0;
    int count = 0;
    for (int j = bin_limits[y]; j < bin_limits[y + 1]; j++) {
      sum += vReal[j];
      count++;
    }
    double avg = count > 0 ? sum / count : 0;
    if (avg < 5.0) avg = 0;  // silence threshold (adjust if needed)
    float db = 20.0 * log10(avg + 1.0);
    db = constrain(db, 20, 80);  // ignore anything below 20 dB
    float h = (db - 20) / 60.0 * 8.0;  // rescale
    smoothed[y] = alpha * h + (1.0 - alpha) * smoothed[y];

    uint8_t height = (uint8_t)smoothed[y];
    height = constrain(height, 0, 8);
    for (uint8_t x = 0; x < height; x++) {
      p_drawPixel(x, y, 0xFF);  // horizontal bar, grows rightward
    }
  }

  p_scan(1);
  delay(50);
}
