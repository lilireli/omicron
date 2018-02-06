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
 
/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/
static void vBlinkLedGreen(void* pvParameters); 
static void vBlinkLedRed(void* pvParameters);
static void vBlinkLedYellow(void* pvParameters);
 
/******************************************************************************
 * Public function definitions.
 ******************************************************************************/
xQueueHandle GlobalBlinkingQueue = 0;
xSemaphoreHandle gatekeeper = 0; 

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

    GlobalBlinkingQueue = xQueueCreate(5, sizeof(int));
    gatekeeper = xSemaphoreCreateMutex();


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
static void vBlinkLedGreen(void* pvParameters)
{
    DDRD |= _BV(PD5); // sets pin 5 to write
 
    for ( ;; )
    {
        if (xSemaphoreTake(gatekeeper, 2000))
        {
            PORTD ^= _BV(PD5); // sets pin 5 to reverse value it was before
            vTaskDelay(1000);
            xQueueSend(GlobalBlinkingQueue, 1, 1000);
            xQueueSend(GlobalBlinkingQueue, 2, 1000);
            xQueueSend(GlobalBlinkingQueue, 3, 1000);
            vTaskDelay(4000);

            PORTD ^= _BV(PD5);
            xSemaphoreGive(gatekeeper);
        }

        vTaskDelay(500);

    }
}

static void vBlinkLedRed(void* pvParameters)
{
    int rx_int = 0;
    DDRD |= _BV(PD4);
 
    for ( ;; )
    {
        while (xQueueReceive(GlobalBlinkingQueue, &rx_int, 500))
        {
            PORTD ^= _BV(PD4);
            vTaskDelay(200);
            PORTD ^= _BV(PD4);
            vTaskDelay(200);
        }
    }
}

static void vBlinkLedYellow(void* pvParameters)
{
    DDRD |= _BV(PD3);
 
    for ( ;; )
    {
        if (xSemaphoreTake(gatekeeper, 4000))
        {
            PORTD ^= _BV(PD3);
            vTaskDelay(1000);
            PORTD ^= _BV(PD3);
            xSemaphoreGive(gatekeeper);
        }

        vTaskDelay(500);
    }
}