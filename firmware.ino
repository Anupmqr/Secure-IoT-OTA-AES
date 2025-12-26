#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Updater.h>
#include <AESLib.h>
#include <SHA256.h>
#include <EEPROM.h>

#define VER 33
#define E_SZ 64
#define B_ADDR 0
#define V_ADDR 1
#define LED 2

unsigned long lt = 0;
bool st = 0;
int fSz = 0; // Store firmware size

const char* ssid = "YOUR_WIFI_SSID";
const char* pass = "YOUR_WIFI_PASSWORD";
const char* fUrl = "http://YOUR_SERVER_IP:PORT/firmware";
const char* mUrl = "http://YOUR_SERVER_IP:PORT/meta";

byte key[16] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x61,0x62,0x63,0x64,0x65,0x66};
AESLib aes;
byte iv[16];
String eHash;

void setup() {
  Serial.begin(9600);
  EEPROM.begin(E_SZ);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected");

  if(check()) {
    EEPROM.write(B_ADDR, 0);
    EEPROM.write(V_ADDR, VER + 1);
    EEPROM.commit();
    if(ota()) ESP.restart();
  } else {
    EEPROM.write(B_ADDR, 1);
    EEPROM.write(V_ADDR, VER);
    EEPROM.commit();
  }
}

void loop() {
  if(millis() - lt >= 1000) {
    lt = millis();
    st = !st;
    digitalWrite(LED, st ? LOW : HIGH);
  }
}

bool check() {
  HTTPClient h;
  WiFiClient c;
  if(!h.begin(c, mUrl)) return 0;
  if(h.GET() != 200) return 0;

  int len = h.getSize();
  if(len <= 0 || len > 512) return 0;

  byte* buf = (byte*)malloc(len);
  if(!buf) return 0;
  
  h.getStream().readBytes(buf, len);
  h.end();

  memcpy(iv, buf, 16);
  
  byte out[512]; 
  int dLen = aes.decrypt(buf + 16, len - 16, out, key, 128, iv);
  free(buf);
  
  out[dLen] = 0;
  String m = String((char*)out);
  Serial.println(m);

  // Parse JSON manually
  int vIdx = m.indexOf("\"ver\":");
  int sIdx = m.indexOf("\"sha\":\"");
  int zIdx = m.indexOf("\"sz\":");
  
  int v = m.substring(vIdx + 6).toInt(); 
  eHash = m.substring(sIdx + 7, sIdx + 71); // 64 chars
  fSz = m.substring(zIdx + 5).toInt();

  return v > VER;
}

bool ota() {
  HTTPClient h;
  WiFiClient c;
  if(!h.begin(c, fUrl)) return 0;
  if(h.GET() != 200) return 0;

  int tot = h.getSize();
  WiFiClient* s = h.getStreamPtr();
  
  s->readBytes(iv, 16);
  
  Update.begin(fSz); // Use exact file size
  SHA256 sha;
  
  byte enc[16], dec[16], nIv[16];
  int rem = tot - 16;
  int wBytes = 0;

  while(rem > 0) {
    s->readBytes(enc, 16);
    memcpy(nIv, enc, 16);
    
    aes.decrypt(enc, 16, dec, key, 128, iv);
    
    // Only write/hash valid bytes, skip padding
    int chunk = 16;
    if (wBytes + 16 > fSz) {
      chunk = fSz - wBytes;
    }

    if (chunk > 0) {
      sha.update(dec, chunk);
      Update.write(dec, chunk);
      wBytes += chunk;
    }
    
    memcpy(iv, nIv, 16);
    rem -= 16;
  }

  byte res[32];
  sha.finalize(res, 32);
  
  char hStr[65];
  for(int i=0; i<32; i++) sprintf(&hStr[i*2], "%02x", res[i]);
  hStr[64] = 0;

  if(eHash != String(hStr)) {
    Serial.println("Hash Fail");
    Update.end(false);
    return 0;
  }
  return Update.end(true);
}
