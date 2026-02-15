# Proyecto ESP32S3

Proyecto realizado en Arduino IDE para prácticas de ingeniería informática.  
Sistema con ESP32-S3 que detecta señales infrarrojas y notifica el estado vía BLE.

## Hardware utilizado
- **Placa:** ESP32-S3
- **Emisor IR:** pin 3
- **Receptor IR:** pin 4
- **LED indicador:** pin 8
- **LED RGB integrado:** pin 48
- **Botón BOOT:** pin 0
- **Pin para despertar (ext0 wakeup):** GPIO 7

## Funcionalidades
- Detección de señal infrarroja con LED indicador.
- Notificaciones y recepción de datos vía BLE (UART service).
- Modo de sueño profundo controlado por botón.
- Timer para lectura periódica del sensor IR.
- LED RGB indica visualmente el estado de la señal IR.

## Librerías necesarias
- `BLEDevice.h`
- `BLEServer.h`
- `BLEUtils.h`
- `BLE2902.h`
- `esp_timer.h`
- `driver/rtc_io.h`

> Nota: Estas librerías se instalan desde **Arduino Library Manager** o ya vienen incluidas en el ESP32 core para Arduino.

## Cómo usar
1. Abrir `proyecto_ESP32S3.ino` en Arduino IDE.
2. Seleccionar la placa ESP32-S3 y el puerto correspondiente.
3. Subir el sketch al dispositivo.
4. Conectar un cliente BLE para recibir notificaciones y monitorear el sensor IR.

## Estado del proyecto
- Funcional y probado en ESP32-S3.
- Código optimizado para prácticas de ingeniería informática.
