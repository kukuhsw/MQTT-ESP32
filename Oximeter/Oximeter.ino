#include <PubSubClient.h>
#include <WiFi.h>

//Konfigurasi hotspot
const char* hostpot = ".................."; // Nama Hotspot 
const char* kunci = ".................."; // kata sandi
const char* username = "................."; // pengguna
const char* pubTopic = "oximeter/....."; // topik yang dikirim

const unsigned int interval = 2000; // selang waktu dalam milidetik

//Deklarasi broker
const char* mqtt_server = "smartlabs.com";
unsigned int mqtt_port = 1883;

//Deklarasi pin sensor
#define LBR_DATA 10
#define BTS_NAIK 11
#define KALIBRASI 0

WiFiClient askClient;
PubSubClient client(askClient);

void setup() {
Serial.begin(115200);
Serial.print("Mulai menghubungkan hotspot");
Serial.println(hostpot);

WiFi.begin(hostpot, kunci);

while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}


Serial.println("");
Serial.println("Terhubung hotspot");
Serial.println("Dengan IP: ");
Serial.println(WiFi.localIP());

client.setServer(mqtt_server, mqtt_port);
client.setCallback(callback);

}

void loop() {
float oxi_data, pertama, kedua, ketiga, sebelum;
float analog_Tab[LBR_DATA], ttl_analog;
float akhir, rataan_analog, awal;
bool naik;
int jml_naik, jlh_baca;
long int last_beat, sekarang, pointer;

// Cek koneksi hotspot
if (!client.connected()) 
reconnect();
client.loop();

// Inisialisasi variabel
for (int i = 0; i < LBR_DATA; i++) analog_Tab[i] = 0;
ttl_analog = 0;
pointer = 0;
while(1) {

// menghitung rata-rata sensor selama periode 20 ms untuk menghilangkan noise 50 Hz yang disebabkan oleh cahaya listrik
jlh_baca = 0;
awal = millis();
rataan_analog = 0.;
do {
rataan_analog += analogRead(A0);
jlh_baca++;
sekarang = millis();
}
while (sekarang < awal + 20); 
rataan_analog /= jlh_baca; // didapatkan rataan

// Tambahkan pengukuran terbaru ke array dan kurangi pengukuran awal dari array
// untuk mempertahankan jumlah pengukuran terakhir
ttl_analog -= analog_Tab[pointer];
ttl_analog += rataan_analog;
analog_Tab[pointer] = rataan_analog;
akhir = ttl_analog / LBR_DATA;

// sekarang memegang rata-rata nilai dalam array (tunggu kurva naik)
if (akhir > sebelum) {
jml_naik++;
if (!naik && jml_naik > BTS_NAIK) {
naik = true;
pertama = millis() - last_beat;
last_beat = millis();

// Hitung rata-rata rata-rata detak jantung menurut tiga detak terakhir
oxi_data = 60000. / (0.4 * pertama + 0.3 * kedua + 0.3 * ketiga)+KALIBRASI;
Serial.print(oxi_data);
Serial.println(" bpm \n"); // satuan denyut nadi

ketiga = kedua;
kedua = pertama;

Serial.println("publish data ke smartlabs");
char mqtt_payload[30] = "";
snprintf (mqtt_payload, 30, "m1=%f", oxi_data);
Serial.print("pesan dikirimkan");
Serial.println(mqtt_payload);
client.publish(pubTopic, mqtt_payload);
Serial.println("data denyut nadi terkirim");
delay(interval);// delay
}
}
else
{
// kurva menurun
naik = false;
jml_naik = 0;
}
sebelum = akhir;
pointer++;
pointer %= LBR_DATA;
}
}

// proses pengiriman MQTT 
void callback(char* topic, byte* payload, unsigned int length) {
Serial.print("Pesan telah terkirim [");
Serial.print(topic);
Serial.print("] ");
for (int i = 0; i < length; i++) {
Serial.print((char)payload[i]);
}
Serial.println();
}

void reconnect() {
// Ulangi sampai terkoneksi
while (!client.connected()) {
Serial.print("Mencari koneksi");
// Coba dapatkan koneksi
if (client.connect("ESP32Client", username, "")) { 
  Serial.println("Klien terkoneksi");
} else {
  Serial.print("gagal, rc=");
  Serial.print(client.state());
  Serial.println("ulang dalam 1 detik");
  // Tunggu selama 1 detik
  delay(1000);
}
}
}
