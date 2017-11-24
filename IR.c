/* 
 * File:   newmain.c
 * Author: HOANG DUY
 *
 * Created on October 5, 2017, 10:00 PM
 */
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


void delay_ms(int t)
{
    T2CON = 0x8000;
    TMR2 = 0;
    PR2 = 4000;//4000/4000000
    while (t--)
    {
        while (TMR2 < PR2);
        TMR2 = 0;
    }
}


void initTimer3(void)
{
    // TIMER = 1/128s // 4*10^6(lenh/s) * ((1/128)s / 1(lenh/chu ky)) = 31250 lan/chu ky
    T3CON = 0x30;            //Stop Timer and reset Controller Register, set clock Fosc/2 bit 1 , set ??ng b? clock ngoài bit 2
    TMR3 = 0x00;               //Timer counter
    //PR3 = 7813;           //Timer period 
    IPC2bits.T3IP = 0b010;   //Priority
    IFS0bits.T3IF = 0;      //Flag
    IEC0bits.T3IE = 1;      //Enable interrupt
    //T3CONbits.TON = 1;      //Start Timer
}
int dem = 0;
void __attribute__((__interrupt__,auto_psv))_T3Interrupt (void)
{
    
    if (dem == 10)
    {
        OC1CON = 0x0008;
        TMR2 = 0;
        PR2 = 1;
        IPC1bits.T2IP = 0b011;   //Priority
        IFS0bits.T2IF = 0;      //Flag
        IEC0bits.T2IE = 1;  
        dem = 0;
        T3CONbits.TON = 0;
    }
    else
    {
        dem++;
    }
    IFS0bits.T3IF = 0;
    
    
}
void __attribute__((__interrupt__,auto_psv))_T2Interrupt (void)
{
    IFS0bits.T2IF = 0;       
}

void initTimer1(void)
{
    // TIMER = 1/128s // 4*10^6(lenh/s) * ((1/128)s / 1(lenh/chu ky)) = 31250 lan/chu ky
    T1CON = 0x30;            //Stop Timer and reset Controller Register, set clock Fosc/2 bit 1 , set ??ng b? clock ngoài bit 2
    TMR1 = 0x00;               //Timer counter
    PR1 = 4000;           //Timer period 
    IPC0bits.T1IP = 0x001;   //Priority
    IFS0bits.T1IF = 0;      //Flag
    IEC0bits.T1IE = 1;      //Enable interrupt
   
}

void __attribute__((__interrupt__,auto_psv))_T1Interrupt (void)
{
    PORTA = 0x00FF;
    delay_ms(20);
    T3CONbits.TON = 0;
    OC1CONbits.OCM = 0b000;
    T1CONbits.TON = 0;
    IFS0bits.T1IF = 0;
       
}

void initPWM(void)
{
    T3CON = 0x8000;
    TMR3 = 0;
    PR3 = 104; //tan so 38 Khz   
    //init PWM
    //activate the PWM module
    OC1CON = 0x000E; //1110
    IPC2bits.T3IP = 0b100; //set ?u tien muc 4
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
}




main(void)
{
    TRISA = 0xFF00;
    TRISDbits.TRISD13 = 1;
    initTimer3();
    initTimer1();
    INTCON1bits.NSTDIS = 0;
    int codeword[4]={1,0,0,1};
    int change;
    PORTA = 0x00FF;
   while(1) 
   {             
       //nh?n nút nh?n ngoài cùng ?? phát xung
       
       if( PORTDbits.RD13 == 0)            
        {      
            delay_ms(5);
            PORTA=0x000F;
            T1CONbits.TON = 1;      //Start Timer1
            //Duyet mang codeword
            int i = 0;
            while( i < 4)
            {
                //lua chon che do 0|1 theo gia tri da luu trong mang
                change = codeword[i];
                    switch (change)
                        {
                        case 0:
                            initPWM();
                            //Giu o che do phat xung 0 den khi du 50 chu ki T2IF = 1 ?ánh d?u k?t thúc thoát kh?i ch? ?? 0
                            while(1)
                            {
                                if(IFS0bits.T2IF == 1)
                                {
                                    break;
                                }
                            }
                            break;
                        case 1:
                            initPWM();
                            OC1R = 0;
                            OC1RS = 52;
                            while(1)
                            {
                                if(IFS0bits.T2IF == 1)
                                {
                                    break;
                                }
                            }
                            break;
                        }
                    i++;
            }
        }
        else
        {
            PORTA = 0x00F0;
            
        }    
   }                  
}