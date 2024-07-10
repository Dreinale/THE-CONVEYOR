#include <M5Stack.h>
#include <Wire.h>
#include <driver/rmt.h>
#include <math.h>

#include "GoPlus2.h"
#include "MFRC522_I2C.h"

//wifi
#include <WiFi.h>

//Get & Post
#include <HTTPClient.h>
#include <ArduinoJson.h>


const char* ssid = "Elixir";
const char* password = "tuptup83";
const char* dolibarrApiUrl = "http://192.168.174.60/dolibarr/api/index.php/";
const char* dolibarrApiKey = "ZqbUlk5pNztLcH18DoYN67V8rF8eC770";

int warehouse_id = 0;

#define STEPMOTOR_I2C_ADDR 0x70
#define PIR_SENSOR_PIN 36

GoPlus2 goPlus;
MFRC522 mfrc522(0x28); // Instance de l'objet MFRC522

char destination = 'B'; // Déclaration globale de la destination

bool justPassed = false; // Ajout d'une nouvelle variable globale

unsigned long motorRestartTime = 0; // Temps pour redémarrer le moteur pas à pas

void setup() {
    M5.begin();
    M5.Power.begin();
    WiFi.begin(ssid, password);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setBrightness(100);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Conveyor System Ready");
    Wire.begin(21, 22, 100000UL); // SDA, SCL pour I2C
    Serial.begin(115200); // Démarrer la communication série
    mfrc522.PCD_Init(); // Initialiser le lecteur RFID
    pinMode(PIR_SENSOR_PIN, INPUT); // Définir le pin du capteur PIR comme entrée
}

void set_NFC() {
    // Afficher l'UID du tag NFC s'il est détecté
    M5.Lcd.fillRect(0, 20, 320, 20, BLACK); // Effacer la zone d'affichage précédente
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextColor(GREEN, BLACK);
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        Serial.println("No NFC tag detected");
        delay(200);
        return; // Sortie si aucun tag NFC n'est présent
    }
}

char read_NFC() {
    set_NFC();
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ") + String(mfrc522.uid.uidByte[i], HEX);
    }
    M5.Lcd.println(uid.c_str());
    Serial.print("UID read: ");
    Serial.println(uid);
    
    destination = uid.length() > 0 ? 'C' : 'B'; // Si UID est lu, destination 'C', sinon 'B'

    return destination;
}

void change_direction(char destination) {
    if (destination == 'B') {
        goPlus.Servo_write_angle(SERVO_NUM0, 135);
        Serial.println("Direction B activée.");
        M5.Lcd.setCursor(0, 100);
        M5.Lcd.printf("Changement de direction vers : %c", destination);
    } else if (destination == 'C') {
        goPlus.Servo_write_angle(SERVO_NUM0, 90);//110
        Serial.println("Direction C activée.");
        M5.Lcd.setCursor(0, 100);
        M5.Lcd.printf("Changement de direction vers : %c", destination);
    } else if (destination == 'A') {
        goPlus.Servo_write_angle(SERVO_NUM0, 150);
        Serial.println("Direction A activée.");
        M5.Lcd.setCursor(0, 100);
        M5.Lcd.printf("Changement de direction vers : %c", destination);
    }
    delay(1000);
}

void SendCommand(byte addr, String command) {
    char commandBuffer[128];
    command.toCharArray(commandBuffer, 128);
    Wire.beginTransmission(addr);
    Wire.write((const uint8_t *)commandBuffer, strlen(commandBuffer));
    Wire.write('\r');
    Wire.write('\n');
    Wire.endTransmission();
}

void motorMoveTo(int positionX) {
    // Envoyer une commande au moteur pas à pas pour se déplacer à la positionX
    String command = "G1 X" + String(positionX) + " F150";
    SendCommand(STEPMOTOR_I2C_ADDR, command);
}

void sendStockMovement(int productId, int warehouseId, int quantity) {
    HTTPClient http;
    http.begin(String(dolibarrApiUrl) + "stockmovements");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("DOLAPIKEY", dolibarrApiKey);

    DynamicJsonDocument doc(1024);
    doc["product_id"] = productId;
    doc["warehouse_id"] = warehouseId;
    doc["qty"] = quantity;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    if(httpResponseCode > 0) {
        String response = http.getString();
        //M5.lcd.setCursor(0, 90);
        //M5.lcd.print("POST OK ");
    } else {
        M5.lcd.print("Error on HTTP POST: ");
        M5.lcd.println(httpResponseCode);
    }
    http.end();
}

int getStockQuantity(int productId, int warehouseId) {
    HTTPClient http;
    http.begin(String(dolibarrApiUrl) + "products/" + String(productId) + "/stock?selected_warehouse_id=" + String(warehouseId));
    http.addHeader("DOLAPIKEY", dolibarrApiKey);

    int httpResponseCode = http.GET();
    if(httpResponseCode > 0) {
        String response = http.getString();

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, response);

        if (error) { // Vérifiez s'il y a une erreur dans la désérialisation
            M5.lcd.print("deserializeJson() failed: ");
            M5.lcd.println(error.c_str());
            return 2;
        }
        int realValue = doc["stock_warehouses"][String(warehouseId)]["real"].as<int>();

        // Affichez la valeur récupérée
        M5.lcd.setCursor(0, 130);
        M5.lcd.print("Real value: ");
        M5.lcd.println(realValue);
        return realValue;

    } else {
        M5.lcd.print("Error on HTTP GET: ");
        M5.lcd.println(httpResponseCode);
        return 2;
    }
    http.end();
}

void loop() {
    if (WiFi.isConnected()) {
            M5.lcd.setCursor(0, 20);
            M5.lcd.print("WiFi connected");
    }

    static int currentPosition = 0;
    static unsigned long lastMoveTime = millis();

    bool pirState = digitalRead(PIR_SENSOR_PIN);

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        destination = read_NFC();
        justPassed = false;
    }

    if (destination == 'B' && getStockQuantity(7, 2) >= 5) {
       destination = 'A';
    } else if (destination == 'C' && getStockQuantity(7, 3) >= 5) {
       destination = 'A';
    }

    M5.Lcd.setCursor(0, 60);
    M5.Lcd.printf("pirState: %d", pirState);
    M5.Lcd.setCursor(0, 80);
    M5.Lcd.printf("destination: %c", destination);

    if (pirState && !justPassed) {
        motorMoveTo(currentPosition); // Arrêter le moteur pas à pas
        change_direction(destination); // Changer la direction du servo moteur
        //delay(1000); // Laisser le temps au colis de s'arrêter
        justPassed = true;
        motorRestartTime = millis(); // Enregistrer le moment du redémarrage du moteur
        
        //post cargaison à la destination
        if(destination == 'B') {
            warehouse_id = 2;
        } else if(destination == 'C') {
            warehouse_id = 3;
        } else if(destination == 'A') {
            warehouse_id = 1;
        }
        sendStockMovement(7, warehouse_id, 1);
    }

    // Gestion du redémarrage et de l'arrêt du moteur pas à pas
    if (justPassed && millis() - motorRestartTime < 2000) {
        // Faire tourner le moteur pas à pas pendant 2 secondes
        motorMoveTo(currentPosition + 1);
    } else if (justPassed && millis() - motorRestartTime >= 2000) {
        // Arrêter le moteur pas à pas après 2 secondes
        justPassed = false;
        destination = 'B'; // Réinitialiser la destination
        change_direction(destination); // Réinitialiser la position du servo
    }

    // Mouvement continu du moteur pas à pas
    if (!justPassed && millis() - lastMoveTime >= 500) {
        currentPosition++;
        lastMoveTime = millis();
        motorMoveTo(currentPosition);
    }

    M5.update();
}