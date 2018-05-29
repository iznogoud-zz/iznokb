#ifndef __IZNOKB_H__
#define __IZNOKB_H__

#include <libopencm3/stm32/gpio.h>

typedef struct 
{
    unsigned int port;
    unsigned int pin;
} pin_t;

#define NUM_ROWs 6
#define NUM_COLS 16

pin_t rows[NUM_ROWs] = {
    (pin_t){.port=GPIOB, .pin=GPIO12},
    (pin_t){.port=GPIOB, .pin=GPIO13},
    (pin_t){.port=GPIOB, .pin=GPIO14},
    (pin_t){.port=GPIOB, .pin=GPIO15},
    (pin_t){.port=GPIOA, .pin=GPIO8},
    (pin_t){.port=GPIOA, .pin=GPIO8}};

pin_t cols[NUM_COLS] = {
    (pin_t){.port=GPIOB, .pin=GPIO8},
    (pin_t){.port=GPIOB, .pin=GPIO9},
    (pin_t){.port=GPIOC, .pin=GPIO14},
    (pin_t){.port=GPIOC, .pin=GPIO15},
    (pin_t){.port=GPIOA, .pin=GPIO0},
    (pin_t){.port=GPIOA, .pin=GPIO1},
    (pin_t){.port=GPIOA, .pin=GPIO2},
    (pin_t){.port=GPIOA, .pin=GPIO3},
    (pin_t){.port=GPIOA, .pin=GPIO4},
    (pin_t){.port=GPIOA, .pin=GPIO5},
    (pin_t){.port=GPIOA, .pin=GPIO6},
    (pin_t){.port=GPIOA, .pin=GPIO7},
    (pin_t){.port=GPIOB, .pin=GPIO0},
    (pin_t){.port=GPIOB, .pin=GPIO1},
    (pin_t){.port=GPIOB, .pin=GPIO10},
    (pin_t){.port=GPIOB, .pin=GPIO11}};












#endif //__IZNOKB_H__