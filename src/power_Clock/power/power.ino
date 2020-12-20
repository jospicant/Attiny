
//********************************************************************************************************************************************************
/*
 * Estudiamos como cambiando la frecuencia del reloj los consumos del Attiny descienden drásticamente
 *  unos 850uA a 1MHz con todo activo ( trabajando normal )
 *  unos 395uA a 31Khz con todo activo ( trabajando normal ), esto es un ahorro de unos 450 uA 
 *  Si se desactiva el ADC tendremos unos 200 uA menos de consumo
 *  
 *  
 */
#include <Arduino.h>
#include <avr/power.h>

#define led 0   // led en PB0
#define adc_disable() (ADCSRA &=~(1 <<ADEN))   // ADEN = 7   1 << 7  = >  1000_0000 invertido = > 0111_1111  esto AND con lo que había lo q hace es poner a 0 el bit ADEN  ADEN=0
#define adc_enable() (ADCSRA |= (1 <<ADEN))    //                         1000_0000 or con ADCSRA =>  ADEN =1


void setup() {
  // put your setup code here, to run once:
  pinMode(led,OUTPUT);

  //Puertos no usados como INPUT_PULLUP con el fin de tener lógica controlada y evitar transiciones
  //ESto no ahorra pero controla que se produzcan transiciones incontroladas = consumo
  pinMode(PB1,INPUT_PULLUP);pinMode(PB2,INPUT_PULLUP);pinMode(PB3,INPUT_PULLUP);pinMode(PB4,INPUT_PULLUP);pinMode(PB5,INPUT_PULLUP); //Puertos no usados como INPUT_PULLUP
  
  clock_prescale_set(clock_div_8);  // configuro Fuses a 1MHz por defecto   --> controlo FUSES DEL RELOJ
  
}

void loop() {
  
  digitalWrite(led,1);                            
  delay(2000);
  digitalWrite(led,0);    //Enciendo 2 sg el LED                                3.6 mA con led encendido
  delay(4000);            //Mido corriente cuando se apaga el LED               850 uA con el Attiny85 Activo (con led apagado)
  adc_disable();          //Deshabilito el módulo d ADC y mido el consumo       650 uA con Attiny85 Activo pero con ADC desactivado  ( 200 uA de ahorro )           
  delay(4000);
  adc_enable();           //Habilito el módulo de ADC y mido el consumo         850 uA al reactivar el ADC.
  delay(4000);

  clock_prescale_set(clock_div_256);  // configuro reloj a  8MHz/256 = 31250 Hz   ==> 1MHz / 32   ( 1000 ms = 32 sg ) 31,25ms = 1 sg   125ms = 4sg

  digitalWrite(led,1);                            
  delay(125);            //4sg
  digitalWrite(led,0);   
  delay(125);                                                                  
  adc_disable();                     
  delay(125);                                                                 // 395 uA con ADC y 195uA sin ADC
  adc_enable();           
  delay(125);
  
  clock_prescale_set(clock_div_8);


}
