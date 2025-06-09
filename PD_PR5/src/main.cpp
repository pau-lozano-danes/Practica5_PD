// PARTE 1


/*


 #include <Arduino.h> 
#include <Wire.h> 
void setup() 
{ 
  Wire.begin(21,20); 
  Serial.begin(115200); 
  while (!Serial);             // Leonardo: wait for serial monitor 
  Serial.println("\nI2C Scanner"); 
} 
void loop() 
{ 
  byte error, address; 
  int nDevices; 
  Serial.println("Scanning..."); 
  nDevices = 0; 
  for(address = 1; address < 127; address++ ) 
  { 
    // The i2c_scanner uses the return value of 
    // the Write.endTransmisstion to see if 
    // a device did acknowledge to the address. 
    Wire.beginTransmission(address); 
    error = Wire.endTransmission(); 
    if (error == 0) 
    { 
      Serial.print("I2C device found at address 0x"); 
      if (address<16) 
        Serial.print("0"); 
      Serial.print(address,HEX); 
      Serial.println("  !"); 
      nDevices++; 
    } 
    else if (error==4) 
    { 
      Serial.print("Unknown error at address 0x"); 
      if (address<16) 
        Serial.print("0"); 
      Serial.println(address,HEX); 
    }     
  } 
  if (nDevices == 0) 
    Serial.println("No I2C devices found\n"); 
  else 
    Serial.println("done\n"); 
  delay(5000);           // wait 5 seconds for next scan 
} 
  

*/


// PARTE 2
/*

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Definir las dimensiones del OLED y la dirección I2C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // No utilizamos un pin de reset
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200); // Inicializar comunicación serie

  // Inicializar la comunicación I2C en los pines SDA (21) y SCL (20) para ESP32
  Wire.begin(21, 20); // Usa los pines 21 (SDA) y 20 (SCL)

  // Inicializar el display OLED con la dirección I2C 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se pudo encontrar un display OLED"));
    for (;;); // Detener el programa si no se encuentra el display
  }

  // Limpiar la pantalla
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Mostrar mensaje inicial
  display.println(F("Escaneando dispositivos I2C..."));
  display.display();
  delay(1000);
}

void loop() {
  byte error, address;
  int nDevices = 0;
  
  display.clearDisplay(); // Limpiar la pantalla antes de mostrar resultados
  display.setCursor(0, 0); // Ubicar el cursor en la parte superior

  // Escanear direcciones I2C de 1 a 126
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      // Si se encuentra un dispositivo, mostrar su dirección en hexadecimal
      display.print(F("Dispositivo I2C en: 0x"));
      if (address < 16) display.print("0"); // Asegurarse de que la dirección tiene 2 dígitos
      display.println(address, HEX); // Mostrar dirección en formato hexadecimal
      nDevices++;
    } else if (error == 4) {
      // Error desconocido
      display.print(F("Error en: 0x"));
      if (address < 16) display.print("0");
      display.println(address, HEX);
    }
  }

  // Si no se encontraron dispositivos, mostrar mensaje correspondiente
  if (nDevices == 0) {
    display.println(F("No se encontraron dispositivos"));
  } else {
    display.println(F("Escaneo completado"));
  }

  display.display(); // Actualizar pantalla con los resultados
  delay(5000); // Esperar 5 segundos antes de hacer otro escaneo
}

*/



// PARTE EXTRA 1



/*

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

// Definir los pines I2C personalizados para ESP32-S3
#define SDA_PIN 21  
#define SCL_PIN 22

// Configuración OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor MAX30105
MAX30105 particleSensor;
uint32_t irBuffer[100], redBuffer[100];
int32_t spo2, heartRate;
int8_t validSPO2, validHeartRate;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);  // Configurar I2C

  // Iniciar pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Fallo en OLED SSD1306");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando...");
  display.display();

  // Iniciar sensor MAX30105
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("Sensor MAX30105 no encontrado");
    while (1);
  }

  // Configuración optimizada para MAX30105
  particleSensor.setup(69, 4, 2, 100, 411, 4096);  // Configuración optimizada para HR + SpO2
}

void loop() {
  // Leer 100 muestras del sensor
  for (byte i = 0; i < 100; i++) {
    while (!particleSensor.available()) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  // Calcular SpO2 y HR
  maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  // Mostrar en pantalla OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("HR: ");
  display.println(validHeartRate ? String(heartRate) + " BPM" : "N/A");
  display.print("SpO2: ");
  display.println(validSPO2 ? String(spo2) + " %" : "N/A");
  display.display();

  Serial.print("HR: ");
  Serial.print(validHeartRate ? String(heartRate) + " BPM" : "N/A");
  Serial.print(" | SpO2: ");
  Serial.println(validSPO2 ? String(spo2) + " %" : "N/A");

  delay(1000);
}
  

*/








// PARTE EXTRA 2



/*



#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

// Configuración WiFi
const char* ssid = "Nautilus";     
const char* password = "20000Leguas"; 

// Servidor Web
WiFiServer server(80);

// Configuración OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor MAX30105
MAX30105 particleSensor;
uint32_t irBuffer[100], redBuffer[100];
int32_t spo2, heartRate;
int8_t validSPO2, validHeartRate;

// Pines I2C para ESP32-S3
#define SDA_PIN 21 
#define SCL_PIN 22 

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Conexión a WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  server.begin();

  // Iniciar OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Fallo en OLED");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando...");
  display.display();

  // Iniciar sensor MAX30105
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("Sensor MAX30105 no encontrado");
    while (1);
  }
  particleSensor.setup(69, 4, 2, 100, 411, 4096);
}

void loop() {
  // Manejar clientes web
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    // Enviar datos al cliente
    String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    response += "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='1'>";
    response += "<style>body{font-family:Arial;text-align:center;} .data{font-size:2em;color:blue;}</style>";
    response += "</head><body>";
    response += "<h2>Monitor de Frecuencia Cardiaca y SpO2</h2>";
    response += "<p>Frecuencia Cardiaca: <span class='data'>" + String(validHeartRate ? String(heartRate) + " BPM" : "N/A") + "</span></p>";
    response += "<p>SpO2: <span class='data'>" + String(validSPO2 ? String(spo2) + " %" : "N/A") + "</span></p>";
    response += "</body></html>";

    client.print(response);
    client.stop();
  }

  // Leer datos del sensor
  for (byte i = 0; i < 100; i++) {
    while (!particleSensor.available()) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  // Mostrar en OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("HR: ");
  display.println(validHeartRate ? String(heartRate) + " BPM" : "N/A");
  display.print("SpO2: ");
  display.println(validSPO2 ? String(spo2) + " %" : "N/A");
  display.display();

  // Mostrar en Monitor Serie
  Serial.print("HR: ");
  Serial.print(validHeartRate ? String(heartRate) + " BPM" : "N/A");
  Serial.print(" | SpO2: ");
  Serial.println(validSPO2 ? String(spo2) + " %" : "N/A");

  delay(1000);
}
  


*/

