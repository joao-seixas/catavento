#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <WebSocketsServer.h>
#include <FS.h> // TODO - mudar o sistema de arquivos para o LittleFS

DNSServer dnsServer;
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// array utilizado para definir quais portas serão usadas
uint8_t ports[] = {D3, D6, D1};

// TODO - criar array dinamicamente, conforme tamanho do array das portas
uint8_t leds[] = {0, 0, 0};

void changeLedStatus() {
  Serial.println("Atualizando portas...");
  for (uint8_t index = 0; index < sizeof(ports) / sizeof(uint8_t); index++) {
    analogWrite(ports[index], leds[index]);
    Serial.print("Porta: ");
    Serial.print(ports[index]);
    Serial.print(" - Valor: ");
    Serial.println(leds[index]);
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  int8_t index;

  switch(type) {
  
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Desconectado!\n", num);
      break;

    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Conexão estabelecida com %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        Serial.print("Enviando leds...");
      
        webSocket.sendBIN(num, leds, sizeof(leds)); // envia o estado dos leds para o novo cliente (num)
      }
      break;

    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      Serial.println("Erro! Recebido texto (esperado binário)");
      break;

    case WStype_BIN:
      // recebe dois bytes (1º byte LED, 2º byte VALOR)
      // TODO - verificar a consistência da informação recebida, para evitar erros
      Serial.printf("[WSc] get binary length: %u\n", length);

      index = payload[0];
      leds[index] = payload[1];
      changeLedStatus();

      // envia o estado dos leds para todos os clientes (broadcast)
      webSocket.broadcastBIN(leds, sizeof(leds));
      break;
  }
}

AsyncWebServerResponse* getResponse(const String& requestedFile) {
  AsyncWebServerRequest* response = nullptr;

  if (requestedFile.equals("/estilos.css"))
    return response->beginResponse(SPIFFS, "/estilos.css", "text/css");
  
  if (requestedFile.equals("/script.js"))
    return response->beginResponse(SPIFFS, "/script.js", "text/javascript");
  
  if (requestedFile.equals("/logo-fundo-escuro.png"))
    return response->beginResponse(SPIFFS, "/logo-fundo-escuro.png", "image/png");

  return response->beginResponse(SPIFFS, "/index.html", "text/html");
}

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    String requestedFile = request->url();
    AsyncWebServerResponse *response = getResponse(requestedFile);
    request->send(response);
    Serial.print("Arquivo requisitado -> ");
    Serial.println(requestedFile);
  }
};

void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true); // habilita as saídas de depuração da biblioteca wi-fi
  delay(1000);
  Serial.flush();
  Serial.println();

  for (uint8_t index = 0; index < sizeof(ports) / sizeof(uint8_t); index++) {
    pinMode(ports[index], OUTPUT);
    Serial.print("Definindo a porta ");
    Serial.print(ports[index]);
    Serial.println(" em modo de SAÍDA...");
  }

  WiFi.softAP("MAQUETE_WIFI");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

  webSocket.onEvent(webSocketEvent);
  webSocket.begin();

// inicia o sistema de arquivos e aguarda sua montagem para só então iniciar o servidor web
// (para evitar que o servidor esteja disponível antes dos arquivos)
  if( !SPIFFS.begin()){
    Serial.println("Erro na montagem do sistema de arquivos...");
    while(1);
  } else {
    Serial.println("Sistema de arquivos montado com sucesso!");
    server.begin();
  }
}

void loop(){
  webSocket.loop();
  dnsServer.processNextRequest();
}
