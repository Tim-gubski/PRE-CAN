// Linear Potentiometer Code, 0-5kOhms roughly, some of the potentiometers
// have different ranges unfortunately, so some need to be rescaled
// FL - front left, FR - front right, RL - rear left, RR - rear right
// refer to tires of the car & approx locations of the sensors

//front sus
#define POT1 32 //FL
#define POT2 33 //FR check on pcb
//rear sus
#define POT3 34 //RL
#define POT4 35 //RR check on pcb

#define PRESSURE1 34 // FRONT
#define PRESSURE2 12 // REAR check on pcb

#define BR_LIGHT 14
#define LED 2

// reading vars
int pot1_total = 0;
int pot2_total = 0;
int pot3_total = 0;
int pot4_total = 0;
int pressure1_total = 0;
int pressure2_total = 0;

int breaklight_threshold = 400;

int i = 0;
long lastTime = millis();
// blink vars
long blinkTime = millis();
bool blinkState = false;


void setup() {
  // put your setup code here, to run once:

  pinMode(POT1, INPUT);
  pinMode(POT2, INPUT);
  pinMode(POT3, INPUT);
  pinMode(POT4, INPUT);
  pinMode(PRESSURE1, INPUT);
  pinMode(PRESSURE2, INPUT);
  pinMode(BR_LIGHT, OUTPUT);
  pinMode(LED, OUTPUT);

  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("Front Embedded Controller");
}

void loop() {
  long currentTime = millis();

  // every 50ms measure averages
  if(currentTime - lastTime > 200){
      int pot1_avg = 0;
      int pot2_avg = 0;
      int pot3_avg = 0;
      int pot4_avg = 0;
      int pressure1_avg = 0;
      int pressure2_avg = 0;
      // avoid divide by zero
      if (i != 0){
        pot1_avg = pot1_total / i;
        pot2_avg = pot2_total / i;
        pot3_avg = pot3_total / i;
        pot4_avg = pot4_total / i;
        pressure1_avg = pressure1_total / i;
        pressure2_avg = pressure2_total / i;
      }

      int pressure1_voltage = map(pressure1_avg, 0, 4095, 0, 3300);
      int pressure2_voltage = map(pressure2_avg, 0, 4095, 0, 3300);
      // Serial.println(analogRead(PRESSURE1));
      Serial.println(pressure2_voltage);
      if (pressure2_voltage > breaklight_threshold){
        digitalWrite(BR_LIGHT, HIGH);      
      }else{
        digitalWrite(BR_LIGHT, LOW);      
      }

      // reset vals
      pot1_total = 0;
      pot2_total = 0;
      pot3_total = 0;
      pot4_total = 0;
      pressure1_total = 0;
      pressure2_total = 0;

      lastTime = currentTime;
      i = 0;
  }
  // accumulate readings
  pot1_total += analogRead(POT1);
  pot2_total += analogRead(POT2);
  pot3_total += analogRead(POT3);
  pot4_total += analogRead(POT4);
  pressure1_total += analogRead(PRESSURE1);
  pressure2_total += analogRead(PRESSURE2);
  i++;
  

  // blink internal led
  if(currentTime > blinkTime + 1000){
    if(blinkState){
      digitalWrite(LED,HIGH);
    }else{
      digitalWrite(LED,LOW);
    }   
    blinkState = !blinkState; 
    blinkTime = currentTime;
  }
}
