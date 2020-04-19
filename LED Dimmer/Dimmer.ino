#include <PubSubClient.h>
#include <Wire.h>
#include <WiFi.h>

//Konfigurasi hotspot
const char* ssid = "..............";          // Nama hotspot
const char* password = "..............";      // Sandi

// deklarasi pengguna dan topik
const char* pengguna = "..............";      // nama pengguna
const char* topik = "dimmer/......./.....";   // publish/pengguna/api


// Deklarasi pin dan PWM
const int led_PWM = 5;
const int frek_PWM = 5000;
const int channel_PWM = 0;
const int resolusi_PWM = 8;
float pwm_cmd = 0;

// Deklarasi MQTT broker
const char* mqtt_server = "smartlabs.com";
unsigned int mqtt_port = 1883;

// Konfigurasi MQTT klien
WiFiClient askClient;

PubSubClient client(askClient);

void setup() {

Serial.begin(115200);

Serial.println("LED DIMMER");
ledcSetup(channel_PWM, frek_PWM, resolusi_PWM); // Pengaturan PWM
ledcAttachPin(, channel_PWM);
Serial.print("Menghubungkan hotspot");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}

Serial.println(“Telah terhubung hotspot“);
Serial.println(“Dengan IP:“);
Serial.println(WiFi.localIP());
client.setServer(mqtt_server, mqtt_port);
client.setCallback(callback);

if (!client.connected())
reconnect();
Serial.print("Subscribe data dari broker");
Serial.print(topik);
// susbscribe
client.subscribe(topik);
}

void loop() {
client.loop();
ledcWrite(channel_PWM, pwm_cmd);
}

void callback(char* topic, byte* payload, unsigned int length) {
Serial.print("\n Perintah dari broker[");
Serial.print(topic);
Serial.print("] ");

char command_str[8];
char module_str[length-8];

Serial.print("\nModule: ");

for (int i = 0; i < 8; i++) {
module_str[i] = (char)payload[i];
Serial.print((char)module_str[i]);

}

Serial.print("\n perintah: ");
for (int i = 0; i < length-8; i++) {
command_str[i] = (char)payload[i+8];
Serial.print((char)command_str[i]);

}

Serial.println("Mengontrol aktuator dengan perintah");
if(strncmp((char *)module_str, (char*) "module1=",8)== 0){
pwm_cmd = (float)atoi((char*)command_str);

// konversi dari max = 100% PWM ke output 8-bit
pwm_cmd *= 255.0/100;

Serial.print("Rasio PWM");
Serial.print(pwm_cmd);
Serial.println("%");
}

}

void reconnect() {

// Ulangi sampai mendapatkan koneksi
while (!client.connected()) {
Serial.print(“proses menerima koneksi“);
// Mencoba untuk terhubung broker
if (client.connect(“ESP32Client“, pengguna, ““)) {
  Serial.println(“Klien terkoneksi“);
} else {
  Serial.print(“gagal, rc=“);
  Serial.print(client.state());
  Serial.println(“coba ulang 5 detik“);
  // Tunggu jeda 5 detik
  delay(5000);
}
}

}
