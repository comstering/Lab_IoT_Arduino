#include<Servo.h>
#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "SecurityLab(2.4)"    //  와이파이 이름
#define STAPSK  "security915"    //  와이파이 비밀번호
#endif

/*서브모터*/
Servo myservo;

/*와이파이*/
const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "210.125.212.191";    //  공용PC IP
const uint16_t port = 8888;    //  웹서버 포트

int pos = 0;

void setup() {
  /*Moter setup*/
  myservo.attach(9);

  /*WiFi setup*/
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

void loop() {
  control();
  delay(1000); // execute once every 5 minutes, don't flood remote service
}

void control() {
  static int light = 0;
  int st;

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  String url = "/IoT/LightStatusUpdate.jsp?check=security";

  
  // This will send a string to the server
  Serial.println("sending data to server");
  client.print(String("POST ") + url + " HTTP/1.1\r\n" + "HOST: " + host + "\r\n" + "Connection: close\r\n\r\n");

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
  while (client.available()) {    //  응답 후 결과 값 받아오기
    String line = client.readStringUntil('\r');

    count++;
    if(count==11)
    {
      result = line;
    }
    Serial.print(line);
  }
  Serial.print("result: " + result);
  if(result.equals("open")) {
    st = 1;
  } else {
    st = 0;
  }

  if(light==st) {
    
  } else {
    light = st;
    if(st == 0) {    //  불끄기
      for(pos=20; pos <= 50; pos+=1) {
        myservo.write(pos);
        delay(10);
      }
      delay(1000);
      for(pos = 50; pos >= 20; pos -= 1) {
        myservo.write(pos);
        delay(10);
      }
      delay(3000);
    } else {    //  불 켜기
      for(pos=20;pos>=-10;pos-=1) {
        myservo.write(pos);
        delay(10);
      }
      delay(1000);
      for(pos=-10;pos<=20;pos+=1) {
        myservo.write(pos);
        delay(10);
      }
      delay(3000);
    }
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}
