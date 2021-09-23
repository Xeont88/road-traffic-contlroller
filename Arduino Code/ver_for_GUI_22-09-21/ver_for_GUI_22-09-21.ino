#include "Parser.h"


#include <PololuLedStrip.h>
#include <iarduino_MultiServo.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

SoftwareSerial mySerial(13, 12); // RX, TX
//
//  В КОРНЕ  SD КАРТЫ - СОЗДАТЬ ПАПКУ mp3  ВНИМАНИЕ !!!!!   ВНИМАНИЕ !!!!!


iarduino_MultiServo MSS;

#define DEBUG 0

// *********************    При первом включении и поиске датчика - вращение д.б. быть по ЧАСОВОЙ (вид сверху).
#define rotate true  // изменить для настройки направления вращения
//#define rotate false  // изменить для настройки направления вращения
#define stp 2 // 2 пин к step  
#define dir 3 // 3 пин к dir
///  ***********************************  настройка напряжения драйвера DRV8825   200 - 300 mV !!!!!!!!!!!!!


#define mag_1 5 // этот пин к геркону   // крайнее положение (в упор) против часовой  (выбрать № датчика 5,6,7 или 8 для этого положения)
#define mag_2 6 // этот пин к геркону   //   следующее по часовой   -\\---         
#define mag_3 7 // этот пин к геркону   //   следующее по часовой   -\\----             
#define mag_4 8 // этот пин к геркону   //  крайнее положение (в упор) по часовой  -\\----

#define ch_LED_1 3 // Сменить матрицу (1) (6-00) на ...
#define ch_LED_2 4 // Сменить матрицу (2) (9-00) на ...
#define ch_LED_3 1 // Сменить матрицу (3) (12-00) на ...
#define ch_LED_4 2 // Сменить матрицу (4) (3-00) на ...
const byte LED_enum[] = {ch_LED_1, ch_LED_2, ch_LED_3, ch_LED_4};

#define led_pin 4 //подключите 4 пин к LED матрицам 6812

#define LED_COUNT 81 // число светодиодов  81
#ifdef DEBUG
#define LED_COUNT 81 // число светодиодов  2
#endif

PololuLedStrip<led_pin> ledStrip;

byte def_pos; // текущая позиция

const byte pos_enum[] = {mag_1, mag_2, mag_3, mag_4};
const int val_pause = 1000; // Задержка между шагами  STEP MOTOR  в микросекундах (больше значение - ниже скорость)
int move_pause; // время на поиск ближайщего датчика (в миллисекундах)

/////  For LED function ////////////////////////
const byte light_red = 8;  // яркость красного
const byte light_green = 4; // яркость зеленого

rgb_color colors[LED_COUNT * 4];
rgb_color color_r = {light_red, 0, 0}; // для красного цвета
rgb_color color_g = {0, light_green, 0}; // для зеленого цвета
rgb_color color_null = {0, 0, 0}; // погасить

const uint8_t data_led_STOP[81] PROGMEM = {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t data_led_all[81] PROGMEM = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t data_led_left[81] PROGMEM = {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t data_led_up_right[81] PROGMEM = {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t data_led_right[81] PROGMEM = {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t data_led_up[81] PROGMEM = {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};


// использовать, в случае неточно установленного сервопривода (рука)
#define POS_0    00
#define POS_90   60
#define POS_180 120

const byte num_servo = 1;   // номера канала PCA SERVO двигателя
byte def_angle; // 0 or 90 or 180

byte r_pos;



void setup() {

  Serial.flush();
  Serial.begin(115200);
  while (!Serial);
  //  pinMode(LED_PIN, 1);

  MSS.begin(); // Servo
  delay(100);
  MSS.servoSet(num_servo, 180, 100, 535);

  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(mag_1, INPUT_PULLUP);
  pinMode(mag_2, INPUT_PULLUP);
  pinMode(mag_3, INPUT_PULLUP);
  pinMode(mag_4, INPUT_PULLUP);

  // for свисток
  mySerial.begin(9600);
  mp3_set_serial (mySerial);    //set Serial for DFPlayer-mini mp3 module
  delay (100);
  mp3_set_volume (30);
  ///////////////////////////


  arrow_OFF();
  flash_LED();
  randomSeed(analogRead(0)); // в качестве опорного числа взять сигнал с НЕПОДКЛЮЧЕННОГО НИКУДА аналогового пина

  MSS.servoWrite(num_servo, POS_0);
  def_angle = POS_0;

  get_pos();
#ifdef DEBUG
  Serial.println(def_pos);
  Serial.println();
  delay(5000);
#endif

  delay(1000);

}


//int hand;
//int pos;

void loop() {

  const byte stop_enum[] = {3, 4, 1, 2};
  const byte all_enum[] = {4, 1, 2, 3};

  if (Serial.available()) {
    char buf[50];
    Serial.readBytesUntil(';', buf, 50);
    Parser data(buf, ',');
    int ints[10];
    data.parseInts(ints);

    switch (ints[0]) {

      // если 1 - задать положение
      case 1:
        set_pos(ints[1]);
        hand_moving(ints[2]);
//        flash_LED();
        break;

      // если 2 - показать ответ (светоиндикация)
      case 2:
        // если рука вниз
        if (ints[2] == 1) {
          if (ints[1] == 1 || ints[1] == 3) {
            arrow_STOP(1);
            arrow_up_right(2);
            arrow_STOP(3);
            arrow_up_right(4);
          } else {
            arrow_up_right(1);
            arrow_STOP(2);
            arrow_up_right(3);
            arrow_STOP(4);
          }
        }

        // если рука вперед
        if (ints[2] == 2) {
          arrow_right(ints[1]);
          arrow_STOP(ints[1] % 4 + 1);
          arrow_STOP(stop_enum[ints[1] - 1]);
          arrow_all(all_enum[ints[1] - 1]);
        }

        // если рука вверх - стоп по всем направлениям
        if (ints[2] == 3) {
          whistle();
          arrow_STOP(1);
          arrow_STOP(2);
          arrow_STOP(3);
          arrow_STOP(4);
          //          flash_LED();
        }

        flash_LED();
        break;

      // если 3 - опустить руку, выключить индикацию (LEDs)
      case 3:
        MSS.servoWrite(num_servo, POS_0);
        arrow_OFF();
      break;
      
      // если 4 - изменить громкость плейера
      case 4:
        mp3_set_volume (ints[1] * 3);
        break;
    }
    //    arrow_OFF();
  }
}

void hand_moving(byte hand) {
  if (hand == 1) {
    MSS.servoWrite(num_servo, POS_0);
  } else if (hand == 2) {
    MSS.servoWrite(num_servo, POS_90);
  } else if (hand == 3) {
    MSS.servoWrite(num_servo, POS_180);
  }
}

void arrow_OFF() {
  for (int i = 0; i < LED_COUNT * 4; i++) {
    colors[i] = color_null;
  }
  flash_LED();
}

void flash_LED() {
  ledStrip.write(colors, LED_COUNT * 4); // Send the values to the LEDs
  delay(200);//задержка
}

void set_pos(byte pos) { // pos 1...4
  if (pos == get_pos()) {
    return;
  }

  bool CW = !rotate;
  if (pos > def_pos) {
    CW = rotate;
  }
  digitalWrite(dir, CW);

  //move_pause = val_pause * abs(pos - def_pos);
  move_pause = 10000;


  unsigned long time_start = millis();
  while (millis() - time_start < move_pause) {
    digitalWrite(stp, HIGH);
    delayMicroseconds(val_pause);
    digitalWrite(stp, LOW);
    delayMicroseconds(val_pause);
    if ( !digitalRead(pos_enum[pos - 1]) ) {
      def_pos = pos;
      break;
    }
  }



}

byte get_pos() {
  bool scan_mag[5];
  scan_mag[1] = !digitalRead(mag_1);
  delay(2);
  scan_mag[2] = !digitalRead(mag_2);
  delay(2);
  scan_mag[3] = !digitalRead(mag_3);
  delay(2);
  scan_mag[4] = !digitalRead(mag_4);
  delay(2);

#ifdef DEBUG
  Serial.print(scan_mag[1]); Serial.print(" - "); Serial.print(scan_mag[2]); Serial.print(" - "); Serial.print(scan_mag[3]); Serial.print(" - "); Serial.println(scan_mag[4]); Serial.println();
#endif

  byte i = 0;
  if (scan_mag[1] || scan_mag[2] || scan_mag[3] || scan_mag[4] ) {
    for (i = 1; i < 5; i++) {
      if (scan_mag[i]) {
        def_pos = i;
        break;
      }
    }
  }
  else {
    // если не в позиции, то выставить в ближайшую
    return move_near_pos();
  }
  return i;
}

byte move_near_pos() {
  move_pause = 2000;
  // против часовой
  bool CW = rotate;
  digitalWrite(dir, CW);

  unsigned long time_start = millis();
  byte n = 1; //
  while (millis() - time_start < move_pause) {
    digitalWrite(stp, HIGH);
    delayMicroseconds(val_pause);
    digitalWrite(stp, LOW);
    delayMicroseconds(val_pause);
    if ( !digitalRead(pos_enum[n - 1]) ) {
      def_pos = n;
      return n;
    }
    n++;
    if (n > 4) {
      n = 1;
    }
  }

  // по часовой
  CW = !rotate;
  digitalWrite(dir, CW);

  time_start = millis();
  n = 1; //
  while (millis() - time_start < move_pause) {
    digitalWrite(stp, HIGH);
    delayMicroseconds(val_pause);
    digitalWrite(stp, LOW);
    delayMicroseconds(val_pause);
    if ( !digitalRead(pos_enum[n - 1]) ) {
      def_pos = n;
      return n;
    }
    n++;
    if (n > 4) {
      n = 1;
    }
  }
  return 0;
}

void arrow_up(byte addr) {

  addr = LED_enum[addr - 1]; // если замена сектора

  int n = 0;

  for (int i = (addr - 1) * LED_COUNT; i < (addr - 1)*LED_COUNT + LED_COUNT ; i++) { // счётчик от 0 до 81
    if (pgm_read_byte(&data_led_up[n]) == 1) {
      colors[i] = color_g;
    }
    else {
      colors[i] = color_null;
    }
    n++;
  }
}

void arrow_right(byte addr) {

  addr = LED_enum[addr - 1]; // если замена сектора

  int n = 0;

  for (int i = (addr - 1) * LED_COUNT; i < (addr - 1)*LED_COUNT + LED_COUNT ; i++) { // счётчик от 0 до 81
    if (pgm_read_byte(&data_led_right[n]) == 1) {
      colors[i] = color_g;
    }
    else {
      colors[i] = color_null;
    }
    n++;
  }
}

void arrow_up_right(byte addr) {

  addr = LED_enum[addr - 1]; // если замена сектора

  int n = 0;

  for (int i = (addr - 1) * LED_COUNT; i < (addr - 1)*LED_COUNT + LED_COUNT ; i++) { // счётчик от 0 до 81
    if (pgm_read_byte(&data_led_up_right[n]) == 1) {
      colors[i] = color_g;
    }
    else {
      colors[i] = color_null;
    }
    n++;
  }
}

void arrow_left(byte addr) {

  addr = LED_enum[addr - 1]; // если замена сектора

  int n = 0;

  for (int i = (addr - 1) * LED_COUNT; i < (addr - 1)*LED_COUNT + LED_COUNT ; i++) { // счётчик от 0 до 81
    if (pgm_read_byte(&data_led_left[n]) == 1) {
      colors[i] = color_g;
    }
    else {
      colors[i] = color_null;
    }
    n++;
  }
}

void arrow_all(byte addr) {

  addr = LED_enum[addr - 1]; // если замена сектора

  int n = 0;

  for (int i = (addr - 1) * LED_COUNT; i < (addr - 1)*LED_COUNT + LED_COUNT ; i++) { // счётчик от 0 до 81
    if (pgm_read_byte(&data_led_all[n]) == 1) {
      colors[i] = color_g;
    }
    else {
      colors[i] = color_null;
    }
    n++;

  }
}

void arrow_STOP(byte addr) {

  addr = LED_enum[addr - 1]; // если замена сектора

  int n = 0;

  for (int i = (addr - 1) * LED_COUNT; i < (addr - 1)*LED_COUNT + LED_COUNT ; i++) { // счётчик от 0 до 81
    if (pgm_read_byte(&data_led_STOP[n]) == 1) {
      colors[i] = color_r;
    }
    else {
      colors[i] = color_null;
    }
    n++;
  }
}


void whistle() {
  delay (20);
  int melody = random(1, 5);
  mp3_play (melody);  // Проигрываем "mp3/0001.mp3"
  delay (20);

  //  delay(3000);
}
