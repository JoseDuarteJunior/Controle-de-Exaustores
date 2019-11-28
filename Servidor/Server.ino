#include <WiFi.h>
#include <WiFiUdp.h>
#include <elapsedMillis.h>
#include "time.h"
#include <EEPROM.h>
#define EEPROM_SIZE 1
//-----------------Parte da rede------------------------ 
const char* ssid     = ""; // sua rede
const char* password = "";
IPAddress local_IP(10, 20, 3, 81);// ip que deseja
IPAddress gateway(10, 20, 3, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(172, 17, 156, 209);
IPAddress secondaryDNS(8, 8, 8, 8);
//------------------------------------------------------
//led de conferencia para conexão wifi
int LED_BUILTIN = 2;
//////////////////////////
//-------------------parte que controla tempo de conexão de cliente-----------------
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 5000;
//-----------------------------------------------------------------------------------
// Parte do horário
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec =  -10800;
const int   daylightOffset_sec = 0;
int tempointerv;
int rele;
//-----------------------------------variaveis globais----------------------------------------
WiFiUDP udp;
String sensorE, sensorI, humidade, condensacao, modoinverno, req, tipodesensor, clientenumero,dayweek,reiniciar;
float sensorIf , sensorEf, condensacaof, humidadef,diferenca;
//---------------------------------------------------------------------------------------------
int modoverao;
//------------------------seta servidor para a porta 80--------------------------

WiFiServer server(80);

//-------------------------------------------------------------------------------
// -------------------variavel que guarda a requisição hhtp-------------------------
String header;
//----------------------------------------------------------------------------------




//-------------------------------------------variaveis do status dos exautores------------------------------------------------------------
String exaustor12Status = "OFF", exaustor34Status = "OFF", exaustor56Status = "OFF", exaustor78Status = "OFF",exaustor9Status = "OFF";
String  automatico, automaticoStatus;//controle="NULO";
//----------------------------------------------------------------------------------------------------------------------------------------



// ------------------------mapeaia variaveis para os pinos GPIO do esp-------------------------
int relayPin = 22; //
int relayPin1 = 23;
int relayPin2 = 16;
int relayPin3 = 17;

int relayPin4 = 18;
int automaticoM = 0;
int veraoStatus = 0;
int invernoStatus= 0;
int todosligados,todosdesligados = 3;

//----------------------------------------------------------------------------------------------



void setup() {
  EEPROM.begin(EEPROM_SIZE);
  automaticoM = EEPROM.read(0);
 
  veraoStatus = EEPROM.read(2);
  invernoStatus  = EEPROM.read(3);
   pinMode(LED_BUILTIN, OUTPUT);
   //////////////////////////
   pinMode(relayPin, OUTPUT);
   pinMode(relayPin1, OUTPUT);
   pinMode(relayPin2, OUTPUT);
   pinMode(relayPin3, OUTPUT);
   pinMode(relayPin4, OUTPUT);
   // Set outputs to LOW
   digitalWrite(relayPin, HIGH);
   digitalWrite(relayPin1, HIGH);
   digitalWrite(relayPin2, HIGH);
   digitalWrite(relayPin3, HIGH);
   digitalWrite(relayPin4, HIGH);
   Serial.begin(115200);
   modoinverno = "";
   automatico = "";
   dayweek ="";
   automaticoStatus="";
   modoverao= 0 ;
   diferenca=0;
   tempointerv = 0;
   rele = 0;
   Serial.print("Conectando em :");
   Serial.println(ssid);
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);
   WiFi.setSleep(false);
   WiFi.config(local_IP, gateway, subnet, secondaryDNS);
   while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
   }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  udp.begin(555);
  server.begin();

//nova parte
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}



void recebeudp () {
  if (udp.parsePacket() > 0) {
    req = "";
    while (udp.available() > 0) {
      char z = udp.read();
      req += z;
    }
  }
  //Serial.println("---" + req + "----");
  int posicao = req.indexOf("*");
  int posicao2 = req.indexOf("B");
  int posicao3 = req.indexOf("+");
  int posicao4 = req.indexOf("#");
  tipodesensor = req.substring(0, 1);
  clientenumero = req.substring(1, 3);
  sensorI = req.substring(3, posicao);
  sensorE = req.substring(posicao + 1, posicao2);
  humidade = req.substring(posicao2 + 1, posicao3);
  condensacao = req.substring(posicao3 + 1, posicao4);
  sensorIf = sensorI.toFloat();
  sensorEf = sensorE.toFloat();
  humidadef = humidade.toFloat();
  condensacaof = condensacao.toFloat();
  diferenca = abs(sensorIf - sensorEf);
// Serial.println("VALOR DA DIFERENÇA==");
 Serial.println(diferenca);
Serial.println(sensorIf);
Serial.println(sensorEf);


//---------------------------------------------------------modo verão--------------------------------------------------------------------

if (automaticoM == 1) {
      if  (   ((sensorIf >= sensorEf) && (veraoStatus == 1) && (diferenca >= 2) )   ){
      if  ( tempointerv != 2 &&tempointerv != 3 && tempointerv != 4 && tempointerv != 5 && tempointerv != 6 && tempointerv != 7 && tempointerv != 8 && tempointerv != 1 && dayweek != "Saturday") //original
        {
           ativa_reles ();
           Serial.println("Verão Status:");
           Serial.println(veraoStatus);
           Serial.println("Intervalo");
           Serial.println(tempointerv);
         }
       else{
        desativa_reles();
       }
       
    }
    else {
      desativa_reles();
    }
       
       
    
//-------------------------------------------------------------------------------------------------------------------------------------

    
     
if (automaticoM != 1)
{
  desativa_reles ();
}



}

 // Serial.println("Temperatura Interna:" + sensorI + "Temperatura Externa:" + sensorE + "Humidade:" + humidade + "Condensação:" + condensacao);
}



void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Falha ao obter a hora");
    return;
  }
   char timeStringBuff[50]; //50 chars should be enough
    char timeStringBuff2[25];
     char timeStringBuff3[25];
 // strftime(timeStringBuff2, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
   strftime(timeStringBuff2, sizeof(timeStringBuff), "%A", &timeinfo);
   strftime(timeStringBuff, sizeof(timeStringBuff), " %H", &timeinfo);
   strftime(timeStringBuff3, sizeof(timeStringBuff3), "%H:%M:%S", &timeinfo);
  //print like "const char*"
   tempointerv = atoi(timeStringBuff);
   dayweek = String(timeStringBuff2);
    reiniciar = String(timeStringBuff3);
 //// if (dayweek == "Friday"){
 if(reiniciar == "15:30:20")
 {
  ESP.restart();
 }
   //Serial.println(reiniciar);
 // }
  //Serial.println(timeStringBuff2);
   tempointerv = atoi(timeStringBuff); 
  
//Serial.println(tempointerv);



}







void desativa_reles(){

  
  digitalWrite(relayPin, HIGH);
  //Serial.println("desativado relé 1"); 
  exaustor12Status = "OFF";
  digitalWrite(relayPin1, HIGH); 
//  Serial.println("desativado relé 2"); 
  exaustor34Status = "OFF";
  digitalWrite(relayPin2, HIGH); 
 // Serial.println("desativado relé 3"); 
  exaustor56Status = "OFF";
  digitalWrite(relayPin3, HIGH);
//  Serial.println("desativado relé 4"); 
  exaustor78Status = "OFF";
  digitalWrite(relayPin4, HIGH);
 // Serial.println("desativado relé 5"); 
  exaustor9Status = "OFF";
   Serial.println("Todos Exaustores Desligados");
 
  
 
}


void ativa_reles() {

    
       
           Serial.println("Ligou relé 1");
           digitalWrite(relayPin, LOW); 
           exaustor12Status = "ON";
        

           Serial.println("Ligou relé 2");
           digitalWrite(relayPin1, LOW); 
           exaustor34Status = "ON";
     
            Serial.println("Ligou relé 3");
            digitalWrite(relayPin2, LOW); 
            exaustor56Status = "ON";
 
            Serial.println("Ligou relé 4");
            digitalWrite(relayPin3, LOW); 
            exaustor78Status = "ON";
       
            Serial.println("Ligou relé 5");
            digitalWrite(relayPin4, LOW); 
            exaustor9Status="ON";
            Serial.println("Todos Exaustores Ligados");
          
            
          
}



void loop() {

  printLocalTime();
  recebeudp ();
 
  //Serial.println("Temperatura Interna:" + sensorI + "Temperatura Externa:" + sensorE + "Humidade:" + humidade + "Condensação:" + condensacao);
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("Novo Cliente");          // print a message out in the serial port
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;// make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {           // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            ////////////////////////////////////////////////////teste////////////////////////////////////
              // turns the GPIOs on and off
            if (header.indexOf("GET /12/ON") >= 0) {//era  if (header.indexOf("GET /26/on") >= 0)
              Serial.println("Exaustor 1 e 2  on");
              if(exaustor12Status != "ON"){
              exaustor12Status = "ON";
              todosdesligados = 1;
              digitalWrite(relayPin, LOW); // pino do relé
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81\">");
              client.print("</head>");
              }
              
            } else if (header.indexOf("GET /12/OFF") >= 0) {// era else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("Exaustor 1 e 2 off");
              if(exaustor12Status != "OFF"){
              exaustor12Status = "OFF";
              todosligados = 1;
              digitalWrite(relayPin, HIGH); 
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81\">");
              client.print("</head>");
               }
            } else if (header.indexOf("GET /34/ON") >= 0) {
              Serial.println("Exaustor 3 e 4  on");
              if(exaustor34Status != "ON"){
              todosdesligados = 1;
              exaustor34Status = "ON";
              digitalWrite(relayPin1, LOW); 
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
               }
            } else if (header.indexOf("GET /34/OFF") >= 0) {
              Serial.println("Exaustor 3 e 4  off");
              if(exaustor34Status != "OFF"){
               todosligados = 1;
              digitalWrite(relayPin1, HIGH); 
              exaustor34Status = "OFF";
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
               }
            } else if (header.indexOf("GET /56/ON") >= 0) {
              Serial.println("Exaustor 5 e 6  on");
              if(exaustor56Status != "ON"){
                todosdesligados = 1;
              exaustor56Status = "ON";
              digitalWrite(relayPin2, LOW); 
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
               }
            } else if (header.indexOf("GET /56/OFF") >= 0) {
              Serial.println("Exaustor 5 e 6  off");
              if(exaustor56Status != "OFF"){
                todosligados = 1;
              digitalWrite(relayPin2, HIGH); 
              exaustor56Status = "OFF";
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
               }
            } else if (header.indexOf("GET /78/ON") >= 0) {
              Serial.println("Exaustor 7 e 8  on");
              if(exaustor78Status != "ON"){
                 todosdesligados = 1;
                digitalWrite(relayPin3, LOW); 
                exaustor78Status = "ON";
                client.print("<HEAD>");
                client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
                client.print("</head>");
               }
            }else if (header.indexOf("GET /78/OFF") >= 0) {
              Serial.println("Exaustor 7 e 8  off");
              if(exaustor78Status != "OFF"){
                
                todosligados = 1;
              digitalWrite(relayPin3, HIGH); 
              exaustor78Status = "OFF";
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
               }
            } else if (header.indexOf("GET /9/ON") >= 0) {
              Serial.println("Exaustor 9  on");
              if(exaustor9Status != "ON"){
                 todosdesligados = 1;
              digitalWrite(relayPin4, LOW); 
              exaustor9Status = "ON";
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
               }
              } else if (header.indexOf("GET /9/OFF") >= 0) {
              Serial.println("Exaustor 9  off");
              if(exaustor9Status != "OFF"){
                todosligados = 1;
              exaustor9Status = "OFF";
              digitalWrite(relayPin4, HIGH);
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"20;URL=http://10.20.3.81 \">");
              client.print("</head>");
               }
            } else if (header.indexOf("GET /invernoS/OFF") >= 0) {
              //Serial.println("GPIO 27 on");
              if(invernoStatus != 0){
                invernoStatus = 0;
                EEPROM.write(3, invernoStatus);
                EEPROM.commit();
              //digitalWrite(output27, HIGH);
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
               }
            } 
             else if (header.indexOf("GET /invernoS/ON") >= 0) {
               if(invernoStatus != 1){
              //Serial.println("GPIO 27 on");
              invernoStatus = 1;
                 EEPROM.write(3, invernoStatus);
              EEPROM.commit();
              veraoStatus = 0;
              modoverao = 0 ;
              desativa_reles();
             
              //digitalWrite(output27, HIGH);
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
               }
            } 
            else if (header.indexOf("GET /veraoS/OFF") >= 0) {
              if(veraoStatus != 0){
              veraoStatus = 0 ;
                EEPROM.write(2,veraoStatus);
              EEPROM.commit();
              desativa_reles();
              //digitalWrite(output27, HIGH);
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81\">");
              client.print("</head>");
               }
            } 
             else if (header.indexOf("GET /veraoS/ON") >= 0) {
              //Serial.println("GPIO 27 on");
              
              if(veraoStatus != 1){
              veraoStatus = 1 ;
               EEPROM.write(2,veraoStatus);
              EEPROM.commit();
              
              invernoStatus = 0;
              EEPROM.write(3,invernoStatus);
              EEPROM.commit();
              desativa_reles();
          
              //digitalWrite(output27, HIGH);
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
              }
            } 
            else if (header.indexOf("GET /controleA/ON") >= 0) {
              //Serial.println("GPIO 27 on");

              Serial.println("Ativou automatico");
              desativa_reles();
              automaticoM= 1 ;
              EEPROM.write(0,automaticoM);
              EEPROM.commit();
              Serial.println("State saved in flash memory");              
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
              client.print("</head>");
              
            } 
            else if (header.indexOf("GET /controleA/OFF") >= 0) {
              //Serial.println("GPIO 27 on");
              Serial.println("Ativou automatico");
              desativa_reles();
              automaticoM= 0 ;
              EEPROM.write(0,automaticoM);
              EEPROM.commit();
              Serial.println("State saved in flash memory");  
              invernoStatus = 0;
              EEPROM.write(3, invernoStatus);
              EEPROM.commit();
              veraoStatus = 0;
              EEPROM.write(2,veraoStatus);
              EEPROM.commit();
              client.print("<HEAD>");
              client.print("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"2;URL=http://10.20.3.81 \">");
            client.print("</head>");
              
            } 
            /////////////////////////////////////////////////////////////////////////////////////////
         

            // Display the HTML web page
            client.println("<!DOCTYPE html><html><head>");
            client.println("<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"20; URL=http://10.20.3.81 \">");
              client.println(" <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js\"></script>");
            client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.7/umd/popper.min.js\"></script>");
            client.println("<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.3.1/js/bootstrap.min.js\"></script>");
            client.println("<script src=\"https://kit.fontawesome.com/94fedfc3d5.js\"></script>");
            client.println("</head>");
            client.println("<style type=\"text/css\">body { background:#143c96 !important; }</style>");
        
            

            client.println("<div class=\"jumbotron jumbotron-fluid text-center \">");
            client.println("<div class=\"container\">");
           
            client.println(" <p class=\"lead\">Sistema para monitorameno e ativação dos exaustores</p>");
            
             


            client.println("<div class=\"container\">");
            client.println("<div id=\"temperaturas\"class=\"card-deck\">");
            client.println("<div class=\"card\">");// text-white
            client.println("<div class=\"card-header text-center mb-2 text-muted\">Temperatura Interna do CD</div>");
            client.println("<div class=\"card-body text-center\">");
            client.println("<span align=\"center\" id=\"TemperaturaInterna\">");
            client.println(sensorI);
            client.println("</span> ºC <i class=\"fas fa-thermometer-half\"></i></div></div><div class=\"card\"><div class=\"card-header text-center mb-2 text-muted\">Temperatura Externa do CD</div>");
            client.println("<div class=\"card-body text-center\"><span align=\"center\" id=\"TemperaturaExterna\">");
            client.println(sensorE);
            client.println("</span>ºC <i class=\"fas fa-thermometer-half\"></i>");
            client.println("</div> </div>");
            client.println( "<div class=\"card\">");
            client.println("<div class=\"card-header text-center mb-2 text-muted\">Humidade no CD</div>");
            client.println("<div class=\"card-body text-center\">");
            client.println("<span align=\"center\" id=\"HumidadeR\">");
            client.println(humidade);
            client.println(" </span>% <i class=\"fas fa-tint\"></i>");
           // client.println(" <span align="center" id="ADCValue3" style="display:none;"></span>);
            client.println("</div> </div> </div></div>");
          
            
            client.println("<p></p>");
            client.println("<div class=\"container text-center\">");
          



           // parte dos controles automaticos 8888*****************************************************************************************************
            client.println("<div class=\"card-deck\" id=\"supervisor\">");
            
            client.println("<div class=\"card \"><div class=\"card-header text-center mb-2 text-muted\">Controle Automático");
            if(automaticoM== 1){
             
               client.println("<a href=\"/controleA/OFF\"><button type=\"button\" class=\"btn btn-danger\">DESLIGAR</button></a></div>");


              client.println(" <ul class=\"list-group \"><li class=\"list-group-item\">");

            
           // if (modoinverno == "SIM"){
            //  client.println("<h4 class=\"card-title lead\">Modo Inverno Ligado <div id=\"status\" class=\"spinner-grow text-success\"> </div> </h4>");
           // }
           // else{
           // client.println("<h4 class=\"card-title lead\">Modo Inverno Desligado <div id=\"status\" class=\"badge badge-danger\"> </div> </h4>");
           // }
            if (invernoStatus== 0) {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\">Modo Inverno Desligado <div id=\"status\" class=\"badge badge-danger\"> </div> </h4>");
              client.println("<p class=\"card-text lead\"> <small>Este modo de operação ativa rotinas pré estabelecidas de acionamento dos exaustores para um melhor desempenho e economia no periodo de inverno</small></p>");
              client.println("<p></p>");
              client.println("<p><a href=\"/invernoS/ON\"><button type=\"button\" class=\"btn btn-success\">LIGAR</button></a></p>");
              
            } else {
              client.println("<h4 class=\"card-title lead\">Modo Inverno Ligado <div id=\"status\" class=\"spinner-grow text-success\"> </div> </h4>");
              client.println("<p class=\"card-text lead\"> <small>Este modo de operação ativa rotinas pré estabelecidas de acionamento dos exaustores para um melhor desempenho e economia no periodo de inverno</small></p>");
              client.println("<p></p>");
              client.println("<p><a href=\"/invernoS/OFF\"><button type=\"button\" class=\"btn btn-danger\">DESLIGAR</button></a></p>");

             } 



           
            
         
            client.println("<p></p></li><li class=\"list-group-item\">");

            if (veraoStatus== 0) {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\">Modo Verão Desligado <div id=\"status\" class=\"badge badge-danger\"> </div> </h4>");
              client.println("<p class=\"card-text lead\"> <small>Este modo de operação ativa rotinas pré estabelecidas de acionamento dos exaustores para um melhor desempenho e economia no periodo de verão</small></p>");
              client.println(tempointerv);
              client.println(" Horas");
              client.println("<p></p>");
              client.println("<p><a href=\"/veraoS/ON\"><button type=\"button\" class=\"btn btn-success\">LIGAR</button></a></p>");
              
            } else {
              client.println("<h4 class=\"card-title lead\">Modo Verão Ligado <div id=\"status\" class=\"spinner-grow text-success\"> </div> </h4>");
              client.println("<p class=\"card-text lead\"> <small>Este modo de operação ativa rotinas pré estabelecidas de acionamento dos exaustores para um melhor desempenho e economia no periodo de verão</small></p>");
              client.println(tempointerv);
              client.println(" Horas");
              client.println("<p></p>");
              client.println("<p><a href=\"/veraoS/OFF\"><button type=\"button\" class=\"btn btn-danger\">DESLIGAR</button></a></p>");

             } 



            
            client.println("</li></ul></div>");


            client.println("<div class=\"card \">");
            client.println("<div  class=\"card-header text-center mb-2 text-muted\">Controle Manual</div>");
            
            client.println("<table class=\"table table-bordered\">");
            client.println("<thead><tr><th>");
            client.println("<li class=\"list-group-item text-center\">");

            
            if (exaustor12Status=="OFF") {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\"> Exaustor 1 e 2:" + exaustor12Status +" </h4>");
              client.println("<p></p>");
              client.println("<a href=\"#\" class=\"btn btn-success disabled\" role=\"button\" aria-disabled=\"true\">LIGAR</a>");
             
            } else {
              client.println("<h4 class=\"card-title lead\"> Exaustor 1 e 2:" + exaustor12Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
              client.println("<p></p>");
              client.println("<a href=\"#\" class=\"btn btn-danger disabled\" role=\"button\" aria-disabled=\"true\">DESLIGAR</a>");
            } 
            client.println("</th><th>");
            client.println("<li class=\"list-group-item text-center\">");
            
            if (exaustor34Status=="OFF") {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\"> Exaustor 3 e 4:"+ exaustor34Status +" </h4>");
              client.println("<p></p>");
              client.println("<a href=\"#\" class=\"btn btn-success disabled\" role=\"button\" aria-disabled=\"true\">LIGAR</a>");
            } else {
              client.println("<h4 class=\"card-title lead\"> Exaustor 3 e 4:" + exaustor34Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
              client.println("<p></p>");
              client.println("<a href=\"#\" class=\"btn btn-danger disabled\" role=\"button\" aria-disabled=\"true\">DESLIGAR</a>");
            } 
            client.println("</th><th>");
            client.println("<li class=\"list-group-item text-center\">");
            
            if (exaustor56Status=="OFF") {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\"> Exaustor 5 e 6:"+ exaustor56Status +" </h4>");
              client.println("<p></p>");
              client.println("<a href=\"#\" class=\"btn btn-success disabled\" role=\"button\" aria-disabled=\"true\">LIGAR</a>");
            } else {
               client.println("<h4 class=\"card-title lead\"> Exaustor 5 e 6:" + exaustor56Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
               client.println("<p></p>");
                client.println("<a href=\"#\" class=\"btn btn-danger disabled\" role=\"button\" aria-disabled=\"true\">DESLIGAR</a>");
            }  
            client.println("</th></tr><tr><th>");

            client.println("<li class=\"list-group-item text-center\">");
            
             if (exaustor78Status=="OFF") {//era    if (output26State=="off") {
             client.println("<h4 class=\"card-title lead\"> Exaustor 7 e 8:"+ exaustor78Status +" </h4>");
             client.println("<p></p>");
            client.println("<a href=\"#\" class=\"btn btn-success disabled\" role=\"button\" aria-disabled=\"true\">LIGAR</a>");
            } else {
               client.println("<h4 class=\"card-title lead\"> Exaustor 7 e 8:" + exaustor78Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
               client.println("<p></p>");
             client.println("<a href=\"#\" class=\"btn btn-danger disabled\" role=\"button\" aria-disabled=\"true\">DESLIGAR</a>");
            } 
            client.println("</th><th>");

            client.println("<li class=\"list-group-item text-center\">");
            
           if (exaustor9Status=="OFF") {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\"> Exaustor 9:"+ exaustor9Status +" </h4>");
            client.println("<p></p>");
            client.println("<a href=\"#\" class=\"btn btn-success disabled\" role=\"button\" aria-disabled=\"true\">LIGAR</a>");
            } else {
               client.println("<h4 class=\"card-title lead\"> Exaustor 9:" + exaustor9Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
               client.println("<p></p>");
             client.println("<a href=\"#\" class=\"btn btn-danger disabled\" role=\"button\" aria-disabled=\"true\">DESLIGAR</a>");
            } 
            client.println("</th></tr></thead></table>");

            client.println("</div></div>");            
            }else{
               client.println("<a href=\"/controleA/ON\"><button type=\"button\" class=\"btn btn-success \">LIGAR</button></a></div>");
               client.println(" <ul class=\"list-group \"><li class=\"list-group-item\">");

            
           // if (modoinverno == "SIM"){
            //  client.println("<h4 class=\"card-title lead\">Modo Inverno Ligado <div id=\"status\" class=\"spinner-grow text-success\"> </div> </h4>");
           // }
           // else{
           // client.println("<h4 class=\"card-title lead\">Modo Inverno Desligado <div id=\"status\" class=\"badge badge-danger\"> </div> </h4>");
           // }
            if (invernoStatus== 0 ) {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\">Modo Inverno Desligado <div id=\"status\" class=\"badge badge-danger disabled\"> </div> </h4>");
              client.println("<p class=\"card-text lead\"> <small>Este modo de operação ativa rotinas pré estabelecidas de acionamento dos exaustores para um melhor desempenho e economia no periodo de inverno</small></p>");
              client.println("<p></p>");
             
              client.println("<a href=\"#\" class=\"btn btn-success disabled\" role=\"button\" aria-disabled=\"true\">LIGAR</a>");
              
            } else {
              client.println("<h4 class=\"card-title lead\">Modo Inverno Ligado <div id=\"status\" class=\"spinner-grow text-success\"> </div> </h4>");
              client.println("<p class=\"card-text lead\"> <small>Este modo de operação ativa rotinas pré estabelecidas de acionamento dos exaustores para um melhor desempenho e economia no periodo de inverno</small></p>");
              client.println("<p></p>");
              client.println("<a href=\"#\" class=\"btn btn-danger disabled\" role=\"button\" aria-disabled=\"true\">DESLIGAR</a>");

             } 



           
            
         
            client.println("<p></p></li><li class=\"list-group-item\">");

            if (veraoStatus== 0) {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\">Modo Verão Desligado <div id=\"status\" class=\"badge badge-danger\"> </div> </h4>");
              client.println("<p class=\"card-text lead\"> <small>Este modo de operação ativa rotinas pré estabelecidas de acionamento dos exaustores para um melhor desempenho e economia no periodo de verão</small></p>");
              client.println("<p></p>");
              client.println("<a href=\"#\" class=\"btn btn-success disabled\" role=\"button\" aria-disabled=\"true\">LIGAR</a>");
              
            } else {
              client.println("<h4 class=\"card-title lead\">Modo Verão Ligado <div id=\"status\" class=\"spinner-grow text-success\"> </div> </h4>");
              client.println("<p class=\"card-text lead\"> <small>Este modo de operação ativa rotinas pré estabelecidas de acionamento dos exaustores para um melhor desempenho e economia no periodo de verão</small></p>");
              client.println("<p></p>");
              client.println("<a href=\"#\" class=\"btn btn-danger disabled\" role=\"button\" aria-disabled=\"true\">DESLIGAR</a>");

             } 



            
            client.println("</li></ul></div>");


            client.println("<div class=\"card \">");
            client.println("<div  class=\"card-header text-center mb-2 text-muted\">Controle Manual</div>");
            
            client.println("<table class=\"table table-bordered\">");
            client.println("<thead><tr><th>");
            client.println("<li class=\"list-group-item text-center\">");

            
            if (exaustor12Status=="OFF") {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\"> Exaustor 1 e 2:" + exaustor12Status +" </h4>");
              client.println("<p></p>");
              client.println("<p><a href=\"/12/ON\"><button type=\"button\" class=\"btn btn-success\">LIGAR</button></a></p>");
            } else {
              client.println("<h4 class=\"card-title lead\"> Exaustor 1 e 2:" + exaustor12Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
              client.println("<p></p>");
              client.println("<p><a href=\"/12/OFF\"><button type=\"button\" class=\"btn btn-danger\">DESLIGAR</button></a></p>");
            } 
            client.println("</th><th>");
            client.println("<li class=\"list-group-item text-center\">");
            
             if (exaustor34Status=="OFF") {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\"> Exaustor 3 e 4:"+ exaustor34Status +" </h4>");
              client.println("<p></p>");
              client.println("<p><a href=\"/34/ON\"><button type=\"button\" class=\"btn btn-success\">LIGAR</button></a></p>");
            } else {
              client.println("<h4 class=\"card-title lead\"> Exaustor 3 e 4:" + exaustor34Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
              client.println("<p></p>");
              client.println("<p><a href=\"/34/OFF\"><button type=\"button\" class=\"btn btn-danger\">DESLIGAR</button></a></p>");
            } 
            client.println("</th><th>");
            client.println("<li class=\"list-group-item text-center\">");
            
            if (exaustor56Status=="OFF") {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\"> Exaustor 5 e 6:"+ exaustor56Status +" </h4>");
              client.println("<p></p>");
              client.println("<p><a href=\"/56/ON\"><button type=\"button\" class=\"btn btn-success\">LIGAR</button></a></p>");
            } else {
               client.println("<h4 class=\"card-title lead\"> Exaustor 5 e 6:" + exaustor56Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
               client.println("<p></p>");
                client.println("<p><a href=\"/56/OFF\"><button type=\"button\" class=\"btn btn-danger\">DESLIGAR</button></a></p>");
            } 
            client.println("</th></tr><tr><th>");

            client.println("<li class=\"list-group-item text-center\">");
            
             if (exaustor78Status=="OFF") {//era    if (output26State=="off") {
             client.println("<h4 class=\"card-title lead\"> Exaustor 7 e 8:"+ exaustor78Status +" </h4>");
             client.println("<p></p>");
              client.println("<p><a href=\"/78/ON\"><button type=\"button\" class=\"btn btn-success\">LIGAR</button></a></p>");
            } else {
               client.println("<h4 class=\"card-title lead\"> Exaustor 7 e 8:" + exaustor78Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
               client.println("<p></p>");
              client.println("<p><a href=\"/78/OFF\"><button type=\"button\" class=\"btn btn-danger\">DESLIGAR</button></a></p>");
            } 
            client.println("</th><th>");

            client.println("<li class=\"list-group-item text-center\">");
            
          if (exaustor9Status=="OFF") {//era    if (output26State=="off") {
              client.println("<h4 class=\"card-title lead\"> Exaustor 9:"+ exaustor9Status +" </h4>");
            client.println("<p></p>");
              client.println("<p><a href=\"/9/ON\"><button type=\"button\" class=\"btn btn-success\">LIGAR</button></a></p>");
            } else {
               client.println("<h4 class=\"card-title lead\"> Exaustor 9:" + exaustor9Status +" </h4> <div id=\"status\" class=\"spinner-grow text-success\"> </div>");
               client.println("<p></p>");
              client.println("<p><a href=\"/9/OFF\"><button type=\"button\" class=\"btn btn-danger\">DESLIGAR</button></a></p>");
            } 
            client.println("</th></tr></thead></table>");

            client.println("</div></div>");

            }

           // client.println("<p class=\"text-center lead\">Temperatura de Condensacao");
            //client.println(condensacao);
            //client.println(" °C </p>");
            
            //client.println(modoinverno);
           // if (modoinverno == "SIM"){
             // client.println("<h4 class=\"card-title lead\">SIM <div id=\"status\" class=\"spinner-grow text-success\"> </div> </h4>");
           // }
          //  else{
          //  client.println("<h4 class=\"card-title lead\">NÃO<div id=\"status\" class=\"badge badge-danger\"> </div> </h4>");
          //  }
            
         //   client.println("</p></div>");

        
         
            ///////////////////////////////////////////////////////////////////////////////////////////////////

            

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
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
