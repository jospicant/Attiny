
/*******************************************************************************************************************************************************************************
 * *****************************************************************************************************************************************************************************
 * 
 * Pondremos a dormir el Attiny, en modo  SLEEP_MODE_PWR_DOWN. Este modo es el que consume menos ya que deshabilita casi todos los relojes
 * a excepción del Watchdog y solo puede despertar por INT0,Pin Change int (PCIE), Condición de inicio por USI (por puerto Serie) o por WatchDog.
 * 
 * Despertaremos al Attiny produciendo una interrupción INT0 por nivel bajo con un pulsador en el PIN7 ( PB2 ) del Attiny
 *  * 
 * Al encender el micro por primera vez, realizamos ciertas operaciones y me pongo a dormir ( mediante el flag  "interrupcion_ok" controlamos si ha producido una interrupción )
 * Despertaremos por la INT0 a nivel bajo ( asíncrono, no necesitamos del reloj ppal, por lo q podremos pararlo y no nos impidirá despertar con esta interrupción )
 * Una vez despertamos haremos lo que necesitemos y volveremos a dormir esperando otra int INT0.
 * 
 * Se controla bien cuando están las interrupciones activas para evitar que se produzcan interrupciones incontroladas, solo las queremos para despertar el micro
 * por el pin7
 * 
 ******************************************************************************************************************************************************************************
 ******************************************************************************************************************************************************************************/
 
#include <Arduino.h>
#include <avr/sleep.h>


#define led 0   // led en PB0

#define adc_disable() (ADCSRA &=~(1 <<ADEN))   // ADEN = 7   1 << 7  = >  1000_0000 invertido = > 0111_1111  esto AND con lo que había lo q hace es poner a 0 el bit ADEN  ADEN=0
#define adc_enable() (ADCSRA |= (1 <<ADEN))    //                         1000_0000 or con ADCSRA =>  ADEN =1
#define Calib_ms 49                            // 1 ms a 1MHz 49 ciclos

volatile bool interrupcion_ok = false;         // Uso tipo volatile para que la interrupción no le afecte


//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
void setup() {

  cli();  // Si Deshabilito interrupciones, delay() ya no funcionará y si lo uso no se comportará correctamente pq usa las interrupciones de los timers --> uso una función DelMs(donde calibro aprox los ms )
  pinMode(led,OUTPUT);
  pinMode(PB1,INPUT_PULLUP);pinMode(PB2,INPUT_PULLUP);pinMode(PB3,INPUT_PULLUP);pinMode(PB4,INPUT_PULLUP);pinMode(PB5,INPUT_PULLUP); //Puertos no usados como INPUT_PULLUP
  adc_disable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);            // Desactivo ADC y configuro Modo en el que voy a dormir
  bitSet(GIMSK,INT0);                             // GIMSK (  INT0=1  PCIE = 0 ) Configuro INterrupción INT0 ( puerto PB2 = pin7 ) 
  bitClear(GIMSK,PCIE);
  bitClear(MCUCR,ISC00);                          // MCUCR ( ICSC00 = 0  ISC01 = 0  Activo por nivel bajo la interrupción INT0 )
  bitClear(MCUCR,ISC01);
  
 }

//*****************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************
void loop() {

   static unsigned int cuenta_int=0; 
   
  //********************* Aquí pondré todo lo q tiene q hacer antes de ir a dormir **************************
  //******** evitaré interrupciones pq quiero q haga todo lo que quiera sin q se active la rutina de int ****
  
  if( interrupcion_ok == false){                         //Primera vez q se alimenta o después de un Reset
    cuenta_int=0;
    interrupcion_ok=false;
    for(int i=0; i<3; i++){ Parpadea_led(250); }
  }
  else{                                                 //Aquí solo entra si viene de una interrupción INT0 ( controlado por flag  interrupcion_ok = false o true )
                                                        //Aquí haré todo lo que quiera hacer tras haber despertado por INT0
    cuenta_int++;                                        //actualizo cuenta y flag de interrupción
    interrupcion_ok=false;
    
    Parpadea_led(1000);                                  //Hago todo lo q quiera tras ocurrir la interrución q me despertó 
    for (int i=0;i<cuenta_int; i++){
      Parpadea_led(200);
    }
        
   //********************************* Ahora ya lo duermo *************************************************
   //******************************************************************************************************
  }
   
    A_Dormir();                               // solo despierta por int  INT0, aquí habilitaré int para poder despertar por INT0
                                              // al volver de la interrupción, las interrupciones se activan solas --> sei() por lo q si quiero seguir controlando 
                                              // q no se produzcan interr, debo desactivarlas.
    cli();    //importante, si no las desactivo puedo producir una INT0 en cualquier parte del código y prefiero tenerlo controlado y que solo se produzca para despertar
              // el Attiny

}

//************************************************************************************************************
//************************************************************************************************************

//Uso esta función para producir unos retrasos artificiales, sin interrupciones, solo por tiempo usado
//en los bucles for

void DelMs(unsigned int milisg){                         // Nota: funciona con volatile 
  for (volatile int i=0; i<milisg; i++){
    for(volatile int j=0; j<Calib_ms; j++){     //este bucle se calibra para que dure aprox 1 ms
      //x bucles/msg   200msg  == 200 veces * x
    }
  }
}

//*************************************************************************************************************
void A_Dormir(){
  sei();                //Habilito int antes de ponerme a dormir pq si no, no podré despertarlo ( salvo reset o apagarlo )
  sleep_enable();       //solo habilitaré int aquí para poder despertar por INT0
  sleep_cpu();
}

//*************************************************************************************************************
void Parpadea_led(int tiempo){
  digitalWrite(led,HIGH); DelMs(tiempo); digitalWrite(led,LOW); DelMs(tiempo);
}


//**************************************************************************************************************
ISR(INT0_vect)
{
  cli();                 //Aseguro int desactivadas para q no se produzca ninguna otra mientras estoy en esta int
  interrupcion_ok=true;  //actualizo Flag
  //cuenta_int++;        // aquí no funciona bien ( aunque sea volatile )
  //cuando sale de la interrupción, se activa automáticamente la interrupción  --> sei()  Ojo con esto.
}
