#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <WebSocketsServer.h>
#include <FS.h> // TODO - mudar o sistema de arquivos para o LittleFS

DNSServer dnsServer;
AsyncWebServer server(80); // declara a porta 80 para o WebServer
WebSocketsServer webSocket = WebSocketsServer(81); // declara a porta 81 para o WebSocketServer

int leds[] = {0, 0, 0}; // string utilizada para guardar os estados dos leds (0 apagado, 1 aceso)

// função para aplicar os estados dos leds nas portas correspondentes
// TODO - melhorar a lógica para remover as condicionais (converter o char para int)
void changeLedStatus() {
  analogWrite(D1, leds[0]);
  analogWrite(D3, leds[1]);
  analogWrite(D6, leds[2]);
  Serial.print("led 1 -> ");
  Serial.println(leds[0]);
  Serial.print("led 2 -> ");
  Serial.println(leds[1]);
  Serial.print("led 3 -> ");
  Serial.println(leds[2]);
  
}

// função que trata os eventos do WebSocketServer
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  int8_t index;
  int strength;
  String payloadString;
  char ledsString[8];
    
  switch(type) {
    
// em caso de deconexão, apenas depura uma mensagem de desconexão
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Desconectado!\n", num);
      break;

// assim que recebe uma nova conexão, envia o estado dos leds para o novo cliente
// TODO - verificar a consistência da informação recebida, para evitar erros
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Conexão estabelecida com %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        Serial.print("Enviando leds: ");
        sprintf(ledsString, "%03d%03d%03d", leds[0], leds[1], leds[2]);
        webSocket.sendTXT(num, ledsString); // envia o estado dos leds para o novo cliente (num)
      }
      break;

// quando recebe uma solicitação de alteração do estado dos leds por algum cliente
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
// altera a variável que guarda o estado dos leds conforme solicitação do cliente
// TODO - melhorar a lógica para evitar as condicionais
      payloadString = ((char*)payload);
      index = payloadString.charAt(0) - '0';
      strength = atoi(payloadString.substring(1).c_str());
      leds[index] = strength;
      sprintf(ledsString, "%03d%03d%03d", leds[0], leds[1], leds[2]);
      Serial.print("Informação recebida, enviando leds: ");
      Serial.println(ledsString);
      changeLedStatus();
      webSocket.broadcastTXT(ledsString); // faz um broadcast do novo estado dos leds para TODOS os clientes
      break;
  }
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
    Serial.println(requestedFile);
  
    AsyncWebServerResponse *response = nullptr;  // declara variável que será usada na resposta

// verifica qual arquivo está sendo requisitado, e o envia de acordo
// TODO - melhorar a lógica para evitar o encadeamento de condicionais
    if (requestedFile.equals("/estilos.css")) {
      response = request->beginResponse(SPIFFS, "/estilos.css", "text/css");
    } else if (requestedFile.equals("/script.js")) {
      response = request->beginResponse(SPIFFS, "/script.js", "text/javascript");
    } else if (requestedFile.equals("/logo-fundo-escuro.png")) {
      response = request->beginResponse(SPIFFS, "/logo-fundo-escuro.png", "image/png");
    } else {
      response = request->beginResponse(SPIFFS, "/index.html", "text/html");
    }
  
    request->send(response); // envia o arquivo
  }
};

void setup(){

// pinagem das portas dos leds
// os números NÃO correspondem aos gravados na placa!!!
// TODO - listar todas as portas em constantes
  pinMode(D1, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D6, OUTPUT);
  Serial.begin(115200); // habilita a saída de depuração
  Serial.setDebugOutput(true); // habilita as saídas de depuração da biblioteca wi-fi (o padrão é true e ela seta para false)

  WiFi.softAP("MAQUETE_WIFI"); // configura o Access Point
  dnsServer.start(53, "*", WiFi.softAPIP()); // configura o servidor de DNS (porta 53) apontando para o Access Point
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // direciona todos os requests para o CaptiveRequestHandler

  webSocket.onEvent(webSocketEvent); // direciona todos os eventos do servidor WebSocket para o webSocketEvent
  webSocket.begin(); // inicia o sevidor WebSocket

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
