#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "RCSwitch.h"

static const char *TAG = "MAIN";

#if CONFIG_RF_RECEIVER
void receiver(void* pvParameter)
{
	ESP_LOGI(TAG, "Start receiver");
	RCSWITCH_t RCSwitch;
	initSwich(&RCSwitch);
	//enableReceive(&RCSwitch, 22);
	ESP_ERROR_CHECK(enableReceive(&RCSwitch, CONFIG_RF_GPIO));

	while(1) {
		if (available(&RCSwitch)) {
			ESP_LOGI(TAG, "Received %lu / %dbit Protocol: %d", 
				getReceivedValue(&RCSwitch), getReceivedBitlength(&RCSwitch), getReceivedProtocol(&RCSwitch));
			resetAvailable(&RCSwitch);
		} else {
			vTaskDelay(1);
		}
	} // end while
}
#endif // CONFIG_RF_RECEIVER

#if CONFIG_RF_TRANSMITTER
void transmitter(void* pvParameter)
{
	ESP_LOGI(TAG, "Start transmitter");
	RCSWITCH_t RCSwitch;
	initSwich(&RCSwitch);
	enableTransmit(&RCSwitch, CONFIG_RF_GPIO);
	ESP_LOGI(TAG, "CONFIG_RF_PROTOCOL=%d", CONFIG_RF_PROTOCOL);
	setProtocol(&RCSwitch, CONFIG_RF_PROTOCOL);
	ESP_LOGI(TAG, "CONFIG_RF_REPEAT=%d", CONFIG_RF_REPEAT);
	setRepeatTransmit(&RCSwitch, CONFIG_RF_REPEAT);
	ESP_LOGI(TAG, "CONFIG_RF_LENGTH=%d", CONFIG_RF_LENGTH);

	uint32_t sendValue = 1;
	while(1) {
#if 0
		sendTriState(&RCSwitch, getCodeWordA("11001", "01000", true));
		vTaskDelay(pdMS_TO_TICKS(1000));
		sendTriState(&RCSwitch, getCodeWordA("11001", "01000", false));
		vTaskDelay(pdMS_TO_TICKS(1000));
#endif

#if 0
		sendTriState(&RCSwitch, getCodeWordB(4, 2, true));
		vTaskDelay(pdMS_TO_TICKS(1000));
		sendTriState(&RCSwitch, getCodeWordB(4, 2, false));
		vTaskDelay(pdMS_TO_TICKS(1000));
#endif

#if 1
		ESP_LOGI(TAG, "Sending value is 0x%"PRIx32, sendValue);
		//sendCode(&RCSwitch, sendValue, 32);
		sendCode(&RCSwitch, sendValue, CONFIG_RF_LENGTH);
		if (sendValue == ULONG_MAX) {
			sendValue = 1;
		} else {
			sendValue++;
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
#endif
	} // end while
}
#endif // CONFIG_RF_TRANSMITTER

void app_main()
{
#if CONFIG_RF_RECEIVER
	xTaskCreate(&receiver, "RECV", 1024*4, NULL, 2, NULL);
#endif

#if CONFIG_RF_TRANSMITTER
	xTaskCreate(&transmitter, "SEND", 1024*4, NULL, 2, NULL);
#endif
}
