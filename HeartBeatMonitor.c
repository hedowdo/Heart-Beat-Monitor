#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "I2C_LCD.h"
#include <pic16f877a.h>

#define _XTAL_FREQ     4000000

#define VCFG0 ADCON1bits.VCFG0
#define VCFG1 ADCON1bits.VCFG1

#define ADC_MIN_VALUE   0
#define ADC_MAX_VALUE   1023
#define HR_MIN          60
#define HR_MAX          150

void ADC_Init() {
    ADCON1 = 0x80;
    ADCON0bits.ADCS = 0b10;
    ADCON1bits.ADFM = 0;
    ADCON0bits.ADON = 1;
}

int ADC_Read(unsigned char channel) {
    if (channel > 7) {
        return 0;
    }
    ADCON0 = (channel << 2) | 0x01;
    __delay_us(20);
    GO_DONE = 1;
    while (GO_DONE);
    return ((ADRESH << 8) + ADRESL);
}

int map(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void main() 
{
    char heart1[15];
    int heart;
    TRISB =0x00;
    TRISA =0xff;
    
    I2C_Master_Init();
    LCD_Init(0x4E);
    ADC_Init();
    
    while(1) 
    {
        RB7=1;
        __delay_ms(200);
        RB7=0;
        __delay_ms(200);
    
        heart = ADC_Read(0);
        
        int bpm = map(heart, ADC_MIN_VALUE, ADC_MAX_VALUE, HR_MIN, HR_MAX);

        sprintf(heart1, "BPM: %d", bpm);

        LCD_Set_Cursor(1, 1);
        LCD_Write_String(heart1);
        
        __delay_ms(3000);

    }
}





