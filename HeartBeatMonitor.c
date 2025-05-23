#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "I2C_LCD.h"
#include <pic16f877a.h>

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ     4000000
#define ADC_MIN_VALUE   0
#define ADC_MAX_VALUE   1023
#define HR_MIN          60
#define HR_MAX          150
#define SAMPLES         10

void ADC_Init() {
    ADCON1 = 0x8E;
    ADCON0 = 0x41;
    __delay_us(20);
}

int ADC_Read(unsigned char channel) {
    if (channel > 7) {
        return 0;
    }
    
    ADCON0 = (ADCON0 & 0xC7) | (channel << 3);
    __delay_us(20);
    
    ADCON0bits.GO_DONE = 1;
    while (ADCON0bits.GO_DONE);
    
    return ((ADRESH << 8) + ADRESL);
}

int map(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int detect_heart_rate() {
    int readings[SAMPLES];
    int sum = 0;
    int avg, threshold;
    int pulse_count = 0;
    int last_state = 0;
    int current_state;
    
    for (int i = 0; i < SAMPLES; i++) {
        readings[i] = ADC_Read(0);
        sum += readings[i];
        __delay_ms(50);
    }
    
    avg = sum / SAMPLES;
    threshold = avg + 50;
    
    for (int i = 0; i < 60; i++) {
        int current_reading = ADC_Read(0);
        current_state = (current_reading > threshold) ? 1 : 0;
        
        if (current_state == 1 && last_state == 0) {
            pulse_count++;
        }
        
        last_state = current_state;
        __delay_ms(50);
    }
    
    return pulse_count * 20;
}

void main() {
    char display_text[16];
    int heart_rate;
    
    TRISB = 0x00;
    TRISA = 0xFF;
    PORTB = 0x00;
    
    I2C_Master_Init();
    LCD_Init(0x4E);
    ADC_Init();
    
    LCD_Set_Cursor(1, 1);
    LCD_Write_String("Heart Rate Mon.");
    LCD_Set_Cursor(2, 1);
    LCD_Write_String("Initializing...");
    __delay_ms(2000);
    LCD_Clear();
    
    while(1) {
        PORTBbits.RB7 = 1;
        __delay_ms(100);
        PORTBbits.RB7 = 0;
        __delay_ms(100);
        
        LCD_Set_Cursor(1, 1);
        LCD_Write_String("Measuring...    ");
        LCD_Set_Cursor(2, 1);
        LCD_Write_String("Please wait     ");
        
        heart_rate = detect_heart_rate();
        
        if (heart_rate < HR_MIN) {
            heart_rate = HR_MIN;
        } else if (heart_rate > HR_MAX) {
            heart_rate = HR_MAX;
        }
        
        sprintf(display_text, "BPM: %d        ", heart_rate);
        LCD_Set_Cursor(1, 1);
        LCD_Write_String(display_text);
        
        if (heart_rate >= 60 && heart_rate <= 100) {
            LCD_Set_Cursor(2, 1);
            LCD_Write_String("Normal          ");
        } else if (heart_rate > 100) {
            LCD_Set_Cursor(2, 1);
            LCD_Write_String("High            ");
        } else {
            LCD_Set_Cursor(2, 1);
            LCD_Write_String("Low             ");
        }
        
        __delay_ms(2000);
    }
}
