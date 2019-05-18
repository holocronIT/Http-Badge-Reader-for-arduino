/* 
Con questo sketch è possibile fare attivare un relè 
al passaggio di una tessera magnetica sul lettore RFID usando un server online per la validazione del badge.
Lo script, tramite lo shield ethernet, effettuerà una richiesta al server http://[your-server]/track?card=[ID-card] per ogni badge letto.
Il server dovrà rispondere "RESULT:3" dove il numero indica il numero di accessi rimanenti. Se sarà magggiore di zero verrà abilitato il relè

Autore Pasqui Andrea

PINOUT:
  
RC522 MODULO    Uno/Nano    
SDA             D10
SCK             D13
MOSI            D11
MISO            D12
IRQ             N/A
GND             GND
RST             D9
3.3V            3.3V

Relè al Pin 5
*/
  
#include <SPI.h>
#include <Ethernet.h>
#include <RFID.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


/* Vengono definiti PIN del RFID reader*/
#define SDA_DIO 7  // Pin 53 per Arduino Mega
#define RESET_DIO 9
#define delayRead 1000 // Tempo 
#define delayLed 2000 
#define ledVerde 3
#define ledRosso 3
#define rele 5

#define Eth_cs  10
#define SD_cs  4



/* Viene creata una istanza della RFID libreria */
RFID RC522(SDA_DIO, RESET_DIO); 
  // inserire tutti i codici esadecimali delle schede magnetiche riconosciute 

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
  


int loopStatus = 0;



// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(91,134,164,10);  // numeric IP for Google (no DNS)
IPAddress server(10,0,0,2);  // numeric IP for Google (no DNS)
//char server[] = "coffee.holocron.it";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(10, 0, 0, 101);
IPAddress myDns(10, 0, 0, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement




  
void setup()
{ 

  lcd.init();  

  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Starting...");
  
  pinMode(Eth_cs,OUTPUT);
  pinMode(SD_cs,OUTPUT);
  pinMode(rele,OUTPUT);
  
  //Serial.begin(9600);

  digitalWrite( SDA_DIO, HIGH );
  digitalWrite( Eth_cs, HIGH );
  digitalWrite( SD_cs, HIGH );
  digitalWrite( rele , HIGH );
  
  SPI.begin(); 

  //Serial.println("Setup RFID");
  
  enableRFID();
  
  RC522.init();
  
  //Serial.println("Setup Ethenet");
  
  enableETH();

  Ethernet.init(10); 


  lcd.setCursor(0,1);
  if (Ethernet.begin(mac) == 0) {
    
    //Serial.println("Failed to configure Ethernet using DHCP");
    lcd.print("Error DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      lcd.print("No Ethernet");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      lcd.print("No Ethernet Cable");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    lcd.print("DHCP:");
    lcd.print(Ethernet.localIP());
  }

  // give the Ethernet shield a second to initialize:
  delay(5000);
  lcd.clear();

  enableRFID();

  loopStatus = 1;

  lcd.setCursor(0,0);
  lcd.print("Want a coffee?");
}
  
void loop()
{
  /* Temporary loop counter */


  byte i;
  
  // Se viene letta una tessera
  if (RC522.isCard() && loopStatus == 1)
  {
    // Viene letto il suo codice 
    RC522.readCardSerial();
    String codiceLetto ="";
    
    lcd.clear();
    lcd.print("Badge: ");  
    // Viene caricato il codice della tessera, all'interno di una Stringa
    for(i = 0; i <= 4; i++)
    {
      codiceLetto+= String (RC522.serNum[i],HEX);
      codiceLetto.toUpperCase();
    }
    lcd.print(codiceLetto);
   
    enableETH();
    
   if (client.connect(server, 80)) {
      //Serial.print("connected to ");
      //Serial.println(client.remoteIP());
      // Make a HTTP request:
      client.print("GET /track?card=");
      client.print(codiceLetto);
      client.println(" HTTP/1.1");
      client.println("Host: your.server.com");
      client.println("Connection: close");
      client.println();
      loopStatus = 2;
    } else {
      // if you didn't get a connection to the server:
       lcd.print("Err Server");
    }
    
  delay(delayRead);  
  }

  if( loopStatus == 2 ){

    int len = client.available();
    if (len > 0) {
      
        byte responseBuffer[80];
        if (len > 80) len = 80;
        client.read(responseBuffer, len);
        String bufferString;
       
        for (int k=0; k <= len; k++){ bufferString += (char)responseBuffer[k]; }

        int posInStr = bufferString.indexOf("RESULT:");
        if( posInStr != -1 ){
            String avalableCoffee = bufferString.substring( (posInStr+7) , (posInStr+9) );
            processResult( avalableCoffee.toInt() );
        }
    }

    if (!client.connected()) {
        loopStatus = 1;
        //Serial.println("disconnected");
    }
  }

}



// Questa funzione verifica se il codice Letto è autorizzato
boolean verificaCodice(String codiceLetto, String codiceAutorizzato){
  if(codiceLetto.equals(codiceAutorizzato)){
    return true;
  }else{
    return false;
  }  
}    



// Questa funzione permette di accendere un LED per un determinato periodo
void accendiLed(int ledPin){
  digitalWrite(ledPin,HIGH);
  delay(delayLed);
  digitalWrite(ledPin,LOW);
}


void enableETH(){

  digitalWrite( SDA_DIO, HIGH );
  digitalWrite( Eth_cs, LOW );
  digitalWrite( SD_cs, HIGH );
  
}


void enableRFID(){

  digitalWrite( SDA_DIO, LOW );
  digitalWrite( Eth_cs, HIGH );
  digitalWrite( SD_cs, HIGH );
  
}


void processResult( int coffee ){
  
    lcd.setCursor(0,1);
    if( coffee > 0 ){
      lcd.print("Ready to go!");
      lcd.setCursor(0,2);
      lcd.print(coffee);
      lcd.print(" access left!");
      digitalWrite( rele , LOW );
      delay(5000);
      digitalWrite( rele , HIGH );
    }else{
       lcd.print("No more access for toady :(");
    }

    delay(2000);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Access?");
}

