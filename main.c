/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
 
#include <avr/io.h>
 
/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/
 
#define mainLED_TASK_PRIORITY   (tskIDLE_PRIORITY)
#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
 
/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/
static void vDetectMotion(void* pvParameters);
static void vBlinkLedGreen(void* pvParameters); 
static void vBlinkLedRed(void* pvParameters);
static void vBlinkLedYellow(void* pvParameters);
 
/******************************************************************************
 * Public function definitions.
 ******************************************************************************/
xQueueHandle blinkingQueue = 0;
xSemaphoreHandle blinkingKeeper = 0;
xSemaphoreHandle doJobSignal = 0;


int main(void)
{
    // Create task.
    // xTaskHandle blink_handle;
    // xTaskCreate
    // (
    //     vBlinkLed,
    //     (signed char*)"blink",
    //     configMINIMAL_STACK_SIZE,
    //     NULL,
    //     mainLED_TASK_PRIORITY,
    //     &blink_handle
    // );

    blinkingQueue = xQueueCreate(5, sizeof(int));
    blinkingKeeper = xSemaphoreCreateMutex();
    vSemaphoreCreateBinary(doJobSignal);

    xTaskCreate(vDetectMotion, (signed char*)"motion", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(vBlinkLedGreen, (signed char*)"blink green", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vBlinkLedRed, (signed char*)"blink red", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vBlinkLedYellow, (signed char*)"blink yellow", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
 
    // Start scheduler.
    vTaskStartScheduler();
 
    return 0;
}
 

void vApplicationIdleHook(void)
{
 
}
 
/******************************************************************************
 * Private function definitions.
 ******************************************************************************/
static void vDetectMotion(void* pvParameters)
{
    bit_set(DDRD, _BV(PD6));  // Pin 6 to write
    bit_clear(DDRB, _BV(PB4)); // Pin 12 to read

    for ( ;; )
    {
        if (bit_get(PINB, _BV(PB4)))
        {
            bit_set(PORTD, _BV(PD6));
            xSemaphoreGive(doJobSignal);
        }
        else
        {
            bit_clear(PORTD, _BV(PD6));
        }

        vTaskDelay(1000);
    }
}

static void vBlinkLedGreen(void* pvParameters)
{
    bit_set(DDRD, _BV(PD5)); // sets pin 5 to write
 
    for ( ;; )
    {
        if (xSemaphoreTake(doJobSignal, 1000))
        {
            if (xSemaphoreTake(blinkingKeeper, 2000))
            {
                bit_set(PORTD, _BV(PD5));
                vTaskDelay(500);
                xQueueSend(blinkingQueue, 1, 500);
                xQueueSend(blinkingQueue, 2, 500);
                xQueueSend(blinkingQueue, 3, 500);
                vTaskDelay(2000);

                bit_clear(PORTD, _BV(PD5));
                xSemaphoreGive(blinkingKeeper);
            }
        }
        

        vTaskDelay(500);

    }
}

static void vBlinkLedRed(void* pvParameters)
{
    int rx_int = 0;
    bit_set(DDRD, _BV(PD4));
 
    for ( ;; )
    {
        while (xQueueReceive(blinkingQueue, &rx_int, 500))
        {
            bit_flip(PORTD, _BV(PD4));
            vTaskDelay(200);
            bit_flip(PORTD, _BV(PD4));
            vTaskDelay(200);
        }
    }
}

static void vBlinkLedYellow(void* pvParameters)
{
    bit_set(DDRD, _BV(PD3));
 
    for ( ;; )
    {
        if (xSemaphoreTake(blinkingKeeper, 4000))
        {
            bit_set(PORTD, _BV(PD3));
            vTaskDelay(1000);
            bit_clear(PORTD, _BV(PD3));
            xSemaphoreGive(blinkingKeeper);
        }

        vTaskDelay(500);
    }
}