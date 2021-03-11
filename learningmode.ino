#include <Servo.h>    //the library which helps us to control the servo motor
#include <SPI.h>      //the communication interface with the modem
#include "RF24.h"     //the library which helps us to control the radio modem
#include <FastLED.h>
#include <IRremote.h>
#include <EEPROM.h>

word remote_signal_code=0;  //remote_signal_code for IR remote control
int realtime_status=0,learning_status=0,ir_action_status=0;
bool label;// mark whether receive signal from ir_remote

//define the servo name
static int FINGER_PINS[4] = {5, 6, 9, 10};
static Servo finger_servos[4];

//define the radio
RF24 radio(7,8);     /*This object represents a modem connected to the Arduino. 
                      Arguments 9 and 10 are a digital pin numbers to which signals 
                      CE and CSN are connected.*/

const uint64_t pipe = 0xE1E4F2F8E1LL; //the address of the modem,that will receive data from the Arduino.

int msg[4];

//DEFINES of LED
#define NUM_LEDS 24
#define DATA_PIN 2
CRGB color;
CRGB leds[NUM_LEDS];
int currentLed = 0;

void gesture_release()
{
 finger_servos[0].writeMicroseconds(1000);
 for (int i=1;i<4;i++)
 {
  finger_servos[i].writeMicroseconds(2000);
 }
 return;
}

// IRRemote controller of Carmp3 kind
typedef enum {
    BTN_CH_MINUS = 41565,
    BTN_CH = 25245,
    BTN_CH_PLUS = 57885,
    BTN_PREV = 8925,
    BTN_NEXT = 765,
    BTN_PLAY = 49725,
    BTN_VOL_DOWN = 57375,
    BTN_VOL_UP = 43095,
    BTN_EQ = 36975,
    BTN_0 = 26775,
    BTN_100_PLUS = 39015,
    BTN_200_PLUS = 45135,
    BTN_1 = 12495,
    BTN_2 = 6375,
    BTN_3 = 31365,
    BTN_4 = 4335,
    BTN_5 = 14535,
    BTN_6 = 23205,
    BTN_7 = 17085,
    BTN_8 = 19125,
    BTN_9 = 21165,
    LONG_PRESS = 65535
} IR_REMOTE_KEYS;

IRrecv irrecv(4); // Initiate IR signal input

void handle_ir_remote_cmd() {
    decode_results remote_signal; // Save signal structure

    if (irrecv.decode(&remote_signal)) {
        //label2=true;
        remote_signal_code = remote_signal.value, HEX;
        if (remote_signal_code==BTN_CH||remote_signal_code==BTN_CH_PLUS||remote_signal_code==BTN_CH_MINUS)
          label=true;
        irrecv.resume();
    }
    else
      remote_signal_code =0;
      return;
}

void LED_SHOW(int a, int b, int c)
{
  if (a) color=CRGB::Yellow;
  else if (b) color=CRGB::Green;
  else if (c) color=CRGB::Blue;
  else color=CRGB::White;
  for (int i=0;i<NUM_LEDS;i++)
  {
    leds[i]=color;
  }
  FastLED.show();
  return;
}

void setup(){
  Serial.begin(9600);
  //define the servo input pins
  for (int i=0;i<4;i++)
  {
    finger_servos[i].attach(FINGER_PINS[i],1000,2000);
  }
  gesture_release();
  realtime_status=0;learning_status=0;ir_action_status=0;
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(10);
  LED_SHOW(0,0,0);
  irrecv.blink13(true);       // If signal is received, then pin13 LED light blink
  irrecv.enableIRIn(); 
  radio.begin();                    //it activates the modem.
  radio.openReadingPipe(1, pipe);   //determines the address of our modem which receive data.
  delay(1000);
}

void change_status()
{
  if (remote_signal_code==BTN_CH_MINUS)
  {
    realtime_status=(realtime_status+1)%2;
    learning_status=0;
    ir_action_status=0;
  }
  if (remote_signal_code==BTN_CH)
  {
    realtime_status=0;
    learning_status=(learning_status+1)%2;
    ir_action_status=0;
  }
  if (remote_signal_code==BTN_CH_PLUS)
  {
    realtime_status=0;
    learning_status=0;
    ir_action_status=(ir_action_status+1)%2;
  }
  LED_SHOW(realtime_status,learning_status,ir_action_status);
  gesture_release();
  delay(2000);
  return;
}

short int aa;
void learn(int button)
{
   radio.startListening();
   delay(10); 
   if(radio.available()){
      radio.read(msg, sizeof(msg));
      for (int i=0;i<4;i++)
      {
        aa=msg[i];
        EEPROM.put(button*8+i*2,aa);
      }
   }
   return;
}

short int bb;
void move_button(int button)
{
  for (int i=0;i<4;i++)
  { 
    EEPROM.get(button*8+i*2,bb);
    finger_servos[i].writeMicroseconds(bb);
  }
  return;
}

void movement_learning(int a)
{
  learn(a);
  move_button(a);
  delay(3000);
  gesture_release();
  return; 
}

void loop(){
  label=false;
  handle_ir_remote_cmd();
  //Serial.println(remote_signal_code);
  if (label) change_status();
  if (realtime_status)
  {
    if (label)
    {
      radio.startListening(); 
    }
    if(radio.available()){
      radio.read(msg, sizeof(msg));
      for (int i=0;i<4;i++)
      { 
        finger_servos[i].writeMicroseconds(msg[i]);
      }
    }
  }
  if (learning_status)
  {
    switch(remote_signal_code)
    {
      case BTN_0:{ movement_learning(0);
                  break;}
      case BTN_1:{ movement_learning(1);
                  break;}
      case BTN_2:{ movement_learning(2);
                  break;}
      case BTN_3:{ movement_learning(3);
                  break;}
      case BTN_4:{ movement_learning(4);
                  break;}
      case BTN_5:{ movement_learning(5);
                  break;}
      case BTN_6:{ movement_learning(6);
                  break;}
      case BTN_7:{ movement_learning(7);
                  break;}
      case BTN_8:{ movement_learning(8);
                  break;}
      case BTN_9:{ movement_learning(9);
                  break;}
    }
  }
  if (ir_action_status)
  {
      switch(remote_signal_code)
      {
      case BTN_0:{move_button(0);
                  break;}
      case BTN_1:{move_button(1);
                  break;}
      case BTN_2:{move_button(2);
                  break;}
      case BTN_3:{move_button(3);
                  break;}
      case BTN_4:{move_button(4);
                  break;}
      case BTN_5:{move_button(5);
                  break;}
      case BTN_6:{move_button(6);
                  break;}
      case BTN_7:{move_button(7);
                  break;}
      case BTN_8:{move_button(8);
                  break;}
      case BTN_9:{move_button(9);
                  break;}
      }
  }
}
