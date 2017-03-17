

/**************************************************************************
    Souliss - Controlla 4 ingressi e 4 uscite indipendenti, 1 sensore di temperatura/umidità

   

    Connect multiple ProDINo Wroom via RS485 using the e08_WiFi_Lights_n2.ino
    on the peer boards.
        
   
    
    Runs on KMP Electronics ProDINo Wroom (WiFi ESP8266)

***************************************************************************/

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

#define DHTTYPE DHT11   // DHT 11
#define DHTPIN  5         // what digital pin we're connected to

// DHT sensor
DHT dht(DHTPIN, DHTTYPE, 11); // for ESP8266 use dht(DHTPIN, DHTTYPE, 11)

#define LIGHT1                  0           // This is the memory slot used for the execution of the logic
#define LIGHT2                  1           
#define LIGHT3                  2           
#define LIGHT4                  3
#define OUT1                    4           // This is the memory slot used for the execution of the logic
#define OUT2                    5           
#define OUT3                    6           
#define OUT4                    7
#define HUMIDITY                8               // Leave 2 slots for T53
#define TEMP0                   10              // Leave 2 slots for T52           



void setup()
{   
    // Init the board
    delay(180000); // Ritardo
    InitDINo();

    // Connect to the WiFi network and get an address from DHCP
    GetIPAddress();                           
    SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp  

    dht.begin();                                // initialize temperature sensor

    Set_Humidity(HUMIDITY);
    Set_Temperature(TEMP0);
    
    // Define Simple Light logics for the relays
    Set_DigitalInput(LIGHT1);
    Set_DigitalInput(LIGHT2);
    Set_DigitalInput(LIGHT3);
    Set_DigitalInput(LIGHT4);
    Set_SimpleLight(OUT1);
    Set_SimpleLight(OUT2);    
    Set_SimpleLight(OUT3);    
    Set_SimpleLight(OUT4);        
    
    // This node will act as addressing server for the other peers in the network
    SetAddressingServer();
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
        
            Logic_DigitalInput(LIGHT1);                          // Execute the logic for IN 1
            Logic_DigitalInput(LIGHT2);                          // Execute the logic for IN 2
            Logic_DigitalInput(LIGHT3);                          // Execute the logic for IN 3
            Logic_DigitalInput(LIGHT4);                          // Execute the logic for IN 4

            
            DigOut(RELAY1, Souliss_T1n_ToggleCmd, OUT1);            // Drive the Relay 1
            DigOut(RELAY2, Souliss_T1n_ToggleCmd, OUT2);           // Drive the Relay 2
            DigOut(RELAY3, Souliss_T1n_ToggleCmd, OUT3);           // Drive the Relay 3
            DigOut(RELAY4, Souliss_T1n_ToggleCmd, OUT4);           // Drive the Relay 4

            Logic_SimpleLight(OUT1);
            Logic_SimpleLight(OUT2);
            Logic_SimpleLight(OUT3);
            Logic_SimpleLight(OUT4);
            
            
        } 
        
        // Here we process all communication with other nodes
        FAST_GatewayComms();    
    }
    
    EXECUTESLOW() { 
        UPDATESLOW();

        SLOW_10s() {  // Process the timer every 10 seconds  
          
            Timer_SimpleLight(OUT1);
            Timer_SimpleLight(OUT2);  
            Timer_SimpleLight(OUT3);
            Timer_SimpleLight(OUT4);
            Logic_Humidity(HUMIDITY);
            Logic_Temperature(TEMP0);
        }  
     SLOW_50s()  {
          float humidity = dht.readHumidity();
          float temperature = dht.readTemperature(false);
          if (!isnan(humidity) || !isnan(temperature)) {
            ImportAnalog(HUMIDITY, &humidity);
            ImportAnalog(TEMP0, &temperature);
          }      
     }
   } 
    
} 
