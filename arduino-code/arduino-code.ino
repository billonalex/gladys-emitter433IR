#include <ArduinoJson.h>
#include <RCSwitch.h>
#include <IRremote.h>

//const int DATA_EMIT_PIN = 6;                                                  // Pin DATA de l'émetteur 433 Mhz
const int DATA_EMIT_PIN = 50;                                                  // Pin DATA de l'émetteur 433 Mhz
const int VCC_EMIT_PIN = 48;
const int GND_EMIT_PIN = 46;

//const int DATA_PIN = 8;                                                       // Pin DATA du récepteur
const int DATA_PIN = 3;                                                       // Pin DATA du récepteur
const int VCC_PIN = 2;
const int GND_PIN = 5;

const int DATA_PIN_LED = 10;                                                  // Pin DATA de l'émetteur IR du ruban LED
const int DATA_PIN_TV = 9;                                                    // Pin DATA de l'émetteur IR de la télévision
//const int DATA_RECV_PIN_IR_LED = 11;                                          // Pin DATA du récepteur IR du ruban LED
//const int DATA_RECV_PIN_IR_TV = 39;                                           // Pin DATA du récepteur IR de la TV
const unsigned int THIGH = 220, TSHORT = 350, TLONG=1400;                     // Temps des états (nécessaire à l'envoi de signaux Chacon)

//IRrecv irrecv_led(DATA_RECV_PIN_IR_LED);
//IRrecv irrecv_tv(DATA_RECV_PIN_IR_TV);
IRsend ir_send;                                                               // Crée une instance pour controler les led (pin 3 par défaut)
RCSwitch mySwitch = RCSwitch();                                               // Crée une instance pour la réception 433 MHz (pin 2)

decode_results results;                                                       // Variable contenant le résultat des réceptions IR

void sendIR_LED(unsigned long code){                                          // Fonction à appeler pour envoyer un code IR au ruban LED
  ir_send.changePin(DATA_PIN_LED);
  ir_send.sendNEC(code, 32); // code télécommande et nombre de bits
}

void sendIR_TELE(unsigned long code){                                         // Fonction à appeler pour envoyer un code IR à la télévision
  ir_send.changePin(DATA_PIN_TV);
  ir_send.sendNEC(code, 32); // code télécommande et nombre de bits
}

void SendRadioCode(long code) {                                               // Fonction à appeler pour envoyer un code en 433 MHz
   mySwitch.send(code, 24);
}

void sendChaconCode(unsigned long code){                                      // Fonction à appeler pour envoyer un code Chacon
  for(int i = 0; i<5; i++){                                                   // Emission du code 5 fois
    emit(code);
  }
}

/*

void detectIR_LED(){                                                          // Fonction à appeler dans void loop() pour permettre la détection de signaux IR du ruban LED
  if (irrecv_led.decode(&results)) {
    Serial.print("{\"action\":\"received\",\"value\":");
    Serial.print(results.value, HEX);
    Serial.println("}");
    irrecv_led.resume(); // Receive the next value
  }
  delay(100);
}


void detectChacon(){                                                           // Fonction à appeler dans void loop() pour permettre la détection de signaux Chacon
  unsigned long sender = listenSignalDIO();

  if(sender != 0){
    Serial.print("{\"action\":\"received\",\"value\":");
    Serial.print(sender);                 
    Serial.println("}");
    delay(200);
  }
}

void detectRadio(){                                                             // Fonction à appeler dans void loop() pour permettre la détection de signaux Radio
  if (mySwitch.available()) {
    int value = mySwitch.getReceivedValue();
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("{\"action\":\"received\",\"value\":");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.println("}");
    }
    delay(200);
    mySwitch.resetAvailable();
  }
}
*/

void emit(unsigned long code){                                                 // Fonction permettant d'envoyer un signal radio Chacon
  digitalWrite(DATA_EMIT_PIN, HIGH);
  delayMicroseconds(THIGH);
  digitalWrite(DATA_EMIT_PIN, LOW); 
  delayMicroseconds(2675);
  for (int i = 0; i < 32; i++) {
    if (code & 0x80000000L) {
      digitalWrite(DATA_EMIT_PIN, HIGH);
      delayMicroseconds(THIGH);
      digitalWrite(DATA_EMIT_PIN, LOW);
      delayMicroseconds(TLONG);
      digitalWrite(DATA_EMIT_PIN, HIGH);
      delayMicroseconds(THIGH);
      digitalWrite(DATA_EMIT_PIN, LOW);
      delayMicroseconds(TSHORT);
    } else {
      digitalWrite(DATA_EMIT_PIN, HIGH);
      delayMicroseconds(THIGH);
      digitalWrite(DATA_EMIT_PIN, LOW);
      delayMicroseconds(TSHORT);
      digitalWrite(DATA_EMIT_PIN, HIGH);
      delayMicroseconds(THIGH);
      digitalWrite(DATA_EMIT_PIN, LOW);
      delayMicroseconds(TLONG);
    }
    code <<= 1;
  }
  digitalWrite(DATA_EMIT_PIN, HIGH);
  delayMicroseconds(THIGH);
  digitalWrite(DATA_EMIT_PIN, LOW);
  delayMicroseconds(10600);
  digitalWrite(DATA_EMIT_PIN, HIGH);
  delayMicroseconds(THIGH);

  //Remise à 0 pour ne pas interférer avec les télécommandes
  digitalWrite(DATA_EMIT_PIN, LOW);
}

// Serial buffer
String command = "";

// End of command marker
char endMarker = '%';

/*
 * Execute the right function
 */

void executeFunction(String json_data) {
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& v = jsonBuffer.parseObject(json_data);
  //on décompose la chaine de cartère
  if ( v["function_name"] == String("SendRadioCode") ) {
    SendRadioCode(v["code"]);
  } else if ( v["function_name"] == String("sendIR_LED") ) {
    sendIR_LED(v["code"]);
  } else if ( v["function_name"] == String("sendIR_TELE") ) {
    sendIR_TELE(v["code"]);
  } else if ( v["function_name"] == String("sendChaconCode") ) {
    sendChaconCode(v["code"]);
  } 
}

/**
 * This function is automatically called when data is received on serial port
 */
void serialEvent() {
    //lit toutes les données (vide le buffer de réception)
    char last_readed_char = Serial.read();
    if ( last_readed_char == endMarker ) {
      executeFunction(command);
      command = "";
    } else {
      command += last_readed_char;
    }
}

/*
* This function is used to detect 433 MHz signals from Chacon outlets
*/

unsigned long listenSignalDIO(){   
        int i = 0;                          //Variable du compteur boucle
        unsigned long t = 0;                //Variable de temps entre 2 front descendant
        byte prevBit = 0;                   //avant dernière "bit" 433 reçue
        byte bit = 0;                       //dernière "bit" 433 reçue
        unsigned long sender = 0;           //code DIO au format unsigned long - ne gère pas le bit on-off, donne 2 codes différents
        t = pulseIn(DATA_PIN, LOW, 1000000);//réception des bits     
        if(t > 2550 && t < 2800) {           //on lance le déchiffrage quand on reçoit un bit correspondant au deuxieme cadenas du signal DIO
          while(i < 64)
              {       
              t = pulseIn(DATA_PIN, LOW, 1000000);  //reception des bits        
              if(t > 280 && t < 340)                //si environ 310 + ou - 30 c'est un bit à 0
                  {   bit = 0;              
                  }
                  else if(t > 1300 && t < 1400) //si environ 1350 + ou moins 50 c'est un bit à 1
                        {   bit = 1;                           
                        }
                        else    //sinon la série est érronnée, on sort de la fonction
                        break;
             if(i % 2 == 1) // on ne traite que les deuxièmes bits (codage manchester)
                  {
                  if((prevBit ^ bit) == 0) //si faux = codage manchester KO, on sort
                       break;
                  sender <<= 1;       //on stock chaque bit dans l'unsigned long
                  sender |= prevBit;
                  }
             prevBit = bit;  //on enregistre le bit précédent
              ++i;
            }
        }  
        if(i == 64){     // si on a 64 bit (i=64) on a un code DIO détecté
         return sender; 
        }       
     return 0; //sinon pas de code DIO
}

void setup() {
  pinMode(GND_EMIT_PIN, OUTPUT);
  pinMode(GND_PIN, OUTPUT);
  digitalWrite(GND_EMIT_PIN, LOW);
  digitalWrite(GND_PIN, LOW);

  pinMode(VCC_EMIT_PIN, OUTPUT);
  pinMode(VCC_PIN, OUTPUT);
  digitalWrite(VCC_EMIT_PIN, HIGH);
  digitalWrite(VCC_PIN, HIGH);

  pinMode(DATA_EMIT_PIN, OUTPUT);
  
  // Open serial communications and wait for port to open: 
  Serial.begin(9600);
  
  /*// Receiver is connected on Arduino Pin #2
  mySwitch.enableReceive(0);
  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(10);
  // Optional set pulse length.
  mySwitch.setPulseLength(310);
  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(1);
  // Optional set number of transmission repetitions.
  mySwitch.setRepeatTransmit(15);*/
  
  // Start the IR receivers
  //irrecv_led.enableIRIn();
  //irrecv_tv.enableIRIn();
   
  delay(1000);
}

void loop() {       // Fonction loop() simplifiée par les appels des fonctions respectives
  //detectChacon();
  //detectRadio();
  //detectIR_LED();
  
  unsigned long sender = listenSignalDIO();

  if(sender != 0){
    Serial.print("{\"action\":\"received\",\"value\":");
    Serial.print(sender);                 
    Serial.println("}");
    delay(200);
  }

  if (mySwitch.available()) {
    int value = mySwitch.getReceivedValue();
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("{\"action\":\"received\",\"value\":");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.println("}");
    }
    delay(200);
    mySwitch.resetAvailable();
  }

/*                                                             // Fonction à appeler dans void loop() pour permettre la détection de signaux IR de la TV
  if (irrecv_tv.decode(&results)) {
    Serial.print("{\"action\":\"received\",\"value\":");
    Serial.print(results.value, HEX);
    Serial.println("}");
    irrecv_tv.resume(); // Receive the next value
    delay(100);
  }*/
}
