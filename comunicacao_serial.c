#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h" // Biblioteca para manejar as máquinas de estado e usar a matriz de LEDs 5x5
#include "hardware/uart.h" // Biblioteca para comunicação via UART
#include "hardware/irq.h" // Biblioteca para lidar com as interrupções dos botões
#include "hardware/i2c.h" // Biblioteca para comunicação I2C
#include "inc/ssd1306.h" // Definições do display e funções necessárias para usá-lo
#include "inc/font.h"// Onde estão os caracteres armazenados para mostrar no display
#include "inc/led_matriz.h"// Onde estão os caracteres armazenados para mostrar no display
#include "pico/bootrom.h" // Biblioteca para reiniciar o dispositivo

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2812.pio.h"

// Definição do número de LEDs e pinos.
#define MATRIZ_LED_PIN 7  // Pino da matriz de LEDs 5x5
#define BUTTON_A 5  // Pino do botão A
#define BUTTON_B 6  // Pino do botão B
#define BUTTON_JOYSTICK 22 // Pino do botão Joystick
#define LED_BLUE_PIN 12  // Pino do LED azul
#define LED_GREEN_PIN 11  // Pino do LED verde
#define DEBOUNCE_DELAY_MS 350  // Tempo de debounce em milissegundos

// Define que uart utilizaremos e sua taxa de transmissão
#define PORTA_UART uart0  // Definição da UART a ser utilizada (uart0)
#define BAUD_RATE 115200  // Taxa de transmissão da UART
// Define os pinos de transmissão e recepção da uart
#define UART_TX_PIN 0  // Pino de transmissão UART (TX)
#define UART_RX_PIN 1  // Pino de recepção UART (RX)

// Definições de canal I2C, endereço do display e GPIO usadas para SDA e SCL
#define I2C_PORT i2c1  // Canal I2C a ser utilizado
#define I2C_SDA 14  // Pino de dados do I2C (SDA)
#define I2C_SCL 15  // Pino de clock do I2C (SCL)
#define endereco 0x3C  // Endereço do display SSD1306

volatile uint32_t ultimo_tempo_buttons = 0;  // Variável para armazenar o tempo da última interrupção acionada pelos botões
volatile bool estado_leds;  // Variável que armazena o estado dos LEDs
ssd1306_t ssd;  // Estrutura do display SSD1306

// Função que inicializa os LEDs e botões, além de configurar os botões com pull-up
void inicializar_leds_e_botoes() {
    // Inicializa o botão A, configura como entrada e ativa o resistor de pull-up
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    // Inicializa o botão B, configura como entrada e ativa o resistor de pull-up
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Inicializa o botão Joystick, configura como entrada e ativa o resistor de pull-up
    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);

    // Inicializa o LED verde, configura como saída e desliga
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, false);

    // Inicializa o LED azul, configura como saída e desliga
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, false);
}

// Função que trata as interrupções dos botões
static void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t tempo_atual = time_us_32() / 1000;  // Obtém o tempo atual em milissegundos
    if (tempo_atual - ultimo_tempo_buttons < DEBOUNCE_DELAY_MS) return;  // Se o tempo passado for menor que o debounce, retorna sem fazer nada
    ultimo_tempo_buttons = tempo_atual;  // Atualiza o tempo da última interrupção

    ssd1306_fill(&ssd, false);  // Limpa o display

    bool estado_leds;

    // Lógica do botão A: Alterna o estado do LED verde e exibe mensagem via UART e I2C
    if (gpio == BUTTON_A) { 
        gpio_put(LED_GREEN_PIN, !gpio_get(LED_GREEN_PIN));  // Alterna o estado do LED verde
        estado_leds = gpio_get(LED_GREEN_PIN);
        printf("O LED verde está: %s\n", estado_leds ? "Ligado" : "Desligado");
        ssd1306_draw_string(&ssd, "Led verde está", 18, 15);  // Exibe a mensagem no display
    }

    // Lógica do botão B: Alterna o estado do LED azul e exibe mensagem via UART e I2C
    if (gpio == BUTTON_B) { 
        gpio_put(LED_BLUE_PIN, !gpio_get(LED_BLUE_PIN));  // Alterna o estado do LED azul
        estado_leds = gpio_get(LED_BLUE_PIN);
        printf("O LED azul está: %s\n", estado_leds ? "Ligado" : "Desligado");
        ssd1306_draw_string(&ssd, "Led azul está", 18, 15);  // Exibe a mensagem no display
    }

    // Lógica do botão Joystick: Exibe mensagem de reset e reinicia o boot
    if (gpio == BUTTON_JOYSTICK) {
        printf("Botão Joystick pressionado. Resetando o boot.\n");
        ssd1306_draw_string(&ssd, "Resetando...", 18, 15);  // Exibe mensagem de reset no display
        ssd1306_send_data(&ssd);  // Envia a mensagem para o display

        reset_usb_boot(0, 0);  // Reinicia o dispositivo
    }

    // Dependendo do estado dos LEDs, exibe "Acesso" ou "Apagado"
    if (estado_leds) {
        ssd1306_draw_string(&ssd, "Acesso", 5, 50);  // Exibe "Acesso" se o LED estiver ligado
    } else {
        ssd1306_draw_string(&ssd, "Apagado", 5, 50);  // Exibe "Apagado" se o LED estiver desligado
    }
    ssd1306_send_data(&ssd);  // Envia os dados para o display
}

int main() {
    stdio_init_all();  // Inicializa a comunicação serial
    // Variáveis e configurações PIO
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);  // Adiciona o programa para controlar a matriz de LEDs
    ws2812_program_init(pio, sm, offset, MATRIZ_LED_PIN, 800000, false);  // Inicializa a matriz de LEDs

    inicializar_leds_e_botoes();  // Inicializa os LEDs e botões
    
    // Registra as interrupções para os botões A, B e Joystick
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    uart_init(PORTA_UART, BAUD_RATE);  // Inicializa a UART com baud rate de 115200
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);  // Configura o pino 0 como TX
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);  // Configura o pino 1 como RX

    // Inicializa a comunicação I2C a 400kHz
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Configura o pino SDA para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);  // Configura o pino SCL para I2C
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);  // Inicializa o display SSD1306
    ssd1306_config(&ssd);  // Configura o display
    ssd1306_fill(&ssd, false);  // Limpa o display
    ssd1306_send_data(&ssd);  // Envia os dados para o display

    // Exibe a mensagem inicial antes do boot
    ssd1306_draw_string(&ssd, "Iniciando                      Sistema", 10, 10);
    ssd1306_send_data(&ssd);  // Envia os dados para o display
    sleep_ms(2000);  // Mantém a mensagem visível por 2 segundos

    ssd1306_fill(&ssd, false);  // Limpa o display antes de entrar no loop principal
    ssd1306_send_data(&ssd);  // Envia os dados para o display

    while (true) {
        if (stdio_usb_connected()) {  // Verifica se o USB está conectado
            char c;  // Caractere que será lido
            if (scanf("%c", &c) == 1) {  // Se um caractere for lido
                ssd1306_fill(&ssd, false);  // Limpa o display
                switch (c) {  // Verifica o caractere lido e exibe o número correspondente na matriz de LEDs
                    case '0':
                        set_one_led(255, 255, 255, numero_0);
                        break;
                    case '1':
                        set_one_led(255, 0, 0, numero_1);
                        break;
                    case '2':
                        set_one_led(255, 255, 255, numero_2);
                        break;
                    case '3':
                        set_one_led(255, 0, 0, numero_3);
                        break;
                    case '4':
                        set_one_led(255, 255, 255, numero_4);
                        break;
                    case '5':
                        set_one_led(255, 0, 0, numero_5);
                        break;
                    case '6':
                        set_one_led(255, 255, 255, numero_6);
                        break;
                    case '7':
                        set_one_led(255, 0, 0, numero_7);
                        break;
                    case '8':
                        set_one_led(255, 255, 255, numero_8);
                        break;
                    case '9':
                        set_one_led(255, 0, 0, numero_9);
                        break;
                    default:
                        break;
                }
                printf("Caractere pressionado: '%c'\n", c);  // Exibe o caractere pressionado via monitor serial
                char str[2] = {c, '\0'};  // Cria uma string com o caractere pressionado
                ssd1306_draw_string(&ssd, "Pressionou: ", 16, 10);  // Exibe a string no display
                ssd1306_draw_string(&ssd, str, 16, 40);  // Exibe o número pressionado no display
                ssd1306_send_data(&ssd);  // Envia os dados para o display
            }
        }
    }

    return 0;  // Fim do programa
}
