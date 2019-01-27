
 
volatile int sensorValue = 0;                   // Напряжение приведенное к вольтам
volatile int contur_status;                     // Переменная хранит номер ячейки массива  включенным каналом
volatile int all_conturs[10];      //
volatile  boolean stab_vcl;                     // статус включен сейчас стабилизатор или нет (задействована ли первичная обмотка стабилизатора или напряжение идет напрямую)
volatile int penultimate_contur_status;         // [хранит номер предпоследнего включеного контура
 
volatile  long max_sensor_value;                // хранит максимальное значение измеряемого напряжения в каждом периоде измерения
volatile byte  measure_count;                   // счетчик таймера 2
volatile byte  time_stabil;                     // счетчик времени норального напряжения
volatile int timer_mini;
volatile int timer_delay1;

 
#define CYCLE_1_TIME 5         // время циклa 5ms не используется
#define TIME_MIN 60000         //таймер 1 min
#define TIME_STAB_WORK 1       // 1 min    время стабильного состояния напряжения без работы стабилизатора
#define CYCLE_2_TIME 100       // время цикла  100ms не используется
#define MEASURE_PERIOD 20      // время цикла   30ms  время периода измерения напряжения
#define TIME_DELAY 25          // время цикла  50ms   время задержки после переключения канала

#define KOEFIC_VOLT 0.0706033376
#define USTAV_VOLT 220
#define CONTUR_FIRST 0
#define CONTUR_LAST 9
#define STEP_IGNOR 10 

#define analogInPin PA0
#define on_stab   PB10  
#define off_stab  PB11
#define contur_1  PB12
#define contur_2  PB13 
#define contur_3  PB14 
#define contur_4  PB15
#define contur_5  PB3
#define contur_6  PB4 
#define contur_7  PB5 
#define contur_8  PB6 
#define contur_9  PB7 
#define contur_10 PB8 
 
//---------- Обявление методов инициализации
 
 void pin_ini();
 void all_conturs_ini();
 void timer_ini();
 
//---------------------



 
void setup() {
   digitalWrite(on_stab,LOW);                                                                                                                             
   digitalWrite(off_stab,LOW); 
  contur_status = 0;
   pin_ini();
   all_conturs_ini();
   timer_ini(); 
  stab_vcl = LOW;
  sensorValue = USTAV_VOLT;
  max_sensor_value = 0;
  Serial1.begin(9600);

}


void loop() {
  readmaxValue();
  work_with_timers();
  chek_vcl_stab();
  upr_stab();
  

 Serial1.println(contur_status);

  
}


//  void contur_off(){                                                        // Выключаю старший канал и включаю младший
//  if(contur_status > CONTUR_FIRST){
//    contur_status -=1;
//    digitalWrite(all_conturs[contur_status], HIGH);
//    digitalWrite(all_conturs[contur_status + 1], LOW);
//    // Serial1.println('Contur_--'+contur_status+'VIKL');
//     startDelay();
//  } 
//   else { digitalWrite(all_conturs[contur_status], LOW);
//   // Serial1.println('Contur_--'+contur_status+'VIKL');
//    startDelay();
//   }
//}
//
//  
//  void contur_on(){
//  if (contur_status == CONTUR_FIRST && digitalRead(all_conturs[contur_status]) == LOW){
//     digitalWrite(all_conturs[contur_status], HIGH);
//      Serial1.println('Contur_--'+contur_status+'VCL');
//   //  startDelay();
//   return;
//  }
//  if(contur_status != CONTUR_LAST){
//    contur_status +=1;   
//    digitalWrite(all_conturs[contur_status], HIGH);
//    digitalWrite(all_conturs[contur_status - 1], LOW);  // Тут наоборот
//    Serial1.println('Contur_--'+contur_status+'VCL');
//   // startDelay();
//    return;
//   }
// }

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

void contur_off(){                                                       
    if(contur_status != CONTUR_FIRST){
    check_countur_status(LOW);
    
    digitalWrite(all_conturs[contur_status], HIGH);
    
    digitalWrite(all_conturs[penultimate_contur_status], LOW);
    startDelay();
  } 
   else digitalWrite(all_conturs[contur_status], LOW);
   startDelay();
}

  
  void contur_on(){
  if (contur_status == CONTUR_FIRST && digitalRead(all_conturs[contur_status]) == LOW){
     digitalWrite(all_conturs[contur_status], HIGH);
     startDelay();
   return;
  }
  else{
     check_countur_status(HIGH);  
    digitalWrite(all_conturs[contur_status], HIGH);
    digitalWrite(all_conturs[penultimate_contur_status], LOW);  // Тут наоборот
    startDelay();
   }
 }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------


void upr_stab(){
   if(sensorValue < USTAV_VOLT - STEP_IGNOR && stab_vcl == HIGH){        // Tут принимаю решение  надо вкл или выкл котур     
      contur_on();                                                        // Выходит за рамки усавки с дельтой значит принимаем решение
   }
    if(sensorValue >  USTAV_VOLT + (STEP_IGNOR / 2) && stab_vcl == HIGH){   
    contur_off();
  } 
}


void chek_vcl_stab(){                                                                                        // Задача этой функции понять нужно ли включать первичную обмотку или нет
 if(sensorValue <  USTAV_VOLT - (STEP_IGNOR *2) && stab_vcl == LOW){                                        // Если стаб включен то просадку первых 20в игнорируем если больше     
      digitalWrite (off_stab,HIGH);
      delay(10);
      digitalWrite(on_stab,HIGH) ;                                                                          //включаем питание обмотки
      stab_vcl = HIGH; 
      Serial.println('Stab On');                                                    
 }
 if(sensorValue > (USTAV_VOLT - STEP_IGNOR) && stab_vcl == HIGH ){                                          // Если напряжение в норме и стаб включен
      if(contur_status == CONTUR_FIRST && digitalRead(contur_1)== LOW && time_stabil == TIME_STAB_WORK ){   // проверяю не являеться ли норма результатом работ стабилизатора
        digitalWrite(on_stab,LOW);                                                                          // И если нет то включаю обмотку
        delay(10);                                                        
        digitalWrite(off_stab,LOW);                                                                          // Хотелось бы организовать задержку по принятию решения на отключение
        stab_vcl = LOW;
        Serial.println('Stab Off');  
    }
 }
}


void work_with_timers(){
  if(timer_mini >TIME_MIN){
    time_stabil++;
  timer_mini = 0;
  }
  if(time_stabil > TIME_STAB_WORK){
      time_stabil = 0;
    timer_mini = 0;  
  }
  if(sensorValue < (USTAV_VOLT - STEP_IGNOR)){
       time_stabil = 0;
    timer_mini = 0;
  }
   if( timer_delay1 > TIME_DELAY){
    timer_delay1 = 0;
  }
}


void func_timer(){

  measure_count ++;
  timer_delay1++;
  timer_mini++;

 }


 void readmaxValue(){
  max_sensor_value = max(max_sensor_value,(int)analogRead(analogInPin));
  if(measure_count >= MEASURE_PERIOD ){
    measure_count = 0;
    sensorValue =(int) (max_sensor_value*KOEFIC_VOLT); // НАДО УМНОЖИТЬ НА КОЭФИЦИЕНТ ПРИВЕДЕНИЯ К 220 ВОЛЬТ
    max_sensor_value = 0;
    Serial1.println(sensorValue);
  }
 
 }


 void startDelay(){
  timer_delay1 = 0;
  boolean flag = HIGH;
  while(flag ==HIGH){
    if(timer_delay1==TIME_DELAY){
      flag = LOW;
    }
  }
 }

  void check_countur_status(boolean mode){
  int val  = USTAV_VOLT - sensorValue;
  if(val<0) {
    val = -1*val;
    }
    val = val/STEP_IGNOR;
    int ost = sensorValue%STEP_IGNOR; 
  if(ost>6){
    val+=1;
    }
   if(val == 0){
    val==1;
   }
  penultimate_contur_status = contur_status;
  if(mode == HIGH){
  contur_status = min(contur_status + val,CONTUR_LAST);
  }
  if(mode == LOW){
  contur_status = max(contur_status - val,CONTUR_FIRST);
  }
  
  }


 void pin_ini(){
    pinMode(PC13, OUTPUT);
    pinMode(contur_1, OUTPUT);
    pinMode(contur_2, OUTPUT);
    pinMode(contur_3, OUTPUT);
    pinMode(contur_4, OUTPUT);
    pinMode(contur_5, OUTPUT);
    pinMode(contur_6, OUTPUT);
    pinMode(contur_7, OUTPUT);
    pinMode(contur_8, OUTPUT);
    pinMode(contur_9, OUTPUT);
    pinMode(contur_10, OUTPUT);
    pinMode(on_stab, OUTPUT);
    pinMode(off_stab, OUTPUT);
 }


 void all_conturs_ini(){
  
    all_conturs[0] = contur_1 ;
    all_conturs[1] = contur_2;
    all_conturs[2] = contur_3;
    all_conturs[3] = contur_4;
    all_conturs[4] = contur_5;
    all_conturs[5] = contur_6;
    all_conturs[6] = contur_7;
    all_conturs[7] = contur_8;
    all_conturs[8] = contur_9;
    all_conturs[9] = contur_10;

 }


 void timer_ini(){
   Timer3.pause(); // останавливаем таймер перед настройкой
  Timer3.setPeriod(1000); // время в микросекундах (1мс)
  Timer3.attachInterrupt(TIMER_UPDATE_INTERRUPT, func_timer); // активируем прерывание
  Timer3.refresh(); // обнулить таймер 
  Timer3.resume(); // запускаем таймер
 }
 voidtestGit(){
	 
