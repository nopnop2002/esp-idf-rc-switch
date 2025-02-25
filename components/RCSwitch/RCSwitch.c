/*
	RCSwitch - Arduino libary for remote control outlet switches
	Copyright (c) 2011 Suat Ozgur.	All right reserved.
	
	Contributors:
	- Andre Koehler / info(at)tomate-online(dot)de
	- Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
	- Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
	- Dominik Fischer / dom_fischer(at)web(dot)de
	- Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com
	- Andreas Steinel / A.<lastname>(at)gmail(dot)com
	- Max Horn / max(at)quendi(dot)de
	- Robert ter Vehn / <first name>.<last name>(at)gmail(dot)com
	- Johann Richard / <first name>.<last name>(at)gmail(dot)com
	- Vlad Gheorghe / <first name>.<last name>(at)gmail(dot)com https://github.com/vgheo
	- Matias Cuenca-Acuna 
	
	Project home: https://github.com/sui77/rc-switch/
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
	Lesser General Public License for more details.
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h" // for esp-idf v5

#include "RCSwitch.h"

static const char *TAG = "RF433";

static const Protocol proto[] = {
  { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false },    // protocol 1
  { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false },    // protocol 2
  { 100, { 30, 71 }, {  4, 11 }, {  9,  6 }, false },    // protocol 3
  { 380, {  1,  6 }, {  1,  3 }, {  3,  1 }, false },    // protocol 4
  { 500, {  6, 14 }, {  1,  2 }, {  2,  1 }, false },    // protocol 5
  { 450, { 23,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 6 (HT6P20B)
  { 150, {  2, 62 }, {  1,  6 }, {  6,  1 }, false },    // protocol 7 (HS2303-PT, i. e. used in AUKEY Remote)
  { 200, {  3, 130}, {  7, 16 }, {  3,  16}, false},     // protocol 8 Conrad RS-200 RX
  { 200, { 130, 7 }, {  16, 7 }, { 16,  3 }, true},      // protocol 9 Conrad RS-200 TX
  { 365, { 18,  1 }, {  3,  1 }, {  1,  3 }, true },     // protocol 10 (1ByOne Doorbell)
  { 270, { 36,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 11 (HT12E)
  { 320, { 36,  1 }, {  1,  2 }, {  2,  1 }, true }      // protocol 12 (SM5212)
};

enum {
	 numProto = sizeof(proto) / sizeof(proto[0])
};

void initSwich(RCSWITCH_t * RCSwitch) {
	RCSwitch->nReceivedValue = 0;
	RCSwitch->nReceivedBitlength = 0;
	RCSwitch->nReceivedDelay = 0;
	RCSwitch->nReceivedProtocol = 0;
	RCSwitch->nReceiveTolerance = 60;
	RCSwitch->nSeparationLimit = 4300;

	RCSwitch->nTransmitterPin = -1;
	setRepeatTransmit(RCSwitch, 10);
	setProtocol(RCSwitch, 1);
	RCSwitch->nReceiverInterrupt = -1;
	setReceiveTolerance(RCSwitch, 60);
	RCSwitch->nReceivedValue = 0;
}

/**
	* Sets the protocol to send, from a list of predefined protocols
	*/
void setProtocol(RCSWITCH_t * RCSwitch, int nProtocol) {
	if (nProtocol < 1 || nProtocol > numProto) {
	nProtocol = 1;	// TODO: trigger an error, e.g. "bad protocol" ???
	}
	RCSwitch->protocol = proto[nProtocol-1];
}

/**
	* Sets the protocol to send with pulse length in microseconds.
	*/
void setProtocolPulseLength(RCSWITCH_t * RCSwitch, int nProtocol, int nPulseLength) {
	setProtocol(RCSwitch, nProtocol);
	setPulseLength(RCSwitch, nPulseLength);
}

/**
	* Sets pulse length in microseconds
	*/
void setPulseLength(RCSWITCH_t * RCSwitch, int nPulseLength) {
	RCSwitch->protocol.pulseLength = nPulseLength;
}

/**
 * Sets Repeat Transmits
 */
void setRepeatTransmit(RCSWITCH_t * RCSwitch, int nRepeatTransmit) {
	RCSwitch->nRepeatTransmit = nRepeatTransmit;
}

/**
 * Set Receiving Tolerance
 */
void setReceiveTolerance(RCSWITCH_t * RCSwitch, int nPercent) {
	RCSwitch->nReceiveTolerance = nPercent;
}

/**
 * Enable transmissions
 *
 * @param nTransmitterPin Arduino Pin to which the sender is connected to
 */
void enableTransmit(RCSWITCH_t * RCSwitch, int nTransmitterPin) {
	RCSwitch->nTransmitterPin = nTransmitterPin;
	//pinMode(this->nTransmitterPin, OUTPUT);
	//gpio_pad_select_gpio( RCSwitch->nTransmitterPin );
	gpio_reset_pin( RCSwitch->nTransmitterPin );
	gpio_set_direction( RCSwitch->nTransmitterPin, GPIO_MODE_OUTPUT );
}

/**
	* Disable transmissions
	*/
void disableTransmit(RCSWITCH_t * RCSwitch) {
	RCSwitch->nTransmitterPin = -1;
}

/**
 * Returns a char[13], representing the code word to be send.
 *
 */
char* getCodeWordA(const char* sGroup, const char* sDevice, bool bStatus) {
	static char sReturn[13];
	int nReturnPos = 0;

	for (int i = 0; i < 5; i++) {
		sReturn[nReturnPos++] = (sGroup[i] == '0') ? 'F' : '0';
	}

	for (int i = 0; i < 5; i++) {
		sReturn[nReturnPos++] = (sDevice[i] == '0') ? 'F' : '0';
	}

	sReturn[nReturnPos++] = bStatus ? '0' : 'F';
	sReturn[nReturnPos++] = bStatus ? 'F' : '0';

	sReturn[nReturnPos] = '\0';
	return sReturn;
}

/**
 * Encoding for type B switches with two rotary/sliding switches.
 *
 * The code word is a tristate word and with following bit pattern:
 *
 * +-----------------------------+-----------------------------+----------+------------+
 * | 4 bits address							 | 4 bits address							 | 3 bits		| 1 bit			 |
 * | switch group								 | switch number							 | not used | on / off	 |
 * | 1=0FFF 2=F0FF 3=FF0F 4=FFF0 | 1=0FFF 2=F0FF 3=FF0F 4=FFF0 | FFF			| on=F off=0 |
 * +-----------------------------+-----------------------------+----------+------------+
 *
 * @param nAddressCode	Number of the switch group (1..4)
 * @param nChannelCode	Number of the switch itself (1..4)
 * @param bStatus				Whether to switch on (true) or off (false)
 *
 * @return char[13], representing a tristate code word of length 12
 */
char* getCodeWordB(int nAddressCode, int nChannelCode, bool bStatus) {
	static char sReturn[13];
	int nReturnPos = 0;

	if (nAddressCode < 1 || nAddressCode > 4 || nChannelCode < 1 || nChannelCode > 4) {
		return 0;
	}

	for (int i = 1; i <= 4; i++) {
		sReturn[nReturnPos++] = (nAddressCode == i) ? '0' : 'F';
	}

	for (int i = 1; i <= 4; i++) {
		sReturn[nReturnPos++] = (nChannelCode == i) ? '0' : 'F';
	}

	sReturn[nReturnPos++] = 'F';
	sReturn[nReturnPos++] = 'F';
	sReturn[nReturnPos++] = 'F';

	sReturn[nReturnPos++] = bStatus ? 'F' : '0';

	sReturn[nReturnPos] = '\0';
	return sReturn;
}

/**
 * Like getCodeWord (Type C = Intertechno)
 */
char* getCodeWordC(char sFamily, int nGroup, int nDevice, bool bStatus) {
	static char sReturn[13];
	int nReturnPos = 0;

	int nFamily = (int)sFamily - 'a';
	if ( nFamily < 0 || nFamily > 15 || nGroup < 1 || nGroup > 4 || nDevice < 1 || nDevice > 4) {
		return 0;
	}
	
	// encode the family into four bits
	sReturn[nReturnPos++] = (nFamily & 1) ? 'F' : '0';
	sReturn[nReturnPos++] = (nFamily & 2) ? 'F' : '0';
	sReturn[nReturnPos++] = (nFamily & 4) ? 'F' : '0';
	sReturn[nReturnPos++] = (nFamily & 8) ? 'F' : '0';

	// encode the device and group
	sReturn[nReturnPos++] = ((nDevice-1) & 1) ? 'F' : '0';
	sReturn[nReturnPos++] = ((nDevice-1) & 2) ? 'F' : '0';
	sReturn[nReturnPos++] = ((nGroup-1) & 1) ? 'F' : '0';
	sReturn[nReturnPos++] = ((nGroup-1) & 2) ? 'F' : '0';

	// encode the status code
	sReturn[nReturnPos++] = '0';
	sReturn[nReturnPos++] = 'F';
	sReturn[nReturnPos++] = 'F';
	sReturn[nReturnPos++] = bStatus ? 'F' : '0';

	sReturn[nReturnPos] = '\0';
	return sReturn;
}

/**
 * Encoding for the REV Switch Type
 *
 * The code word is a tristate word and with following bit pattern:
 *
 * +-----------------------------+-------------------+----------+--------------+
 * | 4 bits address							 | 3 bits address		 | 3 bits		| 2 bits			 |
 * | switch group								 | device number		 | not used | on / off		 |
 * | A=1FFF B=F1FF C=FF1F D=FFF1 | 1=0FF 2=F0F 3=FF0 | 000			| on=10 off=01 |
 * +-----------------------------+-------------------+----------+--------------+
 *
 * Source: http://www.the-intruder.net/funksteckdosen-von-rev-uber-arduino-ansteuern/
 *
 * @param sGroup				Name of the switch group (A..D, resp. a..d) 
 * @param nDevice				Number of the switch itself (1..3)
 * @param bStatus				Whether to switch on (true) or off (false)
 *
 * @return char[13], representing a tristate code word of length 12
 */
char* getCodeWordD(char sGroup, int nDevice, bool bStatus) {
	static char sReturn[13];
	int nReturnPos = 0;

	// sGroup must be one of the letters in "abcdABCD"
	int nGroup = (sGroup >= 'a') ? (int)sGroup - 'a' : (int)sGroup - 'A';
	if ( nGroup < 0 || nGroup > 3 || nDevice < 1 || nDevice > 3) {
		return 0;
	}

	for (int i = 0; i < 4; i++) {
		sReturn[nReturnPos++] = (nGroup == i) ? '1' : 'F';
	}

	for (int i = 1; i <= 3; i++) {
		sReturn[nReturnPos++] = (nDevice == i) ? '1' : 'F';
	}

	sReturn[nReturnPos++] = '0';
	sReturn[nReturnPos++] = '0';
	sReturn[nReturnPos++] = '0';

	sReturn[nReturnPos++] = bStatus ? '1' : '0';
	sReturn[nReturnPos++] = bStatus ? '0' : '1';

	sReturn[nReturnPos] = '\0';
	return sReturn;
}

/**
 * @param sCodeWord		a tristate code word consisting of the letter 0, 1, F
 */
void sendTriState(RCSWITCH_t * RCSwitch, const char* sCodeWord) {
	// turn the tristate code word into the corresponding bit pattern, then send it
	unsigned long code = 0;
	unsigned int length = 0;
	for (const char* p = sCodeWord; *p; p++) {
		code <<= 2L;
		switch (*p) {
			case '0':
				// bit pattern 00
				break;
			case 'F':
				// bit pattern 01
				code |= 1L;
				break;
			case '1':
				// bit pattern 11
				code |= 3L;
				break;
		}
		length += 2;
	}
	sendCode(RCSwitch, code, length);
}

/**
 * Transmit the first 'length' bits of the integer 'code'. The
 * bits are sent from MSB to LSB, i.e., first the bit at position length-1,
 * then the bit at position length-2, and so on, till finally the bit at position 0.
 */
void sendCode(RCSWITCH_t * RCSwitch, unsigned long code, unsigned int length) {
	ESP_LOGD(TAG, "RCSwitch->nTransmitterPin=%d", RCSwitch->nTransmitterPin);
	if (RCSwitch->nTransmitterPin == -1)
		return;

	// make sure the receiver is disabled while we transmit
	ESP_LOGD(TAG, "RCSwitch->nReceiverInterrupt=%d", RCSwitch->nReceiverInterrupt);
	int nReceiverInterrupt_backup = RCSwitch->nReceiverInterrupt;
	if (nReceiverInterrupt_backup != -1) {
		disableReceive(RCSwitch);
	}

	ESP_LOGD(TAG, "RCSwitch->nRepeatTransmit=%d", RCSwitch->nRepeatTransmit);
	for (int nRepeat = 0; nRepeat < RCSwitch->nRepeatTransmit; nRepeat++) {
		for (int i = length-1; i >= 0; i--) {
			if (code & (1L << i))
				transmit(RCSwitch, RCSwitch->protocol.one);
			else
				transmit(RCSwitch, RCSwitch->protocol.zero);
		}
		transmit(RCSwitch, RCSwitch->protocol.syncFactor);
	}

	// Disable transmit after sending (i.e., for inverted protocols)
	//digitalWrite(this->nTransmitterPin, LOW);
	gpio_set_level( RCSwitch->nTransmitterPin, LOW );

	// enable receiver again if we just disabled it
	if (nReceiverInterrupt_backup != -1) {
		enableReceive(RCSwitch, nReceiverInterrupt_backup);
	}
}


/**
 * Transmit a single high-low pulse.
 */
void transmit(RCSWITCH_t * RCSwitch, HighLow pulses) {
	uint8_t firstLogicLevel = (RCSwitch->protocol.invertedSignal) ? LOW : HIGH;
	uint8_t secondLogicLevel = (RCSwitch->protocol.invertedSignal) ? HIGH : LOW;
	
#if 0
	digitalWrite(this->nTransmitterPin, firstLogicLevel);
	delayMicroseconds( this->protocol.pulseLength * pulses.high);
	digitalWrite(this->nTransmitterPin, secondLogicLevel);
	delayMicroseconds( this->protocol.pulseLength * pulses.low);
#endif
	gpio_set_level(RCSwitch->nTransmitterPin, firstLogicLevel );
	//ets_delay_us(RCSwitch->protocol.pulseLength * pulses.high);
	esp_rom_delay_us(RCSwitch->protocol.pulseLength * pulses.high);
	gpio_set_level(RCSwitch->nTransmitterPin, secondLogicLevel);
	//ets_delay_us(RCSwitch->protocol.pulseLength * pulses.low);
	esp_rom_delay_us(RCSwitch->protocol.pulseLength * pulses.low);
}


/**
 * Enable receiving data
 */
esp_err_t enableReceive(RCSWITCH_t * RCSwitch, int interrupt) {
	RCSwitch->nReceiverInterrupt = interrupt;
	return (enableReceiveInternal(RCSwitch));
}

#define ESP_INTR_FLAG_DEFAULT 0


esp_err_t enableReceiveInternal(RCSWITCH_t * RCSwitch) {
	uint64_t gpio_pin_sel = (1ULL<<RCSwitch->nReceiverInterrupt);
	ESP_LOGI(TAG, "RCSwitch->nReceiverInterrupt=%d gpio_pin_sel=%llu", RCSwitch->nReceiverInterrupt, gpio_pin_sel);

	// Configure the data input
	gpio_config_t io_conf = {
		.intr_type = GPIO_INTR_ANYEDGE,
		.mode = GPIO_MODE_INPUT,
		.pin_bit_mask = gpio_pin_sel,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE
	};
	gpio_config(&io_conf);

	//install gpio isr service
	esp_err_t err = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	ESP_LOGI(TAG, "gpio_install_isr_service=%d", err);
	//hook isr handler for specific gpio pin
	err = gpio_isr_handler_add(RCSwitch->nReceiverInterrupt, handleInterrupt, RCSwitch);
	ESP_LOGI(TAG, "gpio_isr_handler_add=%d", err);
	return err;
}

/**
 * Disable receiving data
 */
void disableReceive(RCSWITCH_t * RCSwitch) {
	//detachInterrupt(this->nReceiverInterrupt);
	//gpio_isr_handler_remove(GPIO_NUM_22);
	gpio_isr_handler_remove(RCSwitch->nReceiverInterrupt); // nReceiverInterrupt is GPIO
	RCSwitch->nReceiverInterrupt = -1;
}

bool available(RCSWITCH_t * RCSwitch) {
	return RCSwitch->nReceivedValue != 0;
}

void resetAvailable(RCSWITCH_t * RCSwitch) {
	RCSwitch->nReceivedValue = 0;
}

unsigned long getReceivedValue(RCSWITCH_t * RCSwitch) {
	return RCSwitch->nReceivedValue;
}

unsigned int getReceivedBitlength(RCSWITCH_t * RCSwitch) {
	return RCSwitch->nReceivedBitlength;
}

unsigned int getReceivedDelay(RCSWITCH_t * RCSwitch) {
	return RCSwitch->nReceivedDelay;
}

unsigned int getReceivedProtocol(RCSWITCH_t * RCSwitch) {
	return RCSwitch->nReceivedProtocol;
}

unsigned int* getReceivedRawdata(RCSWITCH_t * RCSwitch) {
	return RCSwitch->timings;
}

/* helper function for the receiveProtocol method */
static inline unsigned int diff(int A, int B) {
	return abs(A - B);
}

bool receiveProtocol(RCSWITCH_t * RCSwitch, const int p, unsigned int changeCount) {
	const Protocol pro = proto[p-1];

	unsigned long code = 0;
	//Assuming the longer pulse length is the pulse captured in timings[0]
	const unsigned int syncLengthInPulses =  ((pro.syncFactor.low) > (pro.syncFactor.high)) ? (pro.syncFactor.low) : (pro.syncFactor.high);
	const unsigned int delay = RCSwitch->timings[0] / syncLengthInPulses;
	const unsigned int delayTolerance = delay * RCSwitch->nReceiveTolerance / 100;

	/* For protocols that start low, the sync period looks like
	 *							 _________
	 * _____________|					|XXXXXXXXXXXX|
	 *
	 * |--1st dur--|-2nd dur-|-Start data-|
	 *
	 * The 3rd saved duration starts the data.
	 *
	 * For protocols that start high, the sync period looks like
	 *
	 *	______________
	 * |							|____________|XXXXXXXXXXXXX|
	 *
	 * |-filtered out-|--1st dur--|--Start data--|
	 *
	 * The 2nd saved duration starts the data
	 */
	const unsigned int firstDataTiming = (pro.invertedSignal) ? (2) : (1);

	for (unsigned int i = firstDataTiming; i < changeCount - 1; i += 2) {
		code <<= 1;
		if (diff(RCSwitch->timings[i], delay * pro.zero.high) < delayTolerance &&
			diff(RCSwitch->timings[i + 1], delay * pro.zero.low) < delayTolerance) {
			// zero
		} else if (diff(RCSwitch->timings[i], delay * pro.one.high) < delayTolerance &&
					diff(RCSwitch->timings[i + 1], delay * pro.one.low) < delayTolerance) {
			// one
			code |= 1;
		} else {
			// Failed
			return false;
		}
	}

	if (changeCount > 7) { // ignore very short transmissions: no device sends them, so this must be noise
		RCSwitch->nReceivedValue = code;
		RCSwitch->nReceivedBitlength = (changeCount - 1) / 2;
		RCSwitch->nReceivedDelay = delay;
		RCSwitch->nReceivedProtocol = p;
		return true;
	}

	return false;
}


void handleInterrupt(void* arg)
{
	RCSWITCH_t *RCSwitch = (RCSWITCH_t *) arg;

	static unsigned int changeCount = 0;
	static unsigned long lastTime = 0;
	static unsigned int repeatCount = 0;

	const long time = esp_timer_get_time();
	const unsigned int duration = time - lastTime;

	if (duration > RCSwitch->nSeparationLimit) {
		// A long stretch without signal level change occurred. This could
		// be the gap between two transmission.
		if (diff(duration, RCSwitch->timings[0]) < 200) {
			// This long signal is close in length to the long signal which
			// started the previously recorded timings; this suggests that
			// it may indeed by a a gap between two transmissions (we assume
			// here that a sender will send the signal multiple times,
			// with roughly the same gap between them).
			repeatCount++;
			if (repeatCount == 2) {
				for(uint8_t i = 1; i <= numProto; i++) {
					if (receiveProtocol(RCSwitch, i, changeCount)) {
						// receive succeeded for protocol i
						break;
					}
				}
				repeatCount = 0;
			}
		}
		changeCount = 0;
	}
	// detect overflow
	if (changeCount >= RCSWITCH_MAX_CHANGES) {
		changeCount = 0;
		repeatCount = 0;
	}

	RCSwitch->timings[changeCount++] = duration;
	lastTime = time;
}
