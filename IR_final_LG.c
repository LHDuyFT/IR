#include <p24fj128ga010.h> 
#include <xc.h>
// CONFIG2
#pragma config POSCMOD = HS             // Primary Oscillator Select (HS Oscillator mode selected)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRI              // Oscillator Select (Primary Oscillator (XT, HS, EC))
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = ON              // Watchdog Timer Enable (Watchdog Timer is enabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

#define SYSCLOCK 8000000
#define HEADER_M 8992
#define HEADER_S 4542
#define PTRAIL 512
#define BIT1_M 529
#define BIT1_S 1704
#define BIT0_M 529
#define BIT0_S 591

// must use pin 3 for IR output

void delay_us(int time)
{
    TMR2 = 0;
    T2CONbits.TON = 1;
    delaycycle = 4*time;
    while (TMR2 < delaycycle);
 
}

void setup() { 

  OC1CON = 0x000E;
  // frequency and duty cycle
  int khz = 38;
  PR3 = SYSCLOCK / 2 / khz / 1000;
  OC1R = 0;
  OC1RS = PR3 / 3; // 33% duty cycle
}
 
inline void mark(int time) {

  T3CONbits.TON = 1;//Activate the PWM module
  delay_us(time);
}
 
inline void space(int time) {

  T3CONbits.TON = 0;//Deactivate the PWM module
  delay_us(time);
}
 
void sendByte(unsigned char b) {
  int i;
  for (i = 0; i < 8; i++) {
    if (b & 1) {
      // one
      mark (BIT1_M);
      space(BIT1_S);
    } else {
      // zero
      mark (B IT0_M);
      space(BIT0_S);
    }
    b >>= 1;
  }
}
 /*
1.Xung phat voi f=38khz trong 0.5ms(set 529)
2.Khoang thoi gian giua 2 xung bit 1: 1.7ms
                               bit 0: 0.65ms
3.ham sendByte :lay bit cuoi de transmit
4.ham delay : delay
 */
void transmit() {
 // header
 mark (HEADER_M);
 space(HEADER_S);
 
 sendByte(0x04);
 sendByte(0xFB);
 sendByte(0xC5);
 sendByte(0x3A);
//CAN XEM LAI 
  mark(BIT1_M); //BIT1_M =529,on timer3, ena pin output PWM,delay 265us 
  space(39804); //off timer3,delay 20ms 
  mark(8985);   //on timer3,dalay 4.5ms
  space(2294);  //off timer3,delay 1ms
 
 // stop
 mark (PTRAIL);
 space(0);
}

main(void)
{
    TRISA = 0xFF00;
    TRISDbits.TRISD13 = 1;
    int isOn = 0;
    setup();
//    INTCON1bits.NSTDIS = 0;             
    while(1) 
    {             
        //check if press the right button
        if( PORTDbits.RD13 == 0)            
        {      
            if(isOn == 0)
            {
                isOn = 1;
                PORTA=0x000F;
                transmit();
            }
        }
        else
        {
            isOn = 0;
            PORTA = 0x00F0;
        }    
    }                  
}