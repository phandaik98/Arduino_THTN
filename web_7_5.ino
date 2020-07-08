#include <SoftwareSerial.h>
#include <DHT.h>

#define DEBUG true
//#define EspSerial Serial1
uint8_t my_serial_bytes[5]={0xA1, 0xF1, 0x04, 0xFB, 0x12};

#define DHTPIN 7          // What digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
DHT dht(DHTPIN, DHTTYPE);

 int Temperature = 0;
 int Humidity = 0;
 byte arr[1];
 String tem,hum;

SoftwareSerial esp8266(2, 3); //  10-RX, 11-TX 
char x;

void sendSensor()
{
  Humidity = dht.readHumidity();
  delay(500);
  Temperature = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  delay(500);

  if (isnan(Humidity) || isnan(Temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}

void setup() {
  Serial.begin(9600);
  esp8266.begin(9600); // chu y phai cung toc do voi ESP 8266
  dht.begin();
 
  pinMode(13, OUTPUT); // tuong tu chan 12
  digitalWrite(13, LOW);
  //========================gui tap lenh AT cho ESP 8266 de cai dat cac tham so cho WiFi================================
  guidulieu("AT+RST\r\n", 2000, DEBUG); // reset module
  guidulieu("AT+CWMODE=1\r\n", 1000, DEBUG); // Chọn chức năng station cho esp
  guidulieu("AT+CWJAP=\"Deadline\",\"minhdz123\"\r\n", 1000, DEBUG); // Kết nối với wifi 
    delay(10000);
  guidulieu("AT+CIFSR\r\n", 1000, DEBUG); // xem ip là bn
  guidulieu("AT+CIPMUX=1\r\n", 1000, DEBUG); // configure for multiple connections
  guidulieu("AT+CIPSERVER=1,80\r\n", 1000, DEBUG); // Mở cổng 80
}
 
void loop() {
  sendSensor();
  arr[0] = Temperature;
  arr[1] = Humidity;
  tem = String(arr[0]);
  hum = String(arr[1]);

//  Serial.println(Temperature);
//  String t = "Temperature";
  while (esp8266.available()) {
    IncomingChar(esp8266.read());
  }
 
  if (x) {
    // Đây là giao diện web page viết dưới dạng html...các bạn có thể dễ dàng edit
  
    String html = "<HTML>";
    html +="<HEAD><TITLE>DOAN</TITLE>";
//    html +="<form action=\"\" method=\"GET\">";
    html +="<a href=\"/ON\" type=\"button\" name=\"L\" value=\"\">ON ";
    html +="<a href=\"/OFF\" type=\"button\" name=\"L\" value=\"\">OFF";
//    html +="</form>";
    
    html +="<div id=\"webpage\">\n";
//    html +="<h1>Weather Report</h1>\n";
    html +="<p>Temp: ";
    html += tem;
    html +="°C</p>";
    html +="<p>Hum: ";
    html += hum;
    html +="%</p>";
    html +="</div>\n";
    html +="</BODY></HTML>";
 
    String cipsend = "AT+CIPSEND=0,"; //gui chuoi data qua Wifi
    cipsend += html.length();
    cipsend += "\r\n";
    
    guidulieu(cipsend, 1000, DEBUG);
    guidulieu(html, 1000, DEBUG);
    guidulieu("AT+CIPCLOSE=0\r\n", 1000, DEBUG);
    x = 0;
  }
 
}
 
void IncomingChar(const byte InChar) {
  static char InLine[300]; //Hạn chế ký tự
  static unsigned int Position = 0;
 
  switch (InChar) {
  case '\r': //Cái này là xuống dòng...cho linux
    break;
 
  case '\n': // xuống dòng cho window...( kết thúc bảng tin )
    InLine[Position] = 0;
    ProcessCommand(String(InLine));
    Position = 0;
    break;
 
  default:
    InLine[Position++] = InChar;
 
  }
}
 
void ProcessCommand(String InLine) {
  Serial.println("InLine: " + InLine);
 
  if (InLine.startsWith("+IPD,")) {
 
    x = 1;
  }
  // lện String.indexOf(kytu)...chả về vị trí của ky tu trong chuỗi String...Nếu giá trị trả về là -1...tức là ky tu không xuất hiện trong chuỗi String
  if (InLine.indexOf("GET /OFF") != -1) {

    digitalWrite(13, LOW);
  }
 
  if (InLine.indexOf("GET /ON") != -1) {
 
    digitalWrite(13, HIGH);
    Serial.write(my_serial_bytes,sizeof(my_serial_bytes));
  }
}
 
String guidulieu(String lenh,
  const int thoigian, boolean debug) {
  String chuoigiatri = "";
 
  esp8266.print(lenh); // send the read character to the esp8266
 
  long int time = millis();
 
  while ((time + thoigian) > millis()) {
    while (esp8266.available()) {
 
      // The esp has data so display its output to the serial window 
      char c = esp8266.read(); // read the next character.
      chuoigiatri += c;
    }
  }
 
  if (debug) {
    Serial.print(chuoigiatri);
  }
 
  return chuoigiatri;
}
