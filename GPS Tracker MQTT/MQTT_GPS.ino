#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <TinyGPS++.h>

//konfigurasi hotspot 
const char* ssid = "..................";      // Masukkan SSID
const char* password = "..................";  // Masukkan Password

//
const char* pengguna = "................."; // my AskSensors pengguna
const char* pubTopic = "publish/..../....."; // publish/pengguna/apiKeyIn
const unsigned int writeInterval = 25000; // write interval (in ms)

static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;

//Konfigurasi MQTT
const char* mqtt_server = "smartlabs.com";
unsigned int mqtt_port = 1883;

// deklarasi perangkat
WiFiClient Klien;
PubSubClient client(Klien);
SoftwareSerial sose(RXPin, TXPin); // The serial connection to the GPS device
TinyGPSPlus gps; // The TinyGPS++ object

// persiapan perangkat
void setup() {
//hubungkan perangkat hotspot
Serial.begin(115200);
Serial.println(ssid);
WiFi.begin(ssid, password);

//check status
while (WiFi.status() != WL_CONNECTED) {
delay(100);
Serial.print(".");
}

Serial.println("Terhubung Hotspot");
Serial.println("IP : ");
Serial.println(WiFi.localIP());

client.setServer(mqtt_server, mqtt_port);
client.setCallback(callback);

// Baudrate GPS
sose.begin(GPSBaud);

}

// perulangan program
void loop() {
  
//Cek koneksi perangkat
if (!client.connected()) 
reconnect();
client.loop();

// Sketsa ini menampilkan informasi setiap kali kalimat baru dikodekan dengan benar.
while (sose.available() > 0)
if (gps.encode(sose.read()))
displayInfo();

if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("Perangkat GPS tak terdeteksi, Periksa pemasangan perangkat"));
    while(true);
  }
}

// Info tampilan GPS 
void displayInfo() {

//cek keakuratan posisi GPS
if (gps.location.isValid()) {
double lintang = (gps.location.lat());
double bujur = (gps.location.lng());

//
Serial.println("Publish data GPS");
char mqtt_payload[50] = "";
snprintf (mqtt_payload, 50, "m1=%lf;%lf", lintang, bujur);
Serial.print("pesan yang dipublish: ");
Serial.println(mqtt_payload);
client.publish(pubTopic, mqtt_payload);
Serial.println(" Data MQTT terpublikasi");

//jeda
delay(writeInterval); 
} else {
  Serial.println(F("Tidak Valid"));
}

}

//panggilan ulang MQTT 
void callback(char* topic, byte* payload, unsigned int length) {
Serial.print("Pesan diterima <");
Serial.print(topic);
Serial.print(">");
for (int i = 0; i < length; i++) {
Serial.print((char)payload[i]);
}
Serial.println();
}

//menghubungkan kembali MQTT
void reconnect() {
// Ulang sampai terkoneksi
while (!client.connected()) {
Serial.print("Berhasil mendapat koneksi");

// Coba sambungkan
if (client.connect("ESP32Client", pengguna, "")) { 
Serial.println("Klien MQTT terhubung");
} else {
Serial.print("failed, rc=");
Serial.print(client.state());
Serial.println("coba lagi 6 detik");
delay(6000);
}
}
}
