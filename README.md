# Práctica 5: Buses de Comunicación I (Introducción y I2C)

## Descripción
El objetivo de esta práctica es comprender el funcionamiento de los buses de comunicación entre periféricos, en particular el protocolo **I2C (Inter-Integrated Circuit)**. Este protocolo serie permite la conexión de múltiples dispositivos utilizando solo dos líneas:
- **SDA (Serial Data Line):** Transporta los datos.
- **SCL (Serial Clock Line):** Proporciona la señal de sincronización.

Su arquitectura maestro-esclavo permite que un dispositivo principal controle varios periféricos mediante direcciones únicas, facilitando la comunicación con sensores, pantallas y otros módulos electrónicos.

---

## Ejercicio Práctico 1: Escáner I2C

### Descripción
Se implementa un escáner I2C que detecta los dispositivos conectados al bus y muestra sus direcciones en el monitor serie.

### Código:
```cpp
#include <Wire.h>

void setup() {
  Wire.begin(21, 20); // SDA, SCL
  Serial.begin(115200);
  while (!Serial); // Esperar al puerto serie
  Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
  }

  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000); // Esperar 5 segundos para volver a escanear
}
```

---

## Ejercicio Práctico 2

### Descripción
Se modifica el código anterior para mostrar las direcciones detectadas en una pantalla OLED SSD1306.

### Código:
```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Wire.begin(21, 20); // SDA, SCL
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  byte error, address;
  int nDevices = 0;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Escaner I2C:");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      display.print("Dispositivo: 0x");
      if (address < 16) display.print("0");
      display.println(address, HEX);
      nDevices++;
    }
  }

  if (nDevices == 0)
    display.println("No encontrados.");
  else
    display.println("Escaneo completo.");

  display.display();
  delay(5000);
}
```

---

## Ejercicio Práctico 3

### Descripción
Se muestra un mensaje personalizado en la pantalla OLED.

### Codigo

```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Wire.begin(21, 20);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("Hola Palma");
  display.display();
}

void loop() {
  // Mensaje fijo
}
```

---

## Ejercicio de Subida de Nota

### Parte 1

### Descripcion
Se configura el ESP32-S3 para medir la frecuencia cardíaca y la saturación de oxígeno utilizando el sensor MAX30105 y mostrar los resultados en la pantalla OLED.

### Codigo 
```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Wire.begin(21, 20);
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("Sensor MAX30105 no encontrado.");
    while (1);
  }

  particleSensor.setup();
}

void loop() {
  const int BUFFER_SIZE = 100;
  uint32_t irBuffer[BUFFER_SIZE];
  uint32_t redBuffer[BUFFER_SIZE];

  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (particleSensor.available() == false);
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  int32_t spo2;
  int8_t validSPO2;
  int32_t heartRate;
  int8_t validHeartRate;

  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  Serial.print("HR: "); Serial.print(heartRate);
  Serial.print(" bpm / SpO2: "); Serial.print(spo2); Serial.println(" %");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("HR: "); display.print(heartRate); display.println(" bpm");
  display.print("SpO2: "); display.print(spo2); display.println(" %");
  display.display();

  delay(1000);
}
```

### Parte 2

### Descripcion
Se añade conectividad WiFi para visualizar los datos a través de un servidor web en tiempo real.

### Codigo

```cpp
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

const char* ssid = "tuSSID";
const char* password = "tuPASSWORD";

WiFiServer server(80);

MAX30105 particleSensor;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int heartRate = 0;
int spo2 = 0;

void setup() {
  Wire.begin(21, 20);
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("Sensor MAX30105 no encontrado.");
    while (1);
  }

  particleSensor.setup();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  obtenerDatos();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("HR: "); display.print(heartRate); display.println(" bpm");
  display.print("SpO2: "); display.print(spo2); display.println(" %");
  display.display();

  WiFiClient client = server.available();
  if (client) {
    String response = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='1'/>";
    response += "<title>Monitor Salud</title></head><body><h1>Monitor Salud</h1>";
    response += "<p>HR: " + String(heartRate) + " bpm</p>";
    response += "<p>SpO2: " + String(spo2) + " %</p></body></html>";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(response);
    client.stop();
  }
}

void obtenerDatos() {
  const int BUFFER_SIZE = 100;
  uint32_t irBuffer[BUFFER_SIZE];
  uint32_t redBuffer[BUFFER_SIZE];

  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (particleSensor.available() == false);
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  int8_t validHeartRate, validSpO2;

  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer, &spo2, &validSpO2, &heartRate, &validHeartRate);
}
```
