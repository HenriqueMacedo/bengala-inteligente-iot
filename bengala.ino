#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define PIN_TRIG 2
#define PIN_ECHO 18
#define PIN_BOTAO 12
#define PIN_Buzzer 4

#define DISTANCIA_ALERTA 50
#define LIMIAR_QUEDA 28000
#define DURACAO_PANICO 10000

const char* SERVER_IP = "192.168.0.3";
const int SERVER_PORT = 5000;

const char* ssid     = "CLARO_CASA40";
const char* password = "SofiDados10$";

const String BOT_TOKEN = "8769133308:AAFHDLkTqjMprWNSaVmCVKi_TTGw8odtoyI";
const String CHAT_ID   = "7105507436";

MPU6050 sensor;
int16_t ax, ay, az, gx, gy, gz;

unsigned long tempoPanico = 0;
bool panicoAtivo = false;

#define MAX_HISTORICO 10
String historico[MAX_HISTORICO];
int totalEventos = 0;

unsigned long ultimoAlertaQueda = 0;
unsigned long ultimoAlertaObst  = 0;
#define INTERVALO_ALERTA 15000

float ultimaDistancia = 0;
int16_t ultimoAx = 0;

void enviarTelegram(String mensagem) {
  if (WiFi.status() != WL_CONNECTED) return;
  
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(10);
  HTTPClient http;
  
  mensagem.replace("\\", "\\\\");
  mensagem.replace("\"", "\\\"");
  mensagem.replace("\n", "\\n");
  
  String url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  
  String body = "{\"chat_id\":\"" + CHAT_ID + "\",\"text\":\"" + mensagem + "\"}";
  Serial.println("Body: " + body); // debug
  
  int code = http.POST(body);
  Serial.println("Telegram code: " + String(code));
  if (code != 200) {
    Serial.println("Erro: " + http.getString());
  }
  http.end();
}

void registrarEvento(String evento) {
  if (totalEventos >= MAX_HISTORICO) {
    for (int i = 0; i < MAX_HISTORICO - 1; i++) {
      historico[i] = historico[i + 1];
    }
    historico[MAX_HISTORICO - 1] = evento;
  } else {
    historico[totalEventos] = evento;
    totalEventos++;
  }
}

void verificarComandos() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(5);
  HTTPClient http;

  static int ultimoUpdateId = 0;

  String url = "https://api.telegram.org/bot" + BOT_TOKEN +
               "/getUpdates?offset=" + String(ultimoUpdateId + 1) +
               "&timeout=1&allowed_updates=message";

  http.begin(client, url);
  http.setTimeout(6000);
  int code = http.GET();

  if (code != 200) {
    Serial.println("Telegram getUpdates falhou: " + String(code));
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  Serial.println("Telegram payload: " + payload); 

  int searchFrom = 0;
  int lastFoundId = -1;
  while (true) {
    int idx = payload.indexOf("\"update_id\":", searchFrom);
    if (idx < 0) break;
    int start = idx + 12;
    int end = payload.indexOf(",", start);
    if (end < 0) end = payload.indexOf("}", start);
    lastFoundId = payload.substring(start, end).toInt();
    searchFrom = end;
  }
  if (lastFoundId > 0) {
    ultimoUpdateId = lastFoundId;
  }

  if (payload.indexOf("\"text\":\"/status\"") >= 0 ||
      payload.indexOf("\"text\": \"/status\"") >= 0) {
    String msg = "STATUS ATUAL\n";
    msg += "Distancia: " + String(ultimaDistancia, 1) + " cm\n";
    msg += "Aceleracao X: " + String(ultimoAx) + "\n";
    msg += "Panico ativo: " + String(panicoAtivo ? "SIM" : "NAO");
enviarTelegram(msg);
    Serial.println("Comando /status respondido.");
  }

  if (payload.indexOf("\"text\":\"/historico\"") >= 0 ||
    payload.indexOf("\"text\": \"/historico\"") >= 0) {
  if (totalEventos == 0) {
    enviarTelegram("Nenhum evento registrado ainda.");
  } else {
    String msg = "HISTORICO:\n";
    for (int i = 0; i < totalEventos && i < MAX_HISTORICO; i++) {
      msg += String(i+1) + ". " + historico[i] + "\n";
      }
      enviarTelegram(msg);
  }
  Serial.println("Comando /historico respondido.");
  }
}

void enviarDados(float distancia, int16_t ax, int16_t ay, int16_t az, String evento) {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  String url = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/dados";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  String body = "{\"distancia\":" + String(distancia, 1) +
                ",\"ax\":" + String(ax) +
                ",\"ay\":" + String(ay) +
                ",\"az\":" + String(az) +
                ",\"evento\":\"" + evento + "\"}";
  http.POST(body);
  http.end();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  sensor.initialize();
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_Buzzer, OUTPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado! IP: " + WiFi.localIP().toString());

  if (sensor.testConnection()) {
    Serial.println("Sistema da Bengala Iniciado com Sucesso!");
  } else {
    Serial.println("Erro ao iniciar o MPU6050. (sensor operacional)");
  }

  enviarTelegram("Bengala Inteligente iniciada e conectada!");
}

void loop() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duracao = pulseIn(PIN_ECHO, HIGH);
  float distancia = duracao * 0.034 / 2;
  ultimaDistancia = distancia;

  sensor.getAcceleration(&ax, &ay, &az);
  sensor.getRotation(&gx, &gy, &gz);
  ultimoAx = ax;

  bool botaoPressionado = (digitalRead(PIN_BOTAO) == LOW);

  if (botaoPressionado && !panicoAtivo) {
    panicoAtivo = true;
    tempoPanico = millis();
    Serial.println("SOS: Panico acionado!");
    registrarEvento("Panico acionado");
    enviarTelegram("SOS! Botao de panico acionado na bengala! Verifique o usuario imediatamente.");
  }

  if (panicoAtivo && (millis() - tempoPanico >= DURACAO_PANICO)) {
    panicoAtivo = false;
    Serial.println("Alerta de panico encerrado.");
  }

  if (panicoAtivo) {
    digitalWrite(PIN_Buzzer, HIGH);
  }
  else if (distancia > 0 && distancia <= DISTANCIA_ALERTA) {
    digitalWrite(PIN_Buzzer, HIGH);
    Serial.println("ALERTA: Obstaculo a frente!");
    if (millis() - ultimoAlertaObst > INTERVALO_ALERTA) {
      registrarEvento("Obstaculo a " + String(distancia, 1) + "cm");
      enviarTelegram("ALERTA: Obstaculo detectado a " + String(distancia, 1) + " cm!");
      ultimoAlertaObst = millis();
    }
  }
  else if (abs(ax) > LIMIAR_QUEDA || abs(ay) > LIMIAR_QUEDA) {
    digitalWrite(PIN_Buzzer, HIGH);
    Serial.println("ALERTA: Possivel queda!");
    if (millis() - ultimoAlertaQueda > INTERVALO_ALERTA) {
      registrarEvento("Possivel queda detectada");
      enviarTelegram("ALERTA! Possivel queda detectada! Verifique o usuario urgentemente.");
      ultimoAlertaQueda = millis();
    }
  }
  else {
    digitalWrite(PIN_Buzzer, LOW);
  }

  static unsigned long ultimaVerificacao = 0;
  if (millis() - ultimaVerificacao > 3000) {
    verificarComandos();
    ultimaVerificacao = millis();
  }

  Serial.print("Dist: "); Serial.print(distancia);
  Serial.print(" cm | Ax: "); Serial.print(ax);
  Serial.print(" | Panico: "); Serial.println(panicoAtivo ? "SIM" : "NAO");

static unsigned long ultimoEnvio = 0;
if (millis() - ultimoEnvio > 2000) {
  String evento = "";
  if (panicoAtivo) evento = "Panico";
  else if (distancia > 0 && distancia <= DISTANCIA_ALERTA) evento = "Obstaculo";
  else if (abs(ax) > LIMIAR_QUEDA || abs(ay) > LIMIAR_QUEDA) evento = "Queda";
  enviarDados(distancia, ax, ay, az, evento);
  ultimoEnvio = millis();
}
  delay(2000);
}
