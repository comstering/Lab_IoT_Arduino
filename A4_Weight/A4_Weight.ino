//This library can be obtained here http://librarymanager/All#Avia_HX711
#include "HX711.h" 
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

/*무게 센서*/
#define LOADCELL_DOUT_PIN D5
#define LOADCELL_SCK_PIN D6
float output = 0.0;

/*와이파이*/
const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "210.125.212.191";    //  공용PC IP
const uint16_t port = 8888;    //  웹서버 포트

/*LEA 암호*/
BYTE pbUserKey[16] = { "security915!@#" };


HX711 scale;
 
//-7050 worked for my 440lb max scale setup
float calibration_factor = 20000;
 
void setup() 
{
  Serial.begin(115200);

  /*무게센서 setup*/
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  //Reset the scale to 0
  scale.tare();
 
  //Get a baseline reading
  long zero_factor = scale.read_average(); 
  //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);
  
   /*WiFi setup*/
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);    //  와이파이 설정

  while (WiFi.status() != WL_CONNECTED) {    //  와이파이 연결 될때까지 반복
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void loop() 
{
  /*무게 센서*/
  //Adjust to this calibration factor
  scale.set_scale(calibration_factor);

  output = scale.get_units(), 1;
  output /= 10;
  Serial.print("Reading: ");
  Serial.print(output);
  
  //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  //기존 예제가 파운드(lbs) 기준이지만 우리는 킬로그램(kg)을 쓸것이므로 'kg'로 바꿉시다.
  Serial.print(" kg"); 
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();

  /*와이파이*/
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  Serial.println();
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  /*Lea 암호*/
  BYTE pbData[16] = { "security" };
  String check = LEA_Encrypto(pbData);

  String url = "/IoT/A4StatusUpdate.jsp?check=";    //  DB 통신할 문장
  url += check;    //  check 값
  url += "&a4=";

  String a4v = a4Value(output);
  url += a4v;

  /*URL 연결*/
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
  while (client.available()) {    //  응답 후 결과 값 받아오기
    String line = client.readStringUntil('\r');

    count++;
    if(count==11) {
      Serial.println("result: " + line);
      break;
    }
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();
 
  delay(1000);
}

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

String a4Value(float a4) {
  if(a4 <= 0.09) {    //  불이 꺼져 있을 때
    BYTE value[16] = { "0" };
    return LEA_Encrypto(value);
  } else {
    BYTE value[16] = { "1" };
    return LEA_Encrypto(value);
  }
}
