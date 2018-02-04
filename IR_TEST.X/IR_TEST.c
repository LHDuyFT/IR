#include <p24fj128ga010.h>
#include <xc.h>
// CONFIG2
#pragma config POSCMOD = HS             // Primary Oscillator Select (HS Oscillator mode selected)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRI              // Oscillator Select (Primary Oscillator (XT, HS, EC))
#pragma config IESO = OFF                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = OFF              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF              // Watchdog Timer Enable (Watchdog Timer is enabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

//-----------------------------------------------------------------------------------------------------------------------------------------------------------

#define SYSCLOCK 8000000
#define HEADER_M 9000
#define HEADER_S 4500
#define PTRAIL 562
#define BIT1_M 562
#define BIT1_S 1687
#define BIT0_M 562
#define BIT0_S 562

void delay_us(int time);
void setup();
inline void mark(int time);
inline void space(int time);
void sendByte(unsigned long b);
void transmit(unsigned long b);
void repeatcode();

//---------------------------------------------------------------------

main (void)
{
    TRISA = 0xFF00;
    TRISDbits.TRISD6 = 1; //POWER
    TRISDbits.TRISD7 = 1; //VOL+
    TRISAbits.TRISA7 = 1; //VOL-
    TRISDbits.TRISD13 = 1; //SELECT
    
    int isOn = 0;
    int isOn1 = 0;
    int isOn2 = 0;
    int isOn3 = 0;
    setup();
    
    while(1)
    {
        
        if( PORTDbits.RD6 == 0 )
        {
            delay_us(10);
            if( PORTDbits.RD6 == 0 )
            {
                if(isOn  == 0)
                {
                PORTA=0x0001;
                isOn = 1;
                transmit(0x20DF10EF);
                }
            }

        }
         else
        {
            PORTA = 0x00F0;
            isOn = 0;
        }
        if(PORTDbits.RD7 == 0)
        {
            delay_us(10);
            if(PORTDbits.RD7 == 0)
            {
                if(isOn1  == 0)
                {
                    PORTA=0x0002;
                isOn1 = 1;
                transmit(0x20DF40BF);
                }
            }
        }
         else
        {
            PORTA = 0x00F0;
            isOn1 = 0;
        }
        if(PORTDbits.RD13 == 0)
        {
            delay_us(10);
            if(PORTDbits.RD13 == 0)
            {
                if(isOn2  == 0)
                {
                    PORTA=0x0002;
                isOn2 = 1;
                transmit(0x20DFD02F);
                }
            }
        }
         else
        {
            PORTA = 0x00F0;
            isOn2 = 0;
        }
        if( PORTAbits.RA7 == 0 )
        {
            delay_us(10);
            if( PORTAbits.RA7 == 0 )
            {
                if(isOn3  == 0)
                {
                PORTA=0x0001;
                isOn3 = 1;
                transmit(0x20DFC03F);
                }
            }

        }
         else
        {
            PORTA = 0x00F0;
            isOn3 = 0;
        }
    }
}

void delay_us(int time)
{
    TMR2 = 0;
    T2CONbits.TON = 1;
    int delaycycle = 4*time;
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

  T3CONbits.TON = 1;//Act T3CONbits.TON = 1ivate the PWM module
  delay_us(time);
}

inline void space(int time) {

  T3CONbits.TON = 0;//Deactivate the PWM module
  delay_us(time);
}

void sendByte(unsigned long b) {

    int i;
    for (i = 0; i < 32; i++)
  {
      if(b & 0x80000000)
      {
      // one
      mark (BIT1_M);
      space(BIT1_S);
      }
      else{
      // zero
      mark (BIT0_M);
      space(BIT0_S);
      }
      b <<= 1;
  }
}
 /*
1.Xung phat voi f=38khz trong 0.5ms(set 529)
2.Khoang thoi gian giua 2 xung bit 1: 1.7ms
                               bit 0: 0.65ms
3.ham sendByte :lay bit cuoi de transmit
4.ham delay : delay
 */
void transmit(unsigned long b) {
 // header
 mark (HEADER_M);
 space(HEADER_S);

 sendByte(b);

 // stop
 mark (PTRAIL);

 space(39804); //off timer3,delay 20ms
 mark(8985);   //on timer3,dalay 4.5ms
 space(2294);  //off timer3,delay 1ms

 // stop
 mark (PTRAIL);
 space(0);
}



