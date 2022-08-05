/* Der Aufruf des RFID-Chips wird mit Code aus den Beispielprojekten der RFID-Bibliothek realisiert
   Da der Code nicht mehr zusammenhängend ist, ist die Quelle hiermit am Anfang aufgeführt.
   Erkennbar sind die Teile noch durch die Englischen Code-Kommentare.
   https://github.com/miguelbalboa/rfid/blob/master/examples/ReadAndWrite/ReadAndWrite.ino
*/

#include <SPI.h>  //Serial Peripheral Interface (SPI) Bibiothek
#include <MFRC522.h> // RFID-Bibiothek hinzufügen
#include <Servo.h> 
#include <Wire.h> // Wire Bibliothek einbinden
#include <LiquidCrystal_I2C.h> // Vorher hinzugefügte LiquidCrystal_I2C Bibliothek einbinden
LiquidCrystal_I2C lcd(0x27, 16, 2); //Hier wird festgelegt um was für einen Display es sich handelt. In diesem Fall eines mit 16 Zeichen in 2 Zeilen und der HEX-Adresse 0x27. Für ein vierzeiliges I2C-LCD verwendet man den Code "LiquidCrystal_I2C lcd(0x27, 20, 4)"

Servo servo;

int taster1 = 7;
int taster2 = 8;
int sensor1 = A0;
int sensor2 = A1;
int schacht1 = 1; // 1 heißt befüllt; 0 heißt leer
int schacht2 = 1; // 1 heißt befüllt; 0 heißt leer
int led1 = 3;
int led2 = 4;
unsigned long wartezeit; // um millis() richtig zu behandeln

#define RST_PIN         9
#define SS_PIN          10

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

/**
   Initialize.
*/
void setup() {
  lcd.init(); //Im Setup wird der LCD gestartet
  lcd.backlight(); //Hintergrundbeleuchtung einschalten (lcd.noBacklight(); schaltet die Beleuchtung aus).
  lcd.clear(); // löscht den INhalt des LCDs

  pinMode(taster1, INPUT_PULLUP);
  pinMode(taster2, INPUT_PULLUP);
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  servo.attach(2); // nutzt die Bibliothek servo um den Servo zu Steuern. Legt den Pin auf 2 fest.
  servo.write(40); // stellt die Ursprungpositon des Servos auf 40
  delay(1000);

  Serial.begin(9600); // Initialize serial communications with the PC
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  // Prepare the key (used key A)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
  Serial.print(F("Using key (for A):"));
  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();

  Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));
}

/**
   Main loop.
*/
void loop() {

  //Leds um das vorhanden sein eines Gegenstands zwischen Led und Abstandssensor zu bestimmen.
  //Werden zur Messung einmal angeschaltet.
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  delay(100);

  int sensor1Ausgabe = analogRead(sensor1);
  Serial.println(sensor1Ausgabe);

  int sensor2Ausgabe = analogRead(sensor2);
  Serial.println(sensor2Ausgabe);


  // überprüft ob im Schacht was drin ist. Also zwischen Led und Sensor etwas liegt.
  if ( sensor1Ausgabe > 500)
    schacht1 = 0;
  if ( sensor2Ausgabe > 500)
    schacht2 = 0;


  //digitalWrite(led1, LOW);
  //digitalWrite(led2, LOW);


  //TODO: Möglickeit zum zurücksetzen des Automaten einbauen
  while (schacht2 == 0 && schacht1 == 0) { // falls gar keine snacks vorhanden sind bleibt das programm im Loop und muss neu gestartet werden
    
    
   lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
    lcd.print("Keine Ausgabe");

    lcd.setCursor(0, 1);// In diesem Fall bedeutet (0,1) das erste Zeichen in der zweiten Zeile.
    lcd.print("moeglich");

    sensor1Ausgabe = analogRead(sensor1);
    sensor2Ausgabe = analogRead(sensor2);
    Serial.println(sensor1Ausgabe);
    Serial.println(sensor2Ausgabe);
  }


  lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
  lcd.print("Verifizieren sie");

  lcd.setCursor(0, 1);// In diesem Fall bedeutet (0,1) das erste Zeichen in der zweiten Zeile.
  lcd.print("sich Bitte!");


  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;



  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Check for compatibility
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("This sample only works with MIFARE Classic cards."));
    return;
  }

  String uid = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    uid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  uid.toUpperCase();
  //überprüft die UID der Karte. Der Zugang wird gewährt, wenn eine der unten Aufgeführten UIDs mit der Karte übereinstimmt.
  if ((uid.substring(1) == "23 2F 1E 19") || (uid.substring(1) == "CC CC A3 16") ) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Zugang gewaehrt");
    lcd.clear();
    lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
    lcd.print("Zugang gewaehrt");

    Serial.println();
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    //digitalWrite(Green, HIGH);
    //delay(10000);
    //digitalWrite(Green, LOW);


    programmablauf();


    //TODO: Naechstes davon abhängig machen ob die Ausgabe geklappt hat.
    lcd.clear();
    lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
    lcd.print("Sie koennen");
    lcd.setCursor(0, 1);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
    lcd.print("Karte entfernen");
    delay(3000);
    lcd.clear();


  }

  else   {

    lcd.clear();
    lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
    lcd.print("Keine");
    lcd.setCursor(0, 1);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
    lcd.print("Berechtigung");
    Serial.println(" Keine Berechtigung");
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    //digitalWrite(RED, HIGH);
    delay(10000);
    //digitalWrite(RED, LOW);
    return;
  }
}

//kümmert sich um die Ausgabe der rechten oder linken Kammer. Also die Steuerung des Servos.
void motorsteuerung(int auswahl) {
  if (auswahl == 1) {
    servo.write(15);
    delay(60);
    servo.write(40);
    delay(1000);
  } else if (auswahl == 2) {
    servo.write(63);
    delay(70);
    servo.write(40);
    delay(1000);
  }
  delay(1000);
}

/**
  Die Funktion legt fest was beim Trücken der Taster passiert, bzw. was parallel auf dem LCD Display ausgeben wird.
  Auch mögliche Fehler werden hier behandelt.
*/
void programmablauf() {
  int tasterGedrueckt = 0;
  tasterGedrueckt = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bitte Auswahl ");
  lcd.setCursor(0, 1);
  lcd.print("treffen!");


  Serial.println(digitalRead(taster1));
  Serial.print("+");
  Serial.print(digitalRead(taster2));
  Serial.print(F("Wartezeit:"));
  wartezeit = millis();
  Serial.println(wartezeit);
  Serial.println(millis());

  while ((digitalRead(taster1) == 1 && digitalRead(taster2) == 1) && tasterGedrueckt == 0) {
    //Serial.println(tasterGedrueckt);
    //Serial.println("Schleife");
    delay(5); //braucht irgendwas in in der Schleife. Sonst springt er aus unerfindlichen Günden hin und wieder raus oder halt auch nicht.
    //Möglicherweise des Prellen???
    if (digitalRead(taster1) != 1 && schacht1 == 1) {// überprüft ob schacht gefüllt ist.

      tasterGedrueckt = 1;
      Serial.println("Taster 1 gedrückt");
      lcd.clear(); // lcd löschen bevor Ausgabe startet. Sonst macht ANzeige keinen Sinn.



      if (rfidBezahlen(2) == true) {
        Serial.println("rfid True");
        lcd.clear();
        lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
        lcd.print("Ausgabe...");
        delay(400); // da das LCD Display nicht schnellgenug reagiert.
        motorsteuerung(tasterGedrueckt);
      } else{
        tasterGedrueckt = 3;
        Serial.println("Taster 3 gedrückt");
      }
    } else if (digitalRead(taster2) != 1 && schacht2 == 1) { // überprüft ob schacht gefüllt ist.
      Serial.println("Taster 2 gedrückt");
      tasterGedrueckt = 2;
      lcd.clear();

      if (rfidBezahlen(3) == true) {
        Serial.println("rfid True");
        lcd.clear();
        lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
        lcd.print("Ausgabe...");
        delay(400); // da das LCD Display nicht schnellgenug reagiert.
        motorsteuerung(tasterGedrueckt);
      } else{
        tasterGedrueckt = 3;
      Serial.println("Taster 3 gedrückt");
      }

    } else if ( (digitalRead(taster2) != 1 && schacht2 == 0) || (digitalRead(taster1) != 1 && schacht1 == 0)) { // falls ein Fach ausgewählt wurde in dem nix drin ist.

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ausgabe nicht");
      lcd.setCursor(0, 1);
      lcd.print("verfuegbar!");
      delay(1000);

    } else if ((millis() - wartezeit) > 20000) {
      tasterGedrueckt = 3; // für Error Meldung.
      Serial.println("Error! Zu lange gewartet!");
      Serial.println(millis() - wartezeit);
      delay(2000);


      Serial.print(F("Wartezeit:"));
      Serial.println(wartezeit);
      Serial.println(millis());
    }
  }

  if (tasterGedrueckt == 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error! Programm");
    lcd.setCursor(0, 1);
    lcd.print("neustart!");
    delay(2000);

  }

  Serial.println(tasterGedrueckt);
  Serial.println("Ende");

  if (tasterGedrueckt == 1 || tasterGedrueckt == 2) {
    Serial.println("Danke fuer Ihren Einkauf!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Danke fuer Ihren");
    lcd.setCursor(0, 1);
    lcd.print("Einkauf!");
    delay(1000);
  }

  tasterGedrueckt = 0;

}

/**
  entfernt einen gewissen Betrag von einer RFID-Karte. Der Betrag ist als int in Sektor 1 Block 4 gespeichert.
  Falls es einen Fehler in der Verarbeitung gibt, gibt die Methode den Boolean Wert false zurück.
  Ansonsten "true".
*/
bool rfidBezahlen(int zahlBetrag) {

  wartezeit = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Zahlungsmittel");
  lcd.setCursor(0, 1);
  lcd.print("davor halten");
  delay(1000);

  int warten = 1;
  bool fehlerstatus = false; //true wenn alles problemlos geklappt hat


/*
 * Um auch beim Absetzten der Karte und wieder aufglegen eine korrekte Abbuchung mit Ausgabe zu gewähleisten.
 */
  while (((millis() - wartezeit) < 20000) && warten == 1) {

    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
        continue;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        continue;

    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
            &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
            &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("This sample only works with MIFARE Classic cards."));
      return;
    }

    String uid = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      uid.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message : ");
    uid.toUpperCase();
    //überprüft die UID der Karte. Der Zugang wird gewährt, wenn eine der unten Aufgeführten UIDs mit der Karte übereinstimmt.
    if ((uid.substring(1) == "23 2F 1E 19") || (uid.substring(1) == "CC CC A3 16") ) //change here the UID of the card/cards that you want to give access
    {
      // In this sample we use the second sector,
      // that is: sector #1, covering block #4 up to and including block #7
      byte sector         = 1;
      byte blockAddr      = 4;
      byte dataBlock[]    = {
        0x00, 0x00, 0x00, 0x00, //  255,  0,   0,  0,
        0x00, 0x00, 0x00, 0x00, //  0,  0,   0,  0,
        0x00, 0x00, 0x00, 0x00, //  0, 0, 0, 0,
        0x00, 0x00, 0x00, 0x00  // 0, 0, 0, 0
      };
      byte trailerBlock   = 7;
      MFRC522::StatusCode status;
      byte buffer[18];
      byte size = sizeof(buffer);
      int guthaben = 0;

      


      // Authenticate using key A
      Serial.println(F("Authenticating using key A..."));
      status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        fehlerstatus = false;
        //return;
        continue;
      }

      // Show the whole sector as it currently is
      Serial.println(F("Current data in sector:"));
      mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
      Serial.println();

      // Read data from the block
      Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
      Serial.println(F(" ..."));
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        fehlerstatus = false;
        continue;
      }
      Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
      dump_byte_array(buffer, 16); Serial.println();
      Serial.println();


      //legt die aktuellen Daten der Karte im dateBlock Array ab.
      for ( int i = 0; i < 16; i++) {
        dataBlock[i] = buffer[i];
      }

      Serial.print(F("Blockdaten "));
      dump_byte_array(dataBlock, 16); Serial.println();


      int i = 0;
      //zieht den zahlBetrag ab. Geht dabei jeden Byteblock durch, ob dort noch eine Zahl gespeichert ist.
      while (zahlBetrag > 0  && i < 16)
      {
        Serial.print(F("Block 1 "));
        dump_byte_array(dataBlock, 16); Serial.println();

        dataBlock[i] = dataBlock[i] - 1;
        zahlBetrag = zahlBetrag - 1;
        Serial.println(dataBlock[i]);
        // wenn der Byteblock null ist, zähler auf den nächsten
        if (!(dataBlock[i] > 0)) {
          i++;
        }
      }

      //falls am Ende immer noch nicht genügend entfernt werden konnte, wird eine Fehlermeldung ausgegeben.
      //TODO: Es sollte der Abgehobene Betrag auch wieder zurück auf die Karte gezahlt werden!
      if (zahlBetrag > 0 && i > 15) {
        Serial.print(F("Nicht genügend Betrag auf der Karte! "));
        fehlerstatus = false;
      }


      // Write data to the block
      Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
      Serial.println(F(" ..."));
      dump_byte_array(dataBlock, 16); Serial.println();
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        fehlerstatus = false;
        continue;
      }
      Serial.println();

      // Read data from the block (again, should now be what we have written)
      Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
      Serial.println(F(" ..."));
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        fehlerstatus = false;
        continue;
      }
      Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
      dump_byte_array(buffer, 16); Serial.println();

      // Check that data in block is what we have written
      // by counting the number of bytes that are equal
      Serial.println(F("Checking result..."));
      byte count = 0;
      for (byte i = 0; i < 16; i++) {
        // Compare buffer (= what we've read) with dataBlock (= what we've written)
        guthaben = guthaben + buffer[i];
        if (buffer[i] == dataBlock[i])
          count++;
      }

     


      Serial.print(F("Number of bytes that match = ")); Serial.println(count);
      if (count == 16) {
        Serial.println(F("Success :-)"));
        fehlerstatus = true;
      } else {
        Serial.println(F("Failure, no match :-("));
        Serial.println(F("  perhaps the write didn't work properly..."));
        fehlerstatus = false;
        continue;
      }
      Serial.println();

      // Dump the sector data
      Serial.println(F("Current data in sector:"));
      mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
      Serial.println();

      // Halt PICC
      mfrc522.PICC_HaltA();
      // Stop encryption on PCD
      mfrc522.PCD_StopCrypto1();

     if(fehlerstatus == true){
      warten= 0;
      //TODO: ??? zeigt manchmal komische Zahlen an. Beim nächsten Durchlauf ist die Zahl dann wieder Richtig. ??? Liegt vielleicht an der Umwanldung von HEx zu Int --> Bisher des Problem nicht ausmachen können...
      lcd.clear();
      lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
      lcd.print("Ihr Guhaben:");
      lcd.setCursor(0, 1);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile.
      lcd.print(guthaben);
      delay(2000);

     }
     
  
    } 

  }
  Serial.println(F("Warten "));
   Serial.println(warten);
  return fehlerstatus;
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }

}
