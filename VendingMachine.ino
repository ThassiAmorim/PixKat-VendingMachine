#include <Adafruit_GFX.h> //1.11.9
#include <MCUFRIEND_kbv.h> //3.1.0-Beta
#include <TouchScreen.h> //1.1.5
#include <WiFi.h> 
#include <HTTPClient.h> //2.2.0
#include <ArduinoJson.h> //7.0.4
#include <ESP32Servo.h> //3.0.5

#include "qrcodegen/qrcodegen.h"
#include "soc/sens_reg.h"

#define MINPRESSURE 200
#define MAXPRESSURE 1000

const int XP = 27, XM = 32, YP = 33, YM = 14; //240x320 ID=0x9325
const int TS_LEFT = 202, TS_RT = 896, TS_TOP = 123, TS_BOT = 871;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button maisB, menosB, confirmB, fechaB; //Botões clicaveis na tela

MCUFRIEND_kbv tft; //Cria uma instância do display

JsonDocument doc; //Cria um documento tipo JSON

Servo servoPistao, servoPorta1, servoPorta2;

const char* ssid = "JOAO"; //SSID da rede Wi-Fi
const char* senha = "Kaszuba02"; //Senha da rede Wi-Fi
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";

typedef struct infopayment_t_{

  const char* id;
  const char* qr_code;

}infopayment_t;
 
infopayment_t pagamento;

String nomeServidor = "https://27wldzwabc.execute-api.us-east-1.amazonaws.com/v1/pix"; //Endereço do servidor

unsigned int quant = 1; //Quantidade de produtos
unsigned int quantMax = 14; //Quantidade máxima de produtos

float preco = 0.02;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars

uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

bool Touch_getXY(void) {
  TSPoint p = ts.getPoint();

  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);      //because TFT control pins

  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);

  if (pressed) {

    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());

  }

  return pressed;
}

infopayment_t Request_qr() {

  HTTPClient https;
  NetworkClientSecure client;

  client.setCACert(root_ca);

  https.begin(client, nomeServidor);
  https.addHeader("Content-Type", "application/json");

  String jsonData = "{\"value\":\"" + String(quant) + "\"}";Serial.println(jsonData);

  int httpResponseCode = https.POST(jsonData);

  if (httpResponseCode > 0) {

    String response = https.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
    
    DeserializationError error = deserializeJson(doc, response);
    pagamento.qr_code = doc["qrcode"];
    pagamento.id = doc["id"]; //Serial.println("DOC[ID]"+doc["id"]);

    return pagamento;

  } else {

    Serial.print("Erro na requisição: ");
    Serial.println(httpResponseCode);

  }

  https.end();
}

bool Check_payment() {

  HTTPClient https;
  NetworkClientSecure client; 

  String jsonData = "{\"id\":" + String(pagamento.id) + "}"; 
  Serial.println(jsonData);

  client.setCACert(root_ca);
  https.begin(client, nomeServidor);
  https.addHeader("Content-Type", "application/json");

  int httpResponseCode = https.POST(jsonData);

  if (httpResponseCode > 0) {

    String response = https.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);

    JsonDocument doc1;
    DeserializationError error = deserializeJson(doc1, response);
    return doc1["status"].as<int>() == 1 ? true : false;

  } else {

    Serial.print("Erro na requisição: ");
    Serial.println(httpResponseCode);
  }

  https.end();
}

static void Print_qrcode(const uint8_t qrcode[]) {

	int size = qrcodegen_getSize(qrcode);
	char border = 2;
  char module_size = 4;
  uint16_t color;

	for (int y = -border; y < size + border; y++) {
		for (int x = -border; x < size + border; x++) {

      if (qrcodegen_getModule(qrcode, x, y)) {color = BLACK;} 
      else {color = WHITE;}

      for (int i = 0; i < module_size; i++) {
        for (int j = 0; j < module_size; j++) {
          tft.writePixel((x+4)*module_size + j + 6, (y+4)*module_size + i + 86, color);
        }
      }
		}
	}
}

static void Criar_qrcode(void) {

  const char *text = pagamento.qr_code;
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level
	
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);

	if(ok) { Print_qrcode(qrcode); }
}

void Print_quant(void) { //Função que imprime a quantidade de produtos

  tft.fillRect(10, 80, 180, 75, BLACK); //Desenha a moldura da impressão

  if (quant < 10) { tft.setCursor(40, 80); } //Posiciona onde será impresso
  else { tft.setCursor(10, 80); } //Posiciona onde será impresso
  
  tft.setTextColor(WHITE, BLACK); //Define a cor do texto
  tft.setTextSize(8); //Define o tamanho do texto
  tft.print(quant); //Imprime o texto

  tft.setCursor(100, 85);
  tft.setTextColor(WHITE,BLACK);
  tft.setTextSize(6);
  tft.cp437(true);
  tft.write(0x2F);

  if (quantMax < 10) { tft.setCursor(160, 80); } //Posiciona onde será impresso
  else { tft.setCursor(130, 80); } //Posiciona onde será impresso

  tft.setTextColor(WHITE,BLACK);
  tft.setTextSize(8);
  tft.print(quantMax);
}

void Show_telaIni(void) {

  tft.fillScreen(BLACK); //Limpa a tela

  //Comandos para exibir o texto inicial
  tft.setCursor(0, 10);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.print("Selecione a quantidade de produtos!");

  //Comandos para criar os circulos do botão
  tft.drawCircle(60, 180, 25, RED);
  tft.fillCircle(60, 180, 25, RED);
  tft.drawCircle(180, 180, 25, RED);
  tft.fillCircle(180, 180, 25, RED);

  //Comandos para criar os botões de mais e menos
  menosB.initButton(&tft, 62, 182, 40, 40, RED, RED, WHITE, "-", 4);
  maisB.initButton(&tft, 182, 182, 40, 40, RED, RED, WHITE, "+", 4);
  menosB.drawButton(false);
  maisB.drawButton(false);

  //Comandos para criar o botão de confirmação
  confirmB.initButton(&tft, 120, 270, 200, 50, BLUE, BLUE, WHITE, "Confirmar", 3);
  confirmB.drawButton(false);

  Print_quant(); //Chama função que exibe a quantidade de produtos na tela
}

void Show_telaFail(void) {

  tft.fillScreen(BLACK);

  tft.setCursor(60, 115);
  tft.setTextColor(RED, BLACK);
  tft.setTextSize(4);
  tft.print("Tempo");

  tft.setCursor(25, 160);
  tft.setTextColor(RED, BLACK);
  tft.setTextSize(4);
  tft.print("Esgotado");

  delay(5000);

  quant = 1;
  Show_telaIni();
}

void Show_telaOut(void) {

  tft.fillScreen(BLACK);

  tft.setCursor(85, 85);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(4);
  tft.print("Sem");

  tft.setCursor(40, 120);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(4);
  tft.print("Estoque");

  tft.setCursor(85, 200);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(6);
  tft.printf(":(");
}

void Show_telaHit(void) { //Função para exibir tela de pago e mover os servos

  tft.fillScreen(BLACK);

  tft.setCursor(50, 115);
  tft.setTextColor(GREEN, BLACK);
  tft.setTextSize(4);
  tft.print("Compra");

  tft.setCursor(25, 160);
  tft.setTextColor(GREEN, BLACK);
  tft.setTextSize(4);
  tft.print("Aprovada");

  //servoPorta1.write(90); //Abre a porta
  //servoPorta2.write(90); //Abre a porta

  int quantB = quant;

  while (quantB > 0) {

    servoPistao.attach(18, 1000, 2000); //Liga o servo do pistão

    servoPistao.write(0); //Faz o servo do pistao girar no sentido anti-horário
    delay(1000);

    servoPistao.write(180); //Faz o servo do pistao girar no sentido horário
    delay(1000);

    servoPistao.detach(); //Desliga o servo do pistão
    delay(250);

    quantB--;
  }

  //servoPorta1.write(0); //Fecha a porta
  //servoPorta2.write(0); //Fecha a porta

  pagamento.id = "";
  pagamento.qr_code = "";
  quantMax = quantMax - quant;
  quant = 1;

  if (quantMax >= 1) { Show_telaIni(); } 
  else { Show_telaOut(); }

}

void Show_telaPix(void) {

  tft.fillScreen(BLACK); //Limpa a tela

  int min = 0, seg = 0;
  bool down, fecha = false;
  bool pago = false;
  unsigned long agora, tmpIni, tmpFim;

  Criar_qrcode();

  tft.drawCircle(25, 25, 22, RED);
  tft.fillCircle(25, 25, 22, RED);

  fechaB.initButton(&tft, 27, 27, 33, 33, RED, RED, WHITE, "X", 3);
  fechaB.drawButton(false);

  for (int i = 120; i > 0; i--) {

    if (fecha || pago) { break; }

    min = (int) i/60;
    seg = (int) i%60;

    tft.setCursor(80, 50);
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(3);

    if (seg < 10) {

      tft.cp437(true);
      tft.printf("0%d:0%d", min, seg); 

    } else { 
      
      tft.cp437(true);
      tft.printf("0%d:%d", min, seg);

    }

    agora = millis();
    tmpFim = 0;
    tmpIni = 0;

    while(millis() < agora + (1000 - (tmpFim - tmpIni))) {

      tmpIni = millis();

      down = Touch_getXY();
      fechaB.press(down && fechaB.contains(pixel_x, pixel_y));

      if (fechaB.isPressed()) { fecha = true; }

      pago = Check_payment();

      tmpFim = millis();
    }
  }

  if (fecha) {

    quant = 1;
    Show_telaIni();

  } else {

    if (pago) { Show_telaHit(); }
    else { Show_telaFail(); }

  }
}

void Ini_WiFi(void) { //Função que inicia a conexão Wi-Fi do microcontrolador

  WiFi.disconnect(true); //Desconectar qualquer conexão WiFi existente
  WiFi.mode(WIFI_STA); //Configurar o modo de WiFi do ESP32

  WiFi.begin(ssid, senha); //Iniciar a conexão WiFi
  Serial.print("Connecting to WiFi ");

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");
    delay(500);
  }

  Serial.println("\nConnected to WiFi network");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

}

void Ini_Servo(void) { //Função que inicia os servo motores

  servoPistao.setPeriodHertz(50);// Standard 50hz servo
  servoPistao.attach(23, 1000, 2000); //Define o pino GPIO 18 para o servo do pistão

  //servoPorta1.attach(22); //Define o pino GPIO 22 para o primeiro servo da porta
  //servoPorta2.attach(19); //Define o pino GPIO 19 para o segundo servo da porta

  //servoPorta1.write(0); //Fecha a porta
  //servoPorta2.write(0); //Fecha a porta

}

void setup(void) {

  analogReadResolution(10);
  Serial.begin(115200);

  uint16_t ID = tft.readID(); //Lê o ID da controladora
  tft.begin(ID); //Inicia o touch para o controlador em questão
  tft.setRotation(0); //Inicia o display em modo retrato

  Ini_Servo(); //Inicia os servos motores

  Ini_WiFi(); //Inicia a conexão Wi-Fi do ESP32

  Show_telaIni(); //Chama função da tela inicial;
}

void loop(void) {

  bool down = Touch_getXY();

  maisB.press(down && maisB.contains(pixel_x, pixel_y));
  menosB.press(down && menosB.contains(pixel_x, pixel_y));
  confirmB.press(down && confirmB.contains(pixel_x, pixel_y));

  if (quantMax >= 1) {

    if (menosB.justReleased()) {

      tft.drawCircle(60, 180, 25, RED);
      tft.fillCircle(60, 180, 25, RED);
      menosB.drawButton();
    }

    if (maisB.justReleased()) {

      tft.drawCircle(180, 180, 25, RED);
      tft.fillCircle(180, 180, 25, RED);
      maisB.drawButton();
    }

    if (menosB.justPressed()) {
      if (quant > 1) {

        tft.drawCircle(60, 180, 25, WHITE);
        tft.fillCircle(60, 180, 25, WHITE);

        quant--;
        Print_quant();

        delay(200);
      }
    }

    if (maisB.justPressed()) {
      if (quant < quantMax) {

        tft.drawCircle(180, 180, 25, WHITE);
        tft.fillCircle(180, 180, 25, WHITE);

        quant++;
        Print_quant();

        delay(200);
      }
    }

    if (confirmB.justPressed()) {

      tft.fillScreen(BLACK); //Limpa a tela

      tft.setCursor(40, 140);
      tft.setTextColor(WHITE, BLACK);
      tft.setTextSize(3);
      tft.print("Aguarde...");

      pagamento = Request_qr();

      Serial.println("Pagamento QRCode: ");
      Serial.println(pagamento.qr_code);
      Serial.println("Pagamento ID: ");
      Serial.println(pagamento.id);

      Show_telaPix();
    }
  }
}