#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "Ciklum Guest";
const char* password = "Welcome@Ciklum";

ESP8266WebServer server(80);

//blue color
const int pending = 13;
//red color
const int fail = 15;
//green color
const int success = 12;
//buz pin
const int speakerPin = 5;
int pendingStatus = 0;
int successStatus = 0;
int failStatus = 0;
bool isSongPlaying = false;

//song setup
int length = 70;
String notes[] = {"G4","G4", "G4", "D#4/Eb4", "A#4/Bb4", "G4", "D#4/Eb4","A#4/Bb4", "G4", "D5", "D5", "D5", "D#5/Eb5", "A#4/Bb4", "F#4/Gb4", "D#4/Eb4","A#4/Bb4", "G4", "G5","G4","G4","G5","F#5/Gb5", "F5","E5","D#5/Eb5","E5", "rest", "G4", "rest","C#5/Db5","C5","B4","A#4/Bb4","A4","A#4/Bb4", "rest", "D#4/Eb4", "rest", "F#4/Gb4", "D#4/Eb4","A#4/Bb4", "G4" ,"D#4/Eb4","A#4/Bb4", "G4"}; 
int beats[] = { 8, 8, 8, 6, 2, 8, 6 , 2 ,16 , 8, 8, 8, 6, 2, 8, 6, 2, 16,8,6,2,8,6,2,2, 2, 2,6,2,2,8,6,2,2,2,2,6,2,2,9,6,2,8,6,2,16  };
int tempo = 50;

  
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);      
  }
}
  
void playNote(String note, int duration) {
  String noteNames[] = { "D#4/Eb4", "E4", "F4", "F#4/Gb4", "G4", "G#4/Ab4", "A4", "A#4/Bb4", "B4", "C5", "C#5/Db5", "D5", "D#5/Eb5", "E5", "F5", "F#5/Gb5", "G5", "G#5/Ab5", "A5", "A#5/Bb5", "B5", "C6", "C#6/Db6", "D6", "D#6/Eb6", "E6", "F6", "F#6/Gb6", "G6" };
  int tones[] = { 1607, 1516, 1431, 1351, 1275, 1203, 1136, 1072, 1012, 955, 901, 851, 803, 758, 715, 675, 637, 601, 568, 536, 506, 477, 450, 425, 401, 379, 357, 337, 318 };
  for (int i = 0; i < 29; i++) {
    if (noteNames[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void pulse() {
  if(successStatus != 1 && failStatus != 1) {

    if (pendingStatus == 0) {
      pendingStatus = 1;
    } else {
      pendingStatus = 0;
    }
  }
  digitalWrite(pending, pendingStatus);
  delay(1000);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void playSong() {
  isSongPlaying = true;
  for (int i = 0; i < length; i++) {
      
      if (notes[i] == "rest") {
        delay(beats[i] * tempo);
      } else {
        playNote(notes[i], beats[i] * tempo);      
      }
      delay(tempo / 2);
    }
  isSongPlaying = false;
}

void handleLeds() {
    digitalWrite(fail, failStatus);
    digitalWrite(pending, pendingStatus);
    digitalWrite(success, successStatus);
}

void setup(void){
  pinMode(pending, OUTPUT);
  pinMode(success, OUTPUT);
  pinMode(fail, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    pulse();
    Serial.print("Can not connect to WiFi :( \n");
  }
  digitalWrite(pending, 0);
  Serial.println("Has been created by UDOLI!!!1 team");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  
  server.on("/success", [](){
    if (isSongPlaying == false) {
      successStatus = 1;
      failStatus = 0;
      pendingStatus = 0;
      handleLeds();
      server.send(200, "text/plain", "success is working");
    } else {
      server.send(200, "text/plain", "Please wait song is playing :(");
    }
  });
  
  server.on("/fail", [](){
    if (isSongPlaying == false) {
      failStatus = 1;
      successStatus = 0;
      pendingStatus = 0;
      handleLeds();
      server.send(200, "text/plain", "fail is working");
      playSong();
    } else {
      server.send(200, "text/plain", "Please wait song is playing :(");
    }
  });

  server.on("/off", [](){
    if (isSongPlaying == false) {
      failStatus = 0;
      successStatus = 0;
      pendingStatus = 0;
      handleLeds();
      server.send(200, "text/plain", "off is working");
    } else {
      server.send(200, "text/plain", "Please wait song is playing :(");
    }
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}