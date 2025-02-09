# Documentação do Projeto: comunicação_serial.c

## Introdução
Este projeto implementa um sistema de comunicação serial utilizando um microcontrolador Raspberry Pi Pico. O sistema gerencia entradas de botões e exibe informações em um display SSD1306, além de acionar LEDs e interagir via UART.

## Link Video
https://drive.google.com/file/d/1pp_qb_402oehYh2bOTIxyFZGBK8wQzlk/view?usp=sharing

## Definição de Hardware
O projeto utiliza os seguintes pinos:

### LEDs
- **Matriz de LEDs:** Pino 7
- **LED Azul:** Pino 12
- **LED Verde:** Pino 11

### Botões
- **Botão A:** Pino 5
- **Botão B:** Pino 6
- **Botão Joystick:** Pino 22

### UART
- **TX:** Pino 0
- **RX:** Pino 1
- **Baud Rate:** 115200

### I2C
- **SDA:** Pino 14
- **SCL:** Pino 15
- **Endereço do Display SSD1306:** 0x3C

## Funcionalidades

### Inicialização
A função `inicializar_leds_e_botoes()`:
- Configura os botões como entrada e ativa pull-ups.
- Configura LEDs como saída e os desliga inicialmente.

### Manipulação de Interrupção dos Botões
A função `gpio_irq_handler(uint gpio, uint32_t events)`:
- Realiza debounce dos botões.
- Alterna estados dos LEDs e exibe mensagens via UART e display OLED.
- Se o botão Joystick for pressionado, reinicializa o dispositivo.

### Configuração da UART
- A UART é inicializada com a taxa de transmissão de 115200 bps.
- Os pinos 0 e 1 são configurados para transmissão e recepção.

### Configuração do I2C e Display OLED
- O display SSD1306 é inicializado via I2C.
- Exibe mensagens indicando o estado dos LEDs e um aviso de reinicialização.

### Loop Principal
- Aguarda eventos e processa entradas de botões para alterar estados de LEDs e exibir informações.
- Caso haja conexão via USB, pode interagir com comunicação serial.

### Explicação Detalhada do Código

#### Inclusão de Bibliotecas
```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "inc/led_matriz.h"
#include "pico/bootrom.h"
#include "ws2812.pio.h"
```
### Essas bibliotecas são necessárias para diferentes funcionalidades:
- `stdio.h`: Entrada e saída padrão (print, scanf, etc.).
- `pico/stdlib.h`: Funções básicas para Raspberry Pi Pico.
- `hardware/pio.h`: Manipulação de máquinas de estado para controle da matriz de LEDs.
- `hardware/uart.h`: Controle da comunicação via UART.
- `hardware/irq.h`: Controle de interrupções.
- `hardware/i2c.h`: Protocolo I2C para comunicação com dispositivos externos.
- `ssd1306.h`: Controle do display OLED SSD1306.
- `font.h`: Fonte de caracteres para o display.
- `led_matriz.h`: Controle da matriz de LEDs.
- `pico/bootrom.h`: Permite reiniciar o microcontrolador.
- `ws2812.pio.h`: Controle dos LEDs WS2812 (endereçáveis).

#### Definição de Pinos e Configurações
```c
#define MATRIZ_LED_PIN 7  
#define BUTTON_A 5  
#define BUTTON_B 6  
#define BUTTON_JOYSTICK 22
#define LED_BLUE_PIN 12  
#define LED_GREEN_PIN 11  
#define DEBOUNCE_DELAY_MS 350  
```
Define os pinos utilizados para os LEDs e botões, além de um tempo de "debounce" para evitar leitura incorreta ao pressionar botões.

```c
#define PORTA_UART uart0  
#define BAUD_RATE 115200  
#define UART_TX_PIN 0  
#define UART_RX_PIN 1  
```
Configuração da comunicação UART, definindo qual porta será usada e a taxa de transmissão.

```c
#define I2C_PORT i2c1  
#define I2C_SDA 14  
#define I2C_SCL 15  
#define endereco 0x3C  
```
Configuração da comunicação I2C para o display SSD1306.

#### Variáveis Globais
```c
volatile uint32_t ultimo_tempo_buttons = 0;
volatile bool estado_leds;
ssd1306_t ssd;
```
- `ultimo_tempo_buttons`: Armazena o tempo da última interrupção para controle de debounce.
- `estado_leds`: Indica o estado dos LEDs.
- `ssd`: Estrutura para o controle do display OLED SSD1306.

#### Função de Inicialização
```c
void inicializar_leds_e_botoes() {
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    
    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);
    
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, false);
    
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, false);
}
```
Configura os botões como entradas com resistores pull-up e os LEDs como saídas inicialmente desligadas.

#### Função de Tratamento de Interrupções
```c
static void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t tempo_atual = time_us_32() / 1000;
    if (tempo_atual - ultimo_tempo_buttons < DEBOUNCE_DELAY_MS) return;
    ultimo_tempo_buttons = tempo_atual;
    
    ssd1306_fill(&ssd, false);
    
    bool estado_leds;
    if (gpio == BUTTON_A) {
        gpio_put(LED_GREEN_PIN, !gpio_get(LED_GREEN_PIN));
        estado_leds = gpio_get(LED_GREEN_PIN);
        printf("O LED verde está: %s\n", estado_leds ? "Ligado" : "Desligado");
        ssd1306_draw_string(&ssd, "Led verde está", 18, 15);
    }

    if (gpio == BUTTON_B) {
        gpio_put(LED_BLUE_PIN, !gpio_get(LED_BLUE_PIN));
        estado_leds = gpio_get(LED_BLUE_PIN);
        printf("O LED azul está: %s\n", estado_leds ? "Ligado" : "Desligado");
        ssd1306_draw_string(&ssd, "Led azul está", 18, 15);
    }

    if (gpio == BUTTON_JOYSTICK) {
        printf("Botão Joystick pressionado. Resetando o boot.\n");
        ssd1306_draw_string(&ssd, "Resetando...", 18, 15);
        ssd1306_send_data(&ssd);
        reset_usb_boot(0, 0);
    }

    ssd1306_send_data(&ssd);
}
```
Essa função trata os eventos quando um botão é pressionado:
- Alterna o estado dos LEDs.
- Exibe mensagens no display OLED e no terminal UART.
- Se o Joystick for pressionado, reinicia o microcontrolador.

#### Função `main()`
```c
int main() {
    stdio_init_all();
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, MATRIZ_LED_PIN, 800000, false);
    
    inicializar_leds_e_botoes();
    
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    uart_init(PORTA_UART, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    i2c_init(I2C_PORT, 400 * 1000);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    
    while (true) {
        if (stdio_usb_connected()) {
            char c;
            if (scanf("%c", &c) == 1) {
                // Processamento do caractere recebido
            }
        }
    }
}
```
Essa é a função principal que inicializa os dispositivos e entra em um loop infinito aguardando comandos.
