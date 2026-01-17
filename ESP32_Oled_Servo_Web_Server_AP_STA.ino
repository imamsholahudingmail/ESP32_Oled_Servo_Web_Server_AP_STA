/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp32-servo-motor-web-server-arduino-ide/ 
*********/

#include <WiFi.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

// GPIO the servo is attached to
static const int servoPin = 13;

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// Replace with your network credentials
const char* ssid = "DigPart";
const char* password = "12443411";

// Pengaturan untuk mode AP (jika mode STA gagal)
const char* ssid_ap = "ESP32_AP";
const char* password_ap = "1234567890";  // Password minimal 8 karakter
const int connection_timeout = 5;       // Batas waktu koneksi dalam detik

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Decode HTTP GET value
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  myservo.attach(servoPin);  // attaches the servo on the servoPin to the servo object

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true)
      ;
  }
  display.setTextColor(WHITE);

  // Connect to Wi-Fi network with SSID and password
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  int attempts = 0;
  // Tunggu koneksi STA atau sampai batas waktu tercapai
  while (WiFi.status() != WL_CONNECTED && attempts < connection_timeout) {
    delay(500);
    Serial.print(".");
    display.print(".");
    attempts++;
  }
  
  // Periksa status koneksi setelah loop
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nKoneksi STA berhasil!");
    Serial.print("Alamat IP STA: ");
    Serial.println(WiFi.localIP());
    // Lanjutkan dengan logika mode STA Anda di sini
  } else {
    Serial.println("\nKoneksi STA gagal. Beralih ke mode Access Point (AP)...");
    activateAPMode();
  }

  TampilkanDiLayar();
  server.begin();
}

void activateAPMode() {
  // Matikan mode STA terlebih dahulu
  WiFi.disconnect();

  // Aktifkan mode AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_ap, password_ap);

  Serial.print("Mode AP aktif. SSID: ");
  Serial.println(ssid_ap);
  Serial.print("Alamat IP AP: ");
  Serial.println(WiFi.softAPIP());
  // Lanjutkan dengan logika mode AP Anda di sini (misalnya, memulai server web)
}

void loop() {
  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {  // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");                                             // print a message out in the serial port
    String currentLine = "";                                                   // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        header += c;
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}");
            client.println(".slider { width: 300px; }</style>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");

            // Web Page
            client.println("</head><body><h1>ESP32 with Servo</h1>");
            client.println("<p>Position: <span id=\"servoPos\"></span></p>");
            client.println("<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\"" + valueString + "\"/>");

            client.println("<script>var slider = document.getElementById(\"servoSlider\");");
            client.println("var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;");
            client.println("slider.oninput = function() { slider.value = this.value; servoP.innerHTML = this.value; }");
            client.println("$.ajaxSetup({timeout:1000}); function servo(pos) { ");
            client.println("$.get(\"/?value=\" + pos + \"&\"); {Connection: close};}</script>");

            client.println("</body></html>");

            //GET /?value=180& HTTP/1.1
            if (header.indexOf("GET /?value=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1 + 1, pos2);

              TampilkanDiLayar();
            }
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void TampilkanDiLayar() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("WEB Servo");
  display.setTextSize(1);
  printWiFiMode(WiFi.getMode());

  //Rotate the servo
  myservo.write(valueString.toInt());
  Serial.println(valueString);

  // ===== LABEL =====
  display.setCursor(0, 36);
  display.print("Position Servo");

  // ===== SERVO =====
  display.setTextSize(2);
  display.setCursor(32, 46);
  display.print(valueString);

  display.setTextSize(2);
  display.cp437(true);
  display.write(167);

  display.display();
}


void printWiFiMode(WiFiMode_t mode) {
  switch (mode) {
    case WIFI_MODE_STA:
      display.setCursor(0, 18);
      display.print("STA: ");
      display.print(ssid);
      display.setCursor(0, 26);

      display.print("IP : ");
      display.print(WiFi.localIP());
      break;
    case WIFI_MODE_AP:
      display.setCursor(0, 18);
      display.print("AP : ");
      display.print(ssid_ap);
      display.setCursor(0, 26);

      display.print("IP : ");
      display.print(WiFi.softAPIP());
      break;
    case WIFI_MODE_APSTA:
      display.print("WIFI_AP_STA (Access Point + Station Mode)");
      break;
    case WIFI_MODE_NULL:
      display.print("WIFI_MODE_NULL (WiFi dinonaktifkan)");
      break;
    default:
      display.print("Mode tidak dikenal");
      break;
  }
}