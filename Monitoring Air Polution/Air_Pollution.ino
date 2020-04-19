#include <PubSubClient.h>
#include <WiFi.h>

// Konfigurasi Hotspot
const char* ssid = “………………“;              // Nama hotspot
const char* password = “………………“;          // Sandi
const char* pengguna = “……………..“;         // nama pengguna
const char* topic = “pollution/...../“;      // publish/pengguna/apiKeyIn
const unsigned int interval = 25000;    // selang waktu dalam detik

//Konfigurasi broker
const char* mqtt_server = “smartlabs.com“;
unsigned int mqtt_port = 1883;

//Deklarasi pin sensor
#define AMBANG_CO 1000 // Ambang kadar udara segar
WiFiClient Klien;
PubSubClient client(Klien);

void setup() {
  Serial.begin(115200);
  
  Serial.print(“Proses koneksi ke hotspot“);
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(“.“);
}

Serial.println(“Telah terhubung hotspot“);
Serial.println(“Dengan IP:“);
Serial.println(WiFi.localIP());
client.setServer(mqtt_server, mqtt_port);
client.setCallback(callback);
}

void loop() {
if (!client.connected())
reconnect();
client.loop();
int Data_sensor = analogRead(A1);
if(Data_sensor < AMBANG_CO){
Serial.print(“Udara segar: “);
} else {
Serial.print(“Polusi: “);
}
Serial.print(Data_sensor); // data yang diperoleh dari pin analog MQ135
Serial.println(“ppm“); // Satuan ppm yaitu part per million
Serial.println(“publish data ke broker“);
char mqtt_payload[30] = ““;
snprintf (mqtt_payload, 30, “m1=%ld“, Data_sensor);
Serial.print(“pesan telah sampai“);
Serial.println(mqtt_payload);
client.publish(topic, mqtt_payload);
Serial.println(“data yang terkirim“);
delay(interval);// delay
}

void callback(char* topic, byte* payload, unsigned int length) {
Serial.print(“Pesan terkirim [“);
Serial.print(topic);
Serial.print(“] “);
for (int i = 0; i < length; i++) {
Serial.print((char)payload[i]);
}
Serial.println();
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
