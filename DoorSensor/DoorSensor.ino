/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "SecurityLab(2.4)"    //  와이파이 이름
#define STAPSK  "security915"    //  와이파이 비밀번호
#define trigPin 13    //  초음파 센서
#define echoPin 12    //  초음파 센서
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "210.125.212.191";    //  공용PC IP
const uint16_t port = 8888;    //  웹서버 포트

void setup() {
  Serial.begin(115200);

  //  초음파 센서 setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //  WiFi setup
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);    //  와이파이 설정

  while (WiFi.status() != WL_CONNECTED) {    //  와이파이 연결 될때까지 반복
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

void loop() {
  /*초음파 센서*/
  long duration, inches, cm;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  //convert the time into a distance
  cm = microsecondsToCentimeters(duration);
  Serial.print(cm);
  Serial.println(" cm");
  
  /*와이파이*/
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;    // 와이파이 통신
  if (!client.connect(host, port)) {    //  ip와 포트로 연결이 안될경우
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  String url = "/IoT/DoorStatusUpdate.jsp?check=security&door=";    //  DB 통신할 문장
  if(cm>=4 && cm<=6) {    //  문이 닫혀 있을 경우
    url += 0;
  }
  else {    //  문이 열려 있을 경우
    url += 1;
  }

  // This will send a string to the server
  Serial.println("sending data to server");
  client.print(String("POST ") + url + " HTTP/1.1\r\n" + "HOST: " + host + "\r\n" + "Connection: close\r\n\r\n");    //    웹서버에 있는 DB 통신 url과 연결

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {    //  웹서버와 통신 응답대기
    if (millis() - timeout > 5000) {    //  응답 시간이 너무 길어질 경우 timeout
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  int count = 1;
  String result;
  while (client.available()) {    //  응답후 결과 값 받아오기
    String line = client.readStringUntil('\r');

    count++;
    if(count==11)
    {
      result = line;
    }
    Serial.print(line);
  }
  Serial.print("result: " + result);

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  delay(1000); // execute once every 5 minutes, don't flood remote service
}
