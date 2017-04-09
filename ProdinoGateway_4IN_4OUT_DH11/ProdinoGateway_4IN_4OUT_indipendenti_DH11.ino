

/**************************************************************************
    Souliss - Controlla 4 ingressi e 4 uscite indipendenti, 1 sensore di temperatura/umidità

    
    Runs on KMP Electronics ProDINo Wroom (WiFi ESP8266)

***************************************************************************/
// RESET OGNI 20 MIN SE NON E' COLLEGATO AL GATEWAY
//#define  VNET_RESETTIME_INSKETCH
//#define VNET_RESETTIME      0x00042F7 // ((20 Min*60)*1000)/70ms = 17143 => 42F7
//#define VNET_HARDRESET      ESP.reset()


// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

#include "bconf/DINo_WiFi_Bridge_RS485.h"   // Define the board type
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/Webhook.h"                   // Enable DHCP and DNS
#include "conf/DynamicAddressing.h"         // Use dynamic address

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "MisiSpot"
#define WiFi_Password           "multiactivity"    

// Include framework code and libraries
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <DHT.h>

/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"

#define DHTTYPE DHT11   // Tipo di sensore DHT 11
#define DHTPIN  5         // PIN D0 GROOVE

// DHT sensor
DHT dht(DHTPIN, DHTTYPE, 11); // for ESP8266 use dht(DHTPIN, DHTTYPE, 11)

#define LIGHT1                  0           // This is the memory slot used for the execution of the logic
#define LIGHT2                  1           
#define LIGHT3                  2           
#define LIGHT4                  3
#define OUT1                    4           
#define OUT2                    5           
#define OUT3                    6           
#define OUT4                    7
#define HUMIDITY                8               // Leave 2 slots for T53
#define TEMP0                   10              // Leave 2 slots for T52           



void setup()
{   
    // Init the board
    delay(1); // Ritardo di setup per permettere al router di effettuare il boot in millisecondi
    
    InitDINo();

    // Connect to the WiFi network and get an address from DHCP
    GetIPAddress();                           
    SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp 
     
    // This is the vNet address for this node, used to communicate with other
    // nodes in your Souliss network
    SetAddress(0xAB01, 0xFF00, 0x0000);
    SetAsPeerNode(0xAB02, 1);
    SetAsPeerNode(0xAB03, 2);
    
    dht.begin();                                // initialize temperature sensor

    Set_T53(HUMIDITY);
    Set_T52(TEMP0);
    
    // Define Simple Light logics for the relays
    Set_T13(LIGHT1);
    Set_T13(LIGHT2);
    Set_T13(LIGHT3);
    Set_T13(LIGHT4);
    Set_SimpleLight(OUT1);
    Set_SimpleLight(OUT2);    
    Set_SimpleLight(OUT3);    
    Set_SimpleLight(OUT4);        
    
    // This node will act as addressing server for the other peers in the network
  //  SetAddressingServer();
}

void loop()
{ 
    // Here we start to play
    EXECUTEFAST() {                     
        UPDATEFAST();   
        
        FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
        
            DigIn2State(IN1, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, LIGHT1);          // Read inputs from IN1
            DigIn2State(IN2, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, LIGHT2);          // Read inputs from IN2
            DigIn2State(IN3, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, LIGHT3);          // Read inputs from IN3
            DigIn2State(IN4, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, LIGHT4);          // Read inputs from IN4
        
            Logic_T13(LIGHT1);                          // Execute the logic for IN 1
            Logic_T13(LIGHT2);                          // Execute the logic for IN 2
            Logic_T13(LIGHT3);                          // Execute the logic for IN 3
            Logic_T13(LIGHT4);                          // Execute the logic for IN 4

            DigOut(RELAY1, Souliss_T1n_ToggleCmd, OUT1);            // Drive the Relay 1
            DigOut(RELAY2, Souliss_T1n_ToggleCmd, OUT2);           // Drive the Relay 2
            DigOut(RELAY3, Souliss_T1n_ToggleCmd, OUT3);           // Drive the Relay 3
            DigOut(RELAY4, Souliss_T1n_ToggleCmd, OUT4);           // Drive the Relay 4

            Logic_SimpleLight(OUT1);
            Logic_SimpleLight(OUT2);
            Logic_SimpleLight(OUT3);
            Logic_SimpleLight(OUT4);    

              // Here we process all communication with other nodes
            FAST_GatewayComms();   
        } 
        
        FAST_91110ms() {
          } 
          
    EXECUTESLOW() { 
        UPDATESLOW();

        SLOW_10s() {  // Process the timer every 10 seconds  
          Serial.println("SLOW_10s");
            Timer_SimpleLight(OUT1);
            Timer_SimpleLight(OUT2);  
            Timer_SimpleLight(OUT3);
            Timer_SimpleLight(OUT4);
            Logic_T53(HUMIDITY);
            Logic_T52(TEMP0);
        }  
        SLOW_50s()  {
          float humidity = dht.readHumidity();
          float temperature = dht.readTemperature(false);
          Serial.println("SLOW_50s");
          Serial.println(humidity);
          if (!isnan(humidity) || !isnan(temperature)) {
            ImportAnalog(HUMIDITY, &humidity);
            ImportAnalog(TEMP0, &temperature);
           
            
            //ci ho anche aggiunto verifica ogni 90 sec (fast 91110) che la ESP sia collegata alla rete Wifi (5 tentativi al 6^fa hard reset):
          //WL_NO_SHIELD = 255,
          //WL_IDLE_STATUS = 0,
          //WL_NO_SSID_AVAIL = 1
          //WL_SCAN_COMPLETED = 2
          //WL_CONNECTED = 3
          //WL_CONNECT_FAILED = 4
          //WL_CONNECTION_LOST = 5
          //WL_DISCONNECTED = 6
                       
          int tent=0;
          Serial.println("Verifico connessione WIFI");
          Serial.print("Connecting to ");
          Serial.println(WiFi_SSID);
          Serial.println("Status: ");
          Serial.println(WiFi.status());
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());  
          Serial.println("");  
          while ((WiFi.status() != 3) && tent<9)
              {
                Serial.println("WIFI non connessa");
                WiFi.disconnect();
                WiFi.mode(WIFI_STA);
                WiFi.begin(WiFi_SSID , WiFi_Password);
                int ritardo =0;
                while ((WiFi.status() != 3) && ritardo<20)
                    {
                      delay(5000);  
                      ritardo +=1;
                      Serial.println(ritardo);
                      }
                if (WiFi.status() != 3 ) 
                delay(2000); 
                tent +=1;
              }
        if (tent>8)Serial.println("Tentativo di riconnessione non riuscito");
        if (tent=0)Serial.println("WIFI connessa");
        }   
     } 
          }      
     }
   }  
