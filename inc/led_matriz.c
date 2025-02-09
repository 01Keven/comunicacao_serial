#include "led_matriz.h"        // Inclui o cabeçalho para a biblioteca de controle de LEDs
#include "hardware/pio.h"      // Inclui a biblioteca para controlar o PIO (Programmable IO)
#include "ws2812.pio.h"        // Inclui o código específico para controle de LEDs WS2812

#define LED_COUNT 25
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 0
bool numero_0[LED_COUNT] = {
    0, 1, 1, 1, 1, 
    1, 0, 0, 1, 0, 
    0, 1, 0, 0, 1, 
    1, 0, 0, 1, 0, 
    0, 1, 1, 1, 1
};


// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 1
bool numero_1[LED_COUNT] = {
    0, 0, 1, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 1, 0, 1, 
    0, 1, 1, 0, 0, 
    0, 0, 1, 0, 0
};

// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 2
bool numero_2[LED_COUNT] = {
    0, 1, 1, 1, 1, 
    1, 0, 0, 0, 0, 
    0, 1, 1, 1, 1, 
    0, 0, 0, 1, 0, 
    0, 1, 1, 1, 1
};

// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 3
bool numero_3[LED_COUNT] = {
    0, 1, 1, 1, 1, 
    0, 0, 0, 1, 0, 
    0, 1, 1, 1, 1, 
    0, 0, 0, 1, 0, 
    0, 1, 1, 1, 1
};


// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 4
bool numero_4[LED_COUNT] = {
    0, 1, 0, 0, 0, 
    0, 0, 0, 1, 0, 
    0, 1, 1, 1, 1, 
    1, 0, 0, 1, 0, 
    0, 1, 0, 0, 1
};


// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 5
bool numero_5[LED_COUNT] = {
    0, 1, 1, 1, 1, 
    0, 0, 0, 1, 0, 
    0, 1, 1, 1, 1, 
    1, 0, 0, 0, 0, 
    0, 1, 1, 1, 1
};


// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 6
bool numero_6[LED_COUNT] = {
    0, 1, 1, 1, 1, 
    1, 0, 0, 1, 0, 
    0, 1, 1, 1, 1, 
    1, 0, 0, 0, 0, 
    0, 1, 1, 1, 1
};

// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 7
bool numero_7[LED_COUNT] = {
    0, 1, 0, 0, 0, 
    0, 0, 0, 1, 0, 
    0, 1, 0, 0, 1, 
    1, 0, 0, 1, 0, 
    0, 1, 1, 1, 1
};

// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 8
bool numero_8[LED_COUNT] = {
    0, 1, 1, 1, 1, 
    1, 0, 0, 1, 0, 
    0, 1, 1, 1, 1, 
    1, 0, 0, 1, 0, 
    0, 1, 1, 1, 1
};

// Buffer para armazenar quais LEDs estão ligados matriz 5x5 formando numero 9
bool numero_9[LED_COUNT] = {
    0, 1, 0, 0, 0, 
    0, 0, 0, 1, 0, 
    0, 1, 1, 1, 1, 
    1, 0, 0, 1, 0, 
    0, 1, 1, 1, 1
};

void set_one_led(uint8_t r, uint8_t g, uint8_t b, bool numero_a_ser_desenhado[])
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < LED_COUNT; i++)
    {
        if (numero_a_ser_desenhado[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}