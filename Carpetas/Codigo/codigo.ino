#include <WiFiS3.h>
#include <Servo.h>

const char* ssid = "iPhone de jm407";
const char* password = "programacion";

WiFiServer server(80);
Servo myServo;

const int SERVO_PIN = 9;
int currentAngle = 90;

void setup() {
  Serial.begin(115200);
  
  // Pausa para dar tiempo a la inicialización del puerto USB nativo del UNO R4
  delay(1500);

  myServo.attach(SERVO_PIN);
  myServo.write(currentAngle);

  Serial.println();
  Serial.print("Conectando a la red WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("¡Conexion exitosa!");
  Serial.print("Direccion IP asignada: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("GET /set?angle=") >= 0) {
      int posStart = request.indexOf("angle=") + 6;
      int posEnd = request.indexOf(" ", posStart);
      String angleStr = request.substring(posStart, posEnd);
      int angle = angleStr.toInt();

      if (angle >= 0 && angle <= 180) {
        currentAngle = angle;
        myServo.write(currentAngle);
        Serial.print("Nuevo angulo: ");
        Serial.println(currentAngle);
      }
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    
    client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
    client.println("<title>Control Servo R4 WiFi</title>");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f9; }");
    client.println(".container { background: white; padding: 20px; border-radius: 10px; display: inline-block; box-shadow: 0 0 10px rgba(0,0,0,0.1); }");
    client.println("input[type=range] { width: 280px; margin: 20px 0; }");
    client.println("</style></head><body>");
    client.println("<div class='container'>");
    client.println("<h1>Control de Servomotor</h1>");
    client.println("<h2>Ángulo: <span id='val'>" + String(currentAngle) + "</span>°</h2>");
    client.println("<input type='range' min='0' max='180' value='" + String(currentAngle) + "' onchange='sendAngle(this.value)' oninput='document.getElementById(\"val\").innerText=this.value'>");
    client.println("</div>");
    client.println("<script>");
    client.println("function sendAngle(val) { fetch('/set?angle=' + val); }");
    client.println("</script>");
    client.println("</body></html>");

    delay(1);
    client.stop();
  }
}
