#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "RCSwitch.h"

#define TAG "MAIN"

void receiver(void* pvParameter)
{
	ESP_LOGI(TAG, "Start receiver");
	RCSWITCH_t RCSwitch;
	initSwich(&RCSwitch);
	//enableReceive(&RCSwitch, 22);
	enableReceive(&RCSwitch, CONFIG_RF_GPIO);

	while(1) {
		if (available(&RCSwitch)) {
			ESP_LOGI(TAG, "Received %lu / %dbit Protocol: %d", 
			getReceivedValue(&RCSwitch), getReceivedBitlength(&RCSwitch), getReceivedProtocol(&RCSwitch));
			resetAvailable(&RCSwitch);
		} else {
			vTaskDelay(1);
		}
	}
}

void transmitter(void* pvParameter)
{
	ESP_LOGI(TAG, "Start transmitter");
	RCSWITCH_t RCSwitch;
	initSwich(&RCSwitch);
	enableTransmit(&RCSwitch, CONFIG_RF_GPIO);
	ESP_LOGI(TAG, "CONFIG_RF_PROTOCOL=%d", CONFIG_RF_PROTOCOL);
	setProtocol(&RCSwitch, CONFIG_RF_PROTOCOL);
	ESP_LOGI(TAG, "CONFIG_RF_LENGTH=%d", CONFIG_RF_LENGTH);

	uint32_t sendValue = 1;
	while(1) {
		ESP_LOGI(TAG, "Sending value is %u", sendValue);
		//send(&RCSwitch, sendValue, 32);
		send(&RCSwitch, sendValue, CONFIG_RF_LENGTH);
		sendValue++;
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void app_main()
{
#if CONFIG_RF_RECEIVER
	xTaskCreate(&receiver, "RECV", 1024*4, NULL, 2, NULL);
#endif

#if CONFIG_RF_TRANSMITTER
	xTaskCreate(&transmitter, "SEND", 1024*4, NULL, 2, NULL);
#endif
}