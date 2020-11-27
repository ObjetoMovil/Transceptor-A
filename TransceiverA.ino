  #include "Movement.h"
  #include <SPI.h>
  #include <nRF24L01.h>
  #include <RF24.h>
  
  #define CE_PIN 48
  #define CSN_PIN 49
  
  typedef struct Movement Movement;
  
  const byte W_ADDRESS[6] = "00001";
  const byte R_ADDRESS[6] = "00002";
  const char DELIMITER = '|';
  Movement movement;
  
  RF24 transceiver(CE_PIN, CSN_PIN);
  
  void setup() {
    Serial.begin(9600);
    transceiver.begin();                
    transceiver.setChannel(115); 
    transceiver.setPALevel(RF24_PA_MAX);     //RF24_PA_MIN,/ RF24_PA_LOW,/ RF24_PA_HIGH,/ RF24_PA_MAX
    transceiver.setDataRate(RF24_250KBPS);   //RF24_1MBPS,/ RF24_2MBPS,/ RF24_250KBPS  
    transceiver.openWritingPipe(W_ADDRESS);
    transceiver.openReadingPipe(1, R_ADDRESS);
  }
  
  void loop() {
    //Verificar si se ha escrito algun mensaje en el monitor serial, de ser asi se establece el transceptor como emisor. 
    if (Serial.available()){
        setMovementValues();        
        transceiver.stopListening();
        transceiver.write(&movement, sizeof(movement));
    } 
  
    //Establecer el transceptor como receptor.     
    else{
      transceiver.startListening(); 
      if (transceiver.available()) {
        char data[50];
        transceiver.read(&data, sizeof(data));
        Serial.println(data);
      }
    }    
  }
  
  
  /**
   * Funcion que obtiene los datos provenientes del puerto serial y los almacena en un String. Realiza un Split
   * del String con el delimitador | y guarda las subcadenas obtenidas dentro un array del array splitedData. Por
   * ultimo establece los valores de la estructura movement.
   */
  void setMovementValues(){
    int splitedData[5];
    int splitedDataIndex = 0;
    String dataValue = "";
  
    //--------------------------- 1 ---------------------------//
    String data = Serial.readStringUntil('\n');
  
    //--------------------------- 2 ---------------------------//
    for(int i=0; i<data.length(); i++){
        //Realizar split.
        if(data.charAt(i) == DELIMITER){
          splitedData[splitedDataIndex] = dataValue.toInt();
          dataValue = "";
          splitedDataIndex++;
        //Seguir concatenando caracteres.  
        } else{
          dataValue += data.charAt(i); 
        }
    }
    
    //--------------------------- 3 ---------------------------//
    movement.type = 'N';
    movement.motor = 'N';
    movement.mTime = 0;
    movement.speed = 0;
    movement.angle = 0;
    movement.radio = 0;
    movement.orientation1 = 'N';
    movement.orientation2 = 'N';
  
    switch(splitedData[0]){
      
      //STRAIGHT
      case 0:
        movement.type = 'S';
        movement.mTime = splitedData[1];
        movement.speed = splitedData[2]+1;
        movement.orientation1 = getOrientation(splitedData[3]);
        movement.motor = 'N';
        movement.angle = 0;
        movement.radio = 0;
        movement.orientation2 = 'N'; 
      break;

      //CURVED
      case 1:
        movement.type = 'C';
        movement.radio = splitedData[1];
        movement.speed = splitedData[2]+1;
        movement.orientation1 = getOrientation(splitedData[3]);
        movement.orientation2 = getOrientation(splitedData[4]);
        movement.motor = 'N';
        movement.mTime = 0;
        movement.angle = 0;
      break;

      //DIAGONAL
      case 2:
        movement.type = 'D';
        movement.mTime = splitedData[1];
        movement.speed = splitedData[2]+1;
        movement.angle = splitedData[3];
        movement.radio = 0; 
        movement.motor = 'N';
        movement.orientation1 = 'N';
        movement.orientation2 = 'N';
      break;

      //BASE
      case 3:
        movement.type = 'B';
        movement.motor = getMotor(splitedData[1]);
        movement.mTime = splitedData[2];
        movement.speed = splitedData[3]+1;
        movement.orientation1 = getOrientation(splitedData[4]);
        movement.angle = 0;
        movement.radio = 0;
        movement.orientation2 = 'N'; 
      break;  
    }
  }

  /**
   * Funcion que devuelve el caracter correspondiente al motor cuyo valor
   * se recibe como parametro.
   */
  char getMotor(int motorValue){
    switch(motorValue){
      case 0: return 'A';
      case 1: return 'B';
      case 2: return 'C';
      case 3: return 'D';
    }
  }

  /**
   * Funcion que devuelve el caracter correspondiente a la orientacion cuyo valor
   * se recibe como parametro.
   */
  char getOrientation(int orientationValue){
    switch(orientationValue){
      case 0: return 'A';
      case 1: return 'B';
      case 2: return 'U';
      case 3: return 'D';
    }
  }

  
  //Rojo     VCC
  //Negro    GND
  //Verde    49   CSN  
  //Azul     48   CE
  //Blanco   50   MISO
  //Narnaja  51   MOSI
  //Amarillo 52   SCK
