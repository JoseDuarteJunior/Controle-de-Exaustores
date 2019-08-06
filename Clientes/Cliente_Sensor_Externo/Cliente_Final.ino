//----------------------------------------------------------------------------------------------//
//                                  VERSÃO v.4. 06/08/2019                                      //
//----------------------------------------------------------------------------------------------//
//---------------------------CODIGO PRODUZIDO POR JOSÉ ANTONIO DUARTE JUNIOR--------------------//
//-------------------------------------ALL RIGHTS RESERVED 2019---------------------------------//
//----------------------------------------------------------------------------------------------//



//--------------------------PARTE DO WATCHDOG PARA AUTO-TRATAMENTO DE TRAVAMENTOS---------------//
#include "esp_system.h"
#include "esp_system.h"
//---------------------BIBLIOTECAS PARA DAR SUPORTE AOS SENSORES DE TEMPERATURA E COMUNICAÇÃO-------------------//
//suporte a sensores de humidade e aumentado a capacidade do protocolo de comunicação.
#include <OneWire.h>  //biblioecas para leitura dos sensores
#include <DallasTemperature.h>
//--------------------------------------------------------------------------------------------------------------//

//-------------------------------------PARTE DO SENSOR DE HUMIDADE----------------------------------------------//
#include "DHTesp.h"
#include "Ticker.h"

//------------------------BIBLIOTECAS DE COMUNICAÇÃO WIFI E UDP PARA ENVIO DOS PACOTES--------------------------//
#include <WiFi.h>  // bibliotecas para conexão wifi
#include <HTTPClient.h>
#include <WiFiUdp.h>//Biblioteca do UDP.
//--------------------------------------------------------------------------------------------------------------//
//----------------------DEFINIÇÃO DOS PINOS DE GPIO ONDE OS SENSORES SERÃO LIGADOS------------------------------//
/*-----define os pinos de gpio onde os sensores serão ligados-----*/
#define SENSOR_1_PIN 22 // For BUS 1 
#define SENSOR_2_PIN 23 // For BUS 2
#define LED 21 //led de status do sistema
int dhtPin = 17; // SENSOR DHT
//---------------------------------------------------------------------------------------------------------------//

//-----------------------------TEMPO SEM RESPOSTA QUE ATIVA GATILHO-----------------------------//
const int wdtTimeout = 3000;// 3 SEGUNDOS
hw_timer_t *timer = NULL; // INICIA TIMER EM NULL
//---------------------------------------------------------------------------------------------//
//---------------------------------METODO QUE REINICALIZA ESP----------------------------------//
void IRAM_ATTR resetModule() {
  ets_printf("reboot\n"); //PRINTA QUE VAI REINICIAR
  esp_restart(); // COMANDO QUE REINICIA ESP
}
//----------------------------------------------------------------------------------------------//
//----------------------DECLARA OBJETOS PARA OS SENSORES---------------------//
/*-----( Declara os objetos )-----*/
OneWire  Bus1(SENSOR_1_PIN);  // Create a 1-wire object
OneWire  Bus2(SENSOR_2_PIN);  // Create another 1-wire object
/*-- passa a referencia dos objetos para o modulo dallas temperature--*/
DallasTemperature sensors1(&Bus1);
DallasTemperature sensors2(&Bus2);
Ticker tempTicker;
DHTesp dht;
//---------------------------------------------------------------------------//

//----------PARTE QUE ENVIA INFORMAÇÃO PARA O SERVIDOR----------------//
WiFiUDP udp;//Cria um objeto da classe UDP.
String x; // vai carregar a leitura do sensores
const char* ssid = "GAM_RF"; // id de conexão na rede
const char* password =  ""; //senha de rede
//--------------------------------------------------------------------//

//----VARIAVEIS DOS SENSORES DE HUMIDADE E PNTO DE CONDENSAÇÃO-----//
String humidade,condensa;

//--------------------------SETUP RODA UMA VEZ-------------------------//
void setup()
{   
   pinMode(LED, OUTPUT); // led confirma transmissão de pacote
//-----------inicializa serial para depurar o projeto-------------------//
   Serial.begin(9600);
   Serial.println("Bem vindo ao Projeto Chernobyl GAM V.4");
   delay(4000);   //Delay nescessario antes do WIFI iniciar
   WiFi.begin(ssid, password); //inicia wifi
//-----------INICIALIZA BIBLIOTECA COM OS PARAMETROS DE MEDIDA-------------//
  sensors1.begin();
  sensors2.begin();
//---------------SETA A RESOLUÇÃO DE LEITURA DOS SENSORES PARA 11 BITS------------//
  sensors1.setResolution(11);
  sensors2.setResolution(11);

//--------------------------------INICIA SENSOR DE HUMIDADE---------------------//
  dht.setup(dhtPin, DHTesp::DHT11);
//-----------------DESCOBRE OS SENSORES----------------------------//
  discoverBusOneWireDevices();//descobre 1
  discoverBusTwoWireDevices();//descobre 2
//------------------LAÇO DE CONEXÃO WIFI---------------------------//
 while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
//-----------------------VARIAVEIS DO WATCHDOG-------------------------//
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
  timerAlarmEnable(timer);                          //enable interrupt
//---------------------------------------------------------------------//
}
//-------------------------LOOP PRINCIPAL---------------------------//
void loop()
{
//-----------------WATCHDOG PEGA TEMPO INICIAL----------------------//
  long loopTime = millis();  
  timerWrite(timer, 0); //RESETA O TIMER  (ALIMENTA WATCHDOG)
 //-----------------------PEGA A TEMPERATURA------------------------//
 sensors1.requestTemperatures();

//--------------------------PEGA HUMIDADE -------------------------//
  TempAndHumidity newValues = dht.getTempAndHumidity();
  float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
  if (isnan(newValues.temperature) || isnan(newValues.humidity)) 
  {}
  else{ 
   humidade = String(newValues.humidity);
   condensa = String(dewPoint);
  }
//------------------PRINTA TEMPERATURAS LIDAS----------------------//
 Serial.print("Temperatura Externa:");
 Serial.println(sensors1.getTempCByIndex(0));
//--PEGA O VALOR LIDO GRAVA NA VARIAVEL ADICIONANDO * QUE É O SEPARADO DE VALORES--//
  x = sensors1.getTempCByIndex(0);
  x = x + "*";
  x = "T01"+ x;//adiciona o T01 para dizer tipo de sensor e numero de sensor
//---------------------------------------------------------------------------------//
//------------PEGA A TEMPERATURA DO SEGUNDO SENSOR------------------//
  sensors2.requestTemperatures();
  Serial.print("Temperatura Externa:");
  Serial.println(sensors2.getTempCByIndex(0));
//------------CONCATENA VARIAVEL-----------------//
  x = x +(sensors2.getTempCByIndex(0));
  Serial.println(x);
//------------------------------INSERE VALORES FINAIS DO SENSOR DE HUMIDADE---------------------------------//
  Serial.print("Humidade");
  Serial.println(humidade);
  Serial.print("Condensação");
  Serial.println(condensa);
  x = x + "B" + humidade + "+" + condensa;
  x = x + "#";
  Serial.println("Pacote Final" + x);
  
//-------------------PARTE DO ENVIO DOS PACOTES UDP PARA O SERVIDOR---------------------//
 if(WiFi.status()== WL_CONNECTED){
  udp.beginPacket("10.20.3.81", 555);//Inicializa o pacote de transmissao ao IP e PORTA.
  udp.println(x);//Adiciona-se o valor ao pacote.
  udp.endPacket();//Finaliza o pacote e envia.
  Serial.println("PACOTE ENVIADO");
  digitalWrite(LED,HIGH); // acende led indicando que enviou
  delay(60);
  digitalWrite(LED,LOW);//apaga LED
 }
  delay(1000);
//----------CALCULA TEMPO TOTAL DO LOOP PARA WATCHDOG---------------//
  loopTime = millis() - loopTime;
//Serial.print("loop time is = ");
//Serial.println(loopTime); //DEVE SER MENOR QUE 3000 NESTE CASO DEU 1854 FOI DEIXADO 3000 DE ESTOURO
}
//-----ROTINA PARA MAPEAR SENSOR EM PINOS SEPARADOS ELE VARRE DUAS VEZES DESCOBRINDO SENSORES-----//
void discoverBusOneWireDevices(void)
{
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  while(Bus1.search(addr)) {
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      return;
    }
  }
  Serial.println();
  Serial.print("Done");
  Bus1.reset_search();
  return;
}
//---------------------------------------------------------------------------------------------//
//---------------SEGUNDA ROTINA PARA DESCOBRIR SEGUNDO SENSOR------------//
void discoverBusTwoWireDevices(void)
{
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  while(Bus2.search(addr)) {
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n\r");
      return;
    }
  }
  Serial.println();
  Bus2.reset_search();
  return;
}
//---------------------------------------------------------------------//
