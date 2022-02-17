#include <dht.h>                  // Incluindo biblioteca sensor DHT11 (Temperatura e Umidade).
#include <Wire.h>                 // Incluindo biblioteca da comunicação I2C.
#include <LiquidCrystal_I2C.h>    // Incluindo biblioteca do display LCD.

#define pinUmidSolo A0           // Pino do sensor de temperatura e umidade ligado no analógico 0
#define pinSensorTempUmid 2      // Pino do sensor de temperatura e umidade ligado no digital 2
#define pinNivel 5               // Pino do sensor de temperatura e umidade ligado no digital 5
#define pinAtuadorPeltier 3      // Pino do sensor de temperatura e umidade ligado no digital 3
#define pinAtuadorCooler 4       // Pino do sensor de temperatura e umidade ligado no digital 4
#define pinLed 7                 // Pino do sensor de temperatura e umidade ligado no digital 7
#define pinAtuadorExaustao 13    // Pino do sensor de temperatura e umidade ligado no digital 13

#define endereco 0x27    // Definindo o endereço do display no I2C
#define colunas 20       // Definindo o número de colunas do display
#define linhas 4         // Definindo o número de linhas do display

// Declarando variáveis
bool estadoReservatorio,
     estadoExaust,
     estadoCooler,
     estadoPeltier,
     estadoLed;
int temperatura = 0,
    umidadeAr = 0,
    umidadeSolo = 0,
    temperaturaAnt = 0,
    umidadeArAnt = 0,
    tempoLed = 1,
    tempoPeltier = 1;

// Instanciando objetos
dht objSensorTempUmid;
LiquidCrystal_I2C lcd(endereco, colunas, linhas);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.init();       // Inicia a comunicação com o display
  lcd.backlight();  // Liga a iluminação do display
  lcd.clear();      // Limpa o display

// Definindo pinos dos sensores como entrada
  pinMode(pinNivel, INPUT_PULLUP);
  pinMode(pinUmidSolo, INPUT);
  pinMode(pinSensorTempUmid, INPUT);

// Definindo pinos dos atuadores como saída
  pinMode(pinAtuadorPeltier, OUTPUT);
  pinMode(pinAtuadorCooler, OUTPUT);
  pinMode(pinAtuadorExaustao, OUTPUT);
  pinMode(pinLed, OUTPUT);

// Mostrando na tela os textos que não mudam
  lcd.print("TEMP:");
  lcd.setCursor(0, 1);
  lcd.print("U.AR:");
  lcd.setCursor(0, 2);
  lcd.print("LUZ:");
  lcd.setCursor(0, 3);
  lcd.print("TANQUE:");
  lcd.setCursor(10, 0);
  lcd.print("U.SOLO:");
  lcd.setCursor(10, 1);
  lcd.print("REFRIG:");
  lcd.setCursor(10, 2);
  lcd.print("EXAUST:");
}

void loop() {
  
// Leitura do sensor de nível e mostrando estado do reservatório
  estadoReservatorio = !digitalRead(pinNivel);
  lcd.setCursor(7, 3);
  if (estadoReservatorio == LOW) {
    lcd.print("Vazio"); 
  }
  else {
    lcd.print("Cheio");
  }

// Leitura do sensor de umidade do solo e mostrando valor
  umidadeSolo = analogRead(pinUmidSolo);
  umidadeSolo = map(umidadeSolo, 360, 1005, 100, 0);
  lcd.setCursor(17, 0);
  lcd.print(umidadeSolo);
  lcd.print("%"); 

// Leitura do sensor de temperatura e umidade do ar e mostrando valores
  objSensorTempUmid.read11(pinSensorTempUmid);
  temperatura = objSensorTempUmid.temperature;
  umidadeAr = objSensorTempUmid.humidity;
  lcd.setCursor(5, 0);
  lcd.print(temperatura);
  lcd.print((char)223);
  lcd.setCursor(5, 1);
  lcd.print(umidadeAr);
  lcd.print("%");

  controleDeTemperatura();
  controleDeUmidadeDoAr();
  controleLuminosidade();

  lcd.setCursor(17, 1);
  if (!estadoPeltier && !estadoCooler) {
    lcd.print("On ");
  }
  else {
    lcd.print("Off");
  }
  
  lcd.setCursor(17, 2);
  if (!estadoExaust) {
    lcd.print("On ");
  }
  else {
    lcd.print("Off");
  }

  Serial.print("tempo LED:");
  Serial.print(tempoLed);
  Serial.println(" segundos");
  Serial.print("tempo Peltier:");
  Serial.print(tempoPeltier);
  Serial.println(" segundos");
  Serial.print("Peltier:");
  Serial.println(estadoPeltier);
  Serial.print("Cooler:");
  Serial.println(estadoCooler);
  Serial.print("Exaustao:");
  Serial.println(estadoExaust);
  Serial.print("LED:");
  Serial.println(estadoLed);
  Serial.println("================================");
  
  tempoLed++;
  tempoPeltier++;
  
  delay(770);
}


// Controle temperatura
void controleDeTemperatura() {
  if (tempoPeltier >= 0 && tempoPeltier<= 600) {
    lcd.setCursor(17, 1);    
    if (temperatura > 24) {
      digitalWrite(pinAtuadorPeltier, LOW);
      digitalWrite(pinAtuadorCooler, LOW);
      estadoPeltier = LOW;
      estadoCooler = LOW;
      temperaturaAnt = temperatura;
    }
    else if (temperatura == 15) {
      digitalWrite(pinAtuadorPeltier, HIGH);
      digitalWrite(pinAtuadorCooler, HIGH);
      estadoPeltier = HIGH;
      estadoCooler = HIGH;
      temperaturaAnt = temperatura;
    }
  }
  else if (tempoPeltier > 600 && tempoPeltier <= 720) {
    digitalWrite(pinAtuadorPeltier, HIGH);
    digitalWrite(pinAtuadorCooler, LOW);
    estadoPeltier = HIGH;
    estadoCooler = LOW;
  }
  else {
    digitalWrite(pinAtuadorPeltier, HIGH);
    digitalWrite(pinAtuadorCooler, HIGH);
    delay(100);
    tempoPeltier = 1;
  }
}


// Controle umidade do ar
void controleDeUmidadeDoAr() {
  if (umidadeAr > 80) {
    digitalWrite(pinAtuadorExaustao, LOW);
    estadoExaust = LOW;
    umidadeArAnt = umidadeAr;
  }
  else if (umidadeAr == 50) {
    digitalWrite(pinAtuadorExaustao, HIGH);
    estadoExaust = HIGH;
    umidadeArAnt = umidadeAr;
  }
}

// Controle luminosidade (os valores 43200 e 86400 representam 12h e 24h em segundos)
void controleLuminosidade() {
  lcd.setCursor(4, 2);
  if (tempoLed <= 43200) {
    digitalWrite(pinLed, LOW);
    estadoLed = LOW;
    lcd.print("On ");
  }
  
  else if (tempoLed > 43200 && tempoLed <= 86400) {
    digitalWrite(pinLed, HIGH);
    estadoLed = HIGH;
    lcd.print("Off");
  }

  else {
    tempoLed = 1;
  }
}
