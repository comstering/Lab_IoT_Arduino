#include<Servo.h>
#include <ESP8266WiFi.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "LEA_core.h"
#include "LEA_core.c"

#ifndef STASSID
#define STASSID "SecurityLab(2.4)"    //  와이파이 이름
#define STAPSK  "security915"    //  와이파이 비밀번호
#endif

/*서브모터*/
Servo myservo;
#define Dpin D6

/*와이파이*/
const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "210.125.212.191";    //  공용PC IP
const uint16_t port = 8888;    //  웹서버 포트

/*LEA 암호*/
BYTE pbUserKey[16] = { "security915!@#" };

/*반복전인 행동 방지*/
int lightstatic = 0;

void setup() {
  Serial.begin(115200);
  /*Moter setup*/
  myservo.attach(Dpin);

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
  Serial.print("connection to ");
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

  /*LEA 암호*/
  BYTE pbData[16] = { "security" };
  String check = LEA_Encrypto(pbData);

  String url = "/IoT/LightArduinoControl.jsp?check=";    //  DB 통신할 문장
  url += check;    //  check 값

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
    if(count==10)
    {
      result = line;
      Serial.println("result: " + result);
      break;
    }
  }
  control(result);

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();
  
  delay(1000); // execute once every 5 minutes, don't flood remote service
}

void control(String result) {
  int value;
  if(result.equals("\non")) {
    value = 1;
    if(lightstatic == value)
      return;
    myservo.write(60);
    delay(1000);
    myservo.write(90);
  } else if(result.equals("\noff")) {
    value = 0;
    if(lightstatic == value)
      return;
    myservo.write(135);
    delay(1000);
    myservo.write(90);
  } else {
    myservo.write(90);
  }
  lightstatic = value;
}

/*LEA 암호화*/
String LEA_Encrypto(BYTE data[16]) {
  BYTE pdwRoundKey[384] = { 0x0, };

  LEA_Key(pbUserKey, pdwRoundKey);
  LEA_Enc(pdwRoundKey, data);

  String encData = "";
  char str[3];
  for(int i = 0; i < 16; i++) {
    /*16진수 문자열로 변환*/
    sprintf(str, "%02x", data[i]);
    encData += str;
  }
  Serial.println(encData);
  return encData;
}
