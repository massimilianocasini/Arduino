
/**************************************************************************
    Souliss - 
***************************************************************************/
//Dice a IDE di puntare al framework
#include <SoulissFramework.h>

// Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "MisiSpot"
#define WiFi_Password           "multiactivity"    

/*#include "bconf/MCU_ESP8266.h"   // Define the board type
#include "conf/DynamicAddressing.h"         // Use dynamic address  
*/
// Include framework code and libraries
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <DHT.h>

/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"

// By default the board will get an IP address with .77 as last byte, you can change it
// in runtime using the Android application SoulissApp

#define DHTTYPE DHT11      // Tipo di sensore DHT 11
#define DHTPIN  0         // PIN 0 ESP
DHT dht(DHTPIN,DHTTYPE,11); // for ESP8266 use dht(DHTPIN, DHTTYPE, 11)

#define HUMIDITY                0           // This is the memory slot used for the execution of the logic
#define TEMP0                   2         
#define LIGHT1                  4       
#define OUT1                    5           

#define OUTPUTPIN 1
#define INPUTPIN 2
void setup()
{   
    // Init the board
    Initialize();
    
    // This board (peer) request an address to the gateway one at runtime, no need
    // to configure any parameter here
    /*SetDynamicAddressing();
    GetAddress();*/
    // Connect to the WiFi network and get an address from DHCP
    GetIPAddress();  
    
    // This is the vNet address for this node, used to communicate with other
  // nodes in your Souliss network
    SetAddress(0xAB02, 0xFF00, 0xAB01);
       
    dht.begin(); 
    
    // Define Simple Light logics for the relays
    Set_T53(HUMIDITY);
    Set_T52(TEMP0);
    Set_T13(LIGHT1);
    Set_T11(OUT1); 
    pinMode(OUTPUTPIN, OUTPUT);  
    pinMode(INPUTPIN, INPUT);   
}

void loop()
{ 
    // Here we start to play
    EXECUTEFAST() {                     
        UPDATEFAST();   
        
        FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
        
            DigIn2State(INPUTPIN, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, LIGHT1);          // Read inputs from IN1
            Logic_T13(LIGHT1);                                                     // Execute the logic for Relay 1
            
            DigOut(OUTPUTPIN, Souliss_T1n_ToggleCmd, OUT1);                             // Drive the Relay 
            Logic_T11(OUT1);
        } 
            
        // Here we process all communication with other nodes
        FAST_PeerComms();
        
        // At first runs, we look for a gateway to join
        //START_PeerJoin();
    }
    
    EXECUTESLOW() { 
        UPDATESLOW();

        SLOW_10s() {                                              // Process the timer every 10 seconds  
          //  Timer_T13(IN1);
            Timer_T11(OUT1);
            Logic_T53(HUMIDITY);
            Logic_T52(TEMP0); 
        }     
        SLOW_50s()  {
          float humidity = dht.readHumidity();
          float temperature = dht.readTemperature(false);
          if (!isnan(humidity) || !isnan(temperature)) {
            ImportAnalog(HUMIDITY, &humidity);
            ImportAnalog(TEMP0, &temperature);
          }      
     }
        // Here we periodically check for a gateway to join
       // SLOW_PeerJoin();        
    }
} 
