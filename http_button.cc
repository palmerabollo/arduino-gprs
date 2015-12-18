#include <GSM.h>

int IN_PIN = 12;
int OUT_PIN = 13;

#define PINNUMBER ""

#define GPRS_APN "sm2ms.movilforum.es"
#define GPRS_LOGIN ""
#define GPRS_PASSWORD ""

char server[] = "hook.io"; // do not include path
int port = 80;

GSMClient client;
GPRS gprs;
GSM gsmAccess(true); // include a 'true' parameter for debug enabled

void setup() {
  pinMode(OUT_PIN, OUTPUT);
  pinMode(IN_PIN, INPUT);

  setupGsmShield();
}

void setupGsmShield()
{
  Serial.begin(9600);
  Serial.println("Starting Arduino Button");

  boolean notConnected = true;
  while (notConnected)
  {
    delay(2000);

    int ok = gsmAccess.begin(PINNUMBER);

    if (ok != GSM_READY)
    {
      Serial.println("GSM status failure. Retry.");
    }
    else
    {
      Serial.println("GSM status success.");
      notConnected = false;
    }
  }

  notConnected = true;
  while (notConnected)
  {
    delay(2000);

    int ok = gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD);

    if (ok != GPRS_READY)
    {
      Serial.println("GPRS status failure. Retry.");
    }
    else
    {
      Serial.println("GPRS status success.");
      notConnected = false;
    }
  }
}

int state = HIGH;    // the current state of the output pin
int reading;         // the current reading from the input pin
int previous = LOW;  // the previous reading from the input pin

long time = 0;         // the last time the output pin was toggled
long debounce = 1000;  // the debounce time, increase if the output flickers

void loop() {
  reading = digitalRead(IN_PIN);

  if (reading == HIGH && previous == LOW && millis() - time > debounce)
  {
    if (state == HIGH)
    {
      state = LOW;
      digitalWrite(OUT_PIN, LOW);
      sendRequest(0);
    }
    else
    {
      state = HIGH;
      digitalWrite(OUT_PIN, LOW);
      sendRequest(100);
    }

    time = millis();
  }

  previous = reading;
}

void sendRequest(int latest)
{
  Serial.println("Connecting to server...");

  if (client.connect(server, port))
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.println("Connection failed");
    client.stop();
    return;
  }

  client.println("GET / HTTP/1.0");
  client.print("Host:");
  client.println(server);
  client.println();

  String response = "";
  boolean pending = true;
  while (pending) {
    if (client.available())
    {
      char c = client.read();
      response += c;

      // cast response obtained from string to char array
      char responsechar[response.length() + 1];
      response.toCharArray(responsechar, response.length() + 1);

      if (strstr(responsechar, "200 OK") != NULL)
      {
        digitalWrite(OUT_PIN, HIGH);
        Serial.println("OK TEST COMPLETE!");
        pending = false;
      }
    }

    if (!pending || !client.connected())
    {
      Serial.println("Disconnecting.");
      client.stop();
      Serial.println("Disconnected.");
      pending = false;
    }
  }
}
