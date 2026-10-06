/******************************************************************************
                                Monitor de Cotações
                                Display OLED 128x32

                          Criado em 06 de Outubro de 2026
                                por Amanda e Ana
                    Adaptado para OLED 128x32 (SSD1306)

 Blog Eletrogate - Veja este e outros projetos e tutoriais no blog Eletrogate
                            https://blog.eletrogate.com/

 Eletrogate - Loja de Arduino \ Robótica \ Automação \ Apostilas \ Kits
                            https://www.eletrogate.com/
******************************************************************************/

#include <WiFi.h>             // Biblioteca nativa do ESP32
#include <HTTPClient.h>       // Biblioteca nativa do ESP32
#include <Wire.h>             // Biblioteca nativa do ESP32
#include <Adafruit_SSD1306.h> // Necessária Instalação
#include <ArduinoJson.h>      // Necessária Instalação

// Configurações da rede WiFi à se conectar
const char* host = "esp32";
const char* ssid = "Redmi 12";
const char* password = "MariaV0803";

#define SCREEN_WIDTH 128 // Largura da tela OLED, em pixels
#define SCREEN_HEIGHT 32 // Altura da tela OLED, em pixels (Ajustado para 32)

#define OLED_RESET     -1 // Pino de Reset (ou -1 se compartilhar o pino de reset do Arduino)

#define SCREEN_ADDRESS 0x3C // Endereço I2C comum para OLED 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float dolar, euro, libra, peso, bitcoin, iene; // armazenará os valores das moedas
float varDolar, varEuro, varLibra, varPeso, varBitcoin, varIene; // armazenará as variações

bool invalido = true; // armazenará se a requisição de cotação é ou não inválida

String url = "https://economia.awesomeapi.com.br/json/last/";
String moedas = "USD-BRL,EUR-BRL,GBP-BRL,ARS-BRL,BTC-BRL,JPY-BRL";

int telaAtual = 5; // índice de tela OLED

// Timers:
unsigned long timerRequisicao; const int periodoRequisicao = 30000; // de requisição HTTP
unsigned long timerExibicao; const int periodoExibicao = 5000;       // de Troca de Tela
unsigned long timerProgress; const int periodoProgress = 50;         // de Barra de Progresso

int larguraDisplay; // armazenará a largura do display (em Pixels)
int alturaDisplay;  // armazenará a altura do display (em Pixels)

void setup() {

  url.concat(moedas);

  Serial.begin(115200);
  Serial.println();
  delay(1000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println("Falha na alocação de SSD1306");
    for (;;);
  }

  larguraDisplay = display.width() - 1;
  alturaDisplay = display.height() - 1;

  // Tela Inicial de Boot
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 2);
  display.println("Cotacao de Moedas");
  display.setCursor(20, 16);
  display.println("Conectando WiFi...");
  display.display();

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(host);
  WiFi.begin(ssid, password);
  Serial.println("[SETUP] Iniciando o WiFi");

  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    Serial.println("[SETUP] WiFi iniciado com sucesso!");
  }
  else
  {
    Serial.println("[SETUP] Houve falha na inicialização do WiFi. A placa será reiniciada.");
    ESP.restart();
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 2);
  display.println("Cotacao de Moedas");
  display.setTextSize(2);
  display.setCursor(15, 14);
  display.println("Busca...");
  display.display();
}

void loop() {

  // Faz a requisição e atualiza os dados
  if (millis() - timerRequisicao >= periodoRequisicao || timerRequisicao == 0) {
    HTTPClient http;
    Serial.println("[HTTP] begin...");
    http.begin(url);
    Serial.println("[HTTP] GET...");
    int httpCode = http.GET();
    if (httpCode == 200)
    {
      invalido = false;
      DynamicJsonDocument doc(2048);

      DeserializationError error = deserializeJson(doc, http.getString());
      if (error)
      {
        Serial.print("deserializeJson() falhou: ");
        Serial.println(error.f_str());
        invalido = true;
      } else
      {
        dolar = String((const char*)doc["USDBRL"]["bid"]).toFloat();
        varDolar = String((const char*)doc["USDBRL"]["pctChange"]).toFloat();
        euro = String((const char*)doc["EURBRL"]["bid"]).toFloat();
        varEuro = String((const char*)doc["EURBRL"]["pctChange"]).toFloat();
        libra = String((const char*)doc["GBPBRL"]["bid"]).toFloat();
        varLibra = String((const char*)doc["GBPBRL"]["pctChange"]).toFloat();
        peso = String((const char*)doc["ARSBRL"]["bid"]).toFloat();
        varPeso = String((const char*)doc["ARSBRL"]["pctChange"]).toFloat();
        bitcoin = String((const char*)doc["BTCBRL"]["bid"]).toFloat();
        varBitcoin = String((const char*)doc["BTCBRL"]["pctChange"]).toFloat();
        iene = String((const char*)doc["JPYBRL"]["bid"]).toFloat();
        varIene = String((const char*)doc["JPYBRL"]["pctChange"]).toFloat();
      }
    } else {
      invalido = true;
    }
    http.end();
    Serial.println("[HTTP] GET END!\n");
    timerRequisicao = millis();
  }

  // Muda a tela
  if (millis() - timerExibicao >= periodoExibicao || timerExibicao == 0) {
    telaAtual++;
    if (telaAtual > 5)
    {
      telaAtual = 0;
    }
    if (invalido)
    {
      timerRequisicao = millis() + periodoRequisicao;
    }

    switch (telaAtual) {
      case 0:
        printMoeda("Dolar", dolar, varDolar);
        break;
      case 1:
        printMoeda("Euro", euro, varEuro);
        break;
      case 2:
        printMoeda("Libra Est.", libra, varLibra);
        break;
      case 3:
        printMoeda("Peso Arge.", peso, varPeso);
        break;
      case 4:
        printMoeda("Bitcoin", bitcoin, varBitcoin);
        break;
      case 5:
        printMoeda("Iene", iene, varIene);
        break;
    }
    display.display();

    timerExibicao = millis();
  }

  // Exibe a barra de progresso
  if (millis() - timerProgress >= periodoProgress || timerProgress == 0) {
    switch (telaAtual)
    {
      case 0:
        printProgressBar(map(millis() - timerExibicao, 0, periodoExibicao, 0, 16));
        break;
      case 1:
        printProgressBar(map(millis() - timerExibicao, 0, periodoExibicao, 16, 32));
        break;
      case 2:
        printProgressBar(map(millis() - timerExibicao, 0, periodoExibicao, 32, 48));
        break;
      case 3:
        printProgressBar(map(millis() - timerExibicao, 0, periodoExibicao, 48, 64));
        break;
      case 4:
        printProgressBar(map(millis() - timerExibicao, 0, periodoExibicao, 64, 80));
        break;
      case 5:
        printProgressBar(map(millis() - timerExibicao, 0, periodoExibicao, 80, 100));
        break;
    }
    display.display();
    timerProgress = millis();
  }

  delay(1);
}

/**
  Exibe os dados da moeda formatados para 128x32
*/
void printMoeda(String nomeMoeda, float moeda, float variacaoMoeda) {
  display.setTextWrap(false);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (invalido) 
  {
    Serial.println("Requisição inválida");
    display.setTextSize(1);
    display.setCursor(10, 2);
    display.println("Cotacao de Moedas");
    display.setTextSize(2);
    display.setCursor(10, 14);
    display.println("Erro Rede");
  }
  else 
  {
    // Linha Superior: Nome da moeda (esquerda) e Variação % com seta (direita)
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(nomeMoeda);

    // Monta a string da variação com o símbolo
    String strVar = "";
    if (variacaoMoeda > 0) {
      strVar += (char)0x18; // Seta para cima ↑
    } else if (variacaoMoeda < 0) {
      strVar += (char)0x19; // Seta para baixo ↓
    } else {
      strVar += (char)0x12; // Seta dupla ↕
    }
    strVar += String(abs(variacaoMoeda), 2) + "%";

    // Alinha a variação no canto superior direito (cada caractere no tamanho 1 tem 6px de largura)
    int posXVar = 128 - (strVar.length() * 6);
    display.setCursor(posXVar, 0);
    display.print(strVar);

    // Linha Central: Valor da Cotação
    String strValor = "R$ " + String(moeda, 2);

    // Se o valor for muito extenso (como Bitcoin > R$ 100.000,00), ajusta a fonte para não estourar a tela
    if (strValor.length() * 12 > 128) { 
      display.setTextSize(1);
      display.setCursor(0, 13);
    } else {
      display.setTextSize(2);
      display.setCursor(0, 10);
    }
    display.print(strValor);
  }
}

/**
  Desenha a barra de progresso no rodapé da tela (128x32)
*/
void printProgressBar(int percent) {
  if (!invalido) 
  {
    // Desenha o contorno da barra na parte inferior (linhas Y 27 a 31)
    display.drawRect(0, alturaDisplay - 4, larguraDisplay, 5, SSD1306_WHITE);

    int larguraInternaMinima = 1;
    int larguraInternaMaxima = larguraDisplay - 1;
    int larguraInterna = map(percent, 0, 100, larguraInternaMinima, larguraInternaMaxima);

    // Preenchimento interno
    display.fillRect(1, alturaDisplay - 3, larguraInterna, 3, SSD1306_WHITE);

    // Linhas verticais separadoras das 6 moedas
    display.fillRect(21, alturaDisplay - 4, 1, 5, SSD1306_BLACK);
    display.fillRect(41, alturaDisplay - 4, 1, 5, SSD1306_BLACK);
    display.fillRect(61, alturaDisplay - 4, 1, 5, SSD1306_BLACK);
    display.fillRect(81, alturaDisplay - 4, 1, 5, SSD1306_BLACK);
    display.fillRect(101, alturaDisplay - 4, 1, 5, SSD1306_BLACK);
  }
}