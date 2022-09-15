/********************************************
 －－－－深圳市晶彩智能有限公司－－－－
  File name：40.WIFI Web Servers LED.ino
  Version：V2.0
  Illustrate：WIFI Web Servers Two-color LED light experiment
 ********************************************/
#include <WiFi.h>

// Enter the WIFI name and password that you can access to the Internet. 
//It is recommended to use an Android phone for web control to ensure that the ESP32 is in the same network.
const char* ssid = "CMCC-404";
const char* password = "12345678";

// Set the web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliary variable for storing the current output state
String output25State = "off";
String output26State = "off";

// Assign output variables to GPIO pins
const int output16 = 16;
const int output17 = 17;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define the timeout in milliseconds (eg: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize output variable to output
  pinMode(output16, OUTPUT);
  pinMode(output17, OUTPUT);
  // Set output to HIGH
  digitalWrite(output16, HIGH);
  digitalWrite(output17, HIGH);

  // Connect to Wi-Fi network using SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print the local IP address and start the web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Monitor clients

  if (client) {                             // If a new client connects
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // Print a message in the serial port
    String currentLine = "";                // Create a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // Loop while client connects
      currentTime = millis();
      if (client.available()) {             // If you want to read bytes from the client
        char c = client.read();             // Then read a byte
        Serial.write(c);                    // Print out on serial monitor
        header += c;
        if (c == '\n') {                    // If the byte is a newline
          // If the current line is empty, there are two newlines on a line.
          // This is the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // Then the content-type, so the client knows what to expect, followed by the empty line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turn gpio on and off

            if (header.indexOf("GET /16/on") >= 0) {
              Serial.println("GPIO 16 on");
              output25State = "on";
              digitalWrite(output16, LOW);
            } else if (header.indexOf("GET /16/off") >= 0) {
              Serial.println("GPIO 16 off");
              output25State = "off";
              digitalWrite(output16, HIGH);
            } else if (header.indexOf("GET /17/on") >= 0) {
              Serial.println("GPIO 17 on");
              output26State = "on";
              digitalWrite(output17, LOW);
            } else if (header.indexOf("GET /17/off") >= 0) {
              Serial.println("GPIO 17 off");
              output26State = "off";
              digitalWrite(output17, HIGH);
            }
            
            // Display HTML pages
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off button
            // Feel free to change the background color and font size properties to suit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Page title
            client.println("<body><h1>ESP32 Web Server LED </h1>");
            
            // Display current status and ON/OFF button of GPIO 16
            client.println("<p>GREEN LED - State " + output25State + "</p>");
            // If output26State is off, show ON button
            if (output25State=="off") {
              client.println("<p><a href=\"/16/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/16/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current status and ON/OFF button of GPIO 17
            client.println("<p> BLUE LED - State " + output26State + "</p>");
            // If output26State is off, show ON button
            if (output26State=="off") {
              client.println("<p><a href=\"/17/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/17/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // HTTP response ends with another blank line
            client.println();
            // Out of the while loop
            break;
          } else { // If you have a newline then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // If you get characters other than carriage return
          currentLine += c;      // Add it to the end of currentLine
        }
      }
    }
    // Clear header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}