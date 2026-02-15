#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "esp_timer.h"
#include "driver/rtc_io.h"

#define botonPin 0 // Pin del boton BOOT 
#define WAKEUP_GPIO GPIO_NUM_7 // pin para despertarlo
#define RGB_BUILTIN 48 // pin del LED RGBx

const int threshold = 4094;

// Pines para el sensor IR y el LED
const int irEmitterPin = 3;   // Pin del emisor IR
const int irReceiverPin = 4;  // Pin del receptor IR
const int ledPin = 8;         // Pin del LED

// Declaraciones y constantes
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
String receivedText = ""; // Para almacenar el texto recibido por BLE
uint8_t txValue = 0;      // Valor enviado por BLE

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UUID del servicio UART
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

hw_timer_t *timer = NULL;
volatile bool timerFlag = false; // Bandera para indicar que se disparó el timer

// ISR del timer
void IRAM_ATTR ISR_Timer() {
    timerFlag = true;
}

// Callbacks del servidor BLE
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) override {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer *pServer) override {
        deviceConnected = false;
    }
};

// Callbacks para manejar escritura en el RX
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        String rxValue = pCharacteristic->getValue();
        if (!rxValue.isEmpty()) {
            Serial.print("Recibido por BLE: ");
            Serial.println(rxValue.c_str()); // Imprime lo recibido por BLE en el monitor serie
        }
    }
};

void serialEvent2() {
    pTxCharacteristic->setValue("recibe infrarrojo\n");
    pTxCharacteristic->notify();
}

void serialEvent() {
    pTxCharacteristic->setValue("no recibe infrarrojo\n");
    pTxCharacteristic->notify();
}

// Configuración inicial
void setup() {
    pinMode(irEmitterPin, OUTPUT);
    pinMode(irReceiverPin, INPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(irEmitterPin, HIGH);
    pinMode(botonPin, INPUT_PULLUP);

    // Configura el pin de despertar
    rtc_gpio_deinit(WAKEUP_GPIO); // desactiva cualquiera
    rtc_gpio_pullup_dis(WAKEUP_GPIO);
    rtc_gpio_pulldown_en(WAKEUP_GPIO);
    esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 1); // Despierta cuando el pin 7 esté en HIGH

    Serial.begin(115200);
    Serial.println("Iniciando servicio UART BLE...");

    // Crear el dispositivo BLE
    BLEDevice::init("plaquita");

    // Crear el servidor BLE
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Crear el servicio BLE
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Crear la característica TX (Notificaciones hacia el móvil)
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());

    // Crear la característica RX (Recepción desde el móvil)
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());

    // Iniciar el servicio y la publicidad
    pService->start();
    pServer->getAdvertising()->start();

    Serial.println("Esperando conexión de cliente para notificar...");

    // Configuración del timer
    timer = timerBegin(1000000);              // Crea el timer con un prescaler de 80 (1 MHz)

  timerAttachInterrupt(timer, &ISR_Timer);  // Asocia la interrupción al timer
  
  timerAlarm(timer, 500000,true,0);         
}

// Bucle principal
void loop() {
    if (timerFlag) {
        timerFlag = false; // Reinicia la bandera

        int sensorValue = analogRead(irReceiverPin);

        // Mostrar el nivel en el monitor serie
        Serial.print("Nivel analógico: ");
        Serial.println(sensorValue);

        if (sensorValue < threshold) { // Señal recibida (no hay obstrucción)
            digitalWrite(ledPin, LOW); // Apaga el LED
            Serial.println("Recibe infrarrojo");
            serialEvent2();
            neopixelWrite(RGB_BUILTIN, 255, 0, 0); // Rojo
        } else { // Señal no recibida (hay obstrucción)
            digitalWrite(ledPin, HIGH); // Enciende el LED
            Serial.println("No recibe infrarrojo");

            // Llama a la rutina para manejar comunicación serie desde el PC
            serialEvent();
            neopixelWrite(RGB_BUILTIN, 0, 255, 0); // Verde
        }

        if (digitalRead(botonPin) == LOW) {
            esp_deep_sleep_start(); // Inicia el modo sleep profundo
        }

        // Si el dispositivo se desconecta, reiniciar publicidad
        if (!deviceConnected && oldDeviceConnected) {
            pServer->startAdvertising();
            Serial.println("Reiniciando publicidad...");
            oldDeviceConnected = deviceConnected;
        }

        // Si se conecta, actualizar el estado
        if (deviceConnected && !oldDeviceConnected) {
            Serial.println("Cliente conectado.");
            oldDeviceConnected = deviceConnected;
        }
    }
}
