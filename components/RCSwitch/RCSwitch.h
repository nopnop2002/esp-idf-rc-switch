#ifndef MAIN_RCSWITCH_H_
#define MAIN_RCSWITCH_H_

#include <stdint.h>
#include <stdbool.h>

#define	LOW  0
#define HIGH 1

	/**
	 * Description of a single pulse, which consists of a high signal
	 * whose duration is "high" times the base pulse length, followed
	 * by a low signal lasting "low" times the base pulse length.
	 * Thus, the pulse overall lasts (high+low)*pulseLength
	 */
	typedef struct HighLow {
		uint8_t high;
		uint8_t low;
	} HighLow;

	/**
	 * A "protocol" describes how zero and one bits are encoded into high/low
	 * pulses.
	 */
	typedef struct Protocol {
		/** base pulse length in microseconds, e.g. 350 */
		uint16_t pulseLength;

		HighLow syncFactor;
		HighLow zero;
		HighLow one;

		/**
		 * If true, interchange high and low logic levels in all transmissions.
		 *
		 * By default, RCSwitch assumes that any signals it sends or receives
		 * can be broken down into pulses which start with a high signal level,
		 * followed by a a low signal level. This is e.g. the case for the
		 * popular PT 2260 encoder chip, and thus many switches out there.
		 *
		 * But some devices do it the other way around, and start with a low
		 * signal level, followed by a high signal level, e.g. the HT6P20B. To
		 * accommodate this, one can set invertedSignal to true, which causes
		 * RCSwitch to change how it interprets any HighLow struct FOO: It will
		 * then assume transmissions start with a low signal lasting
		 * FOO.high*pulseLength microseconds, followed by a high signal lasting
		 * FOO.low*pulseLength microseconds.
		 */
		bool invertedSignal;
	} Protocol;

// Number of maximum high/Low changes per packet.
// We can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync
#define RCSWITCH_MAX_CHANGES 67

typedef struct {
	unsigned long nReceivedValue;
	unsigned int nReceivedBitlength;
	unsigned int nReceivedDelay;
	unsigned int nReceivedProtocol;
	int nReceiveTolerance;
	unsigned nSeparationLimit;
	/*
	 * timings[0] contains sync timing, followed by a number of bits
	 */
	unsigned int timings[RCSWITCH_MAX_CHANGES];
	int nReceiverInterrupt;

	int nTransmitterPin;
	int nRepeatTransmit;

	Protocol protocol;
} RCSWITCH_t;


	void initSwich(RCSWITCH_t * RCSwitch);
#if 0
	void switchOn(int nGroupNumber, int nSwitchNumber);
	void switchOff(int nGroupNumber, int nSwitchNumber);
	void switchOn(const char* sGroup, int nSwitchNumber);
	void switchOff(const char* sGroup, int nSwitchNumber);
	void switchOn(char sFamily, int nGroup, int nDevice);
	void switchOff(char sFamily, int nGroup, int nDevice);
	void switchOn(const char* sGroup, const char* sDevice);
	void switchOff(const char* sGroup, const char* sDevice);
	void switchOn(char sGroup, int nDevice);
	void switchOff(char sGroup, int nDevice);

	void sendTriState(const char* sCodeWord);
	void send(unsigned long code, unsigned int length);
	void send(const char* sCodeWord);
#endif

	esp_err_t enableReceive(RCSWITCH_t * RCSwitch, int interrupt);
	esp_err_t enableReceiveInternal(RCSWITCH_t * RCSwitch);
	void disableReceive(RCSWITCH_t * RCSwitch);
	bool available(RCSWITCH_t * RCSwitch);
	void resetAvailable(RCSWITCH_t * RCSwitch);

	unsigned long getReceivedValue(RCSWITCH_t * RCSwitch);
	unsigned int getReceivedBitlength(RCSWITCH_t * RCSwitch);
	unsigned int getReceivedDelay(RCSWITCH_t * RCSwitch);
	unsigned int getReceivedProtocol(RCSWITCH_t * RCSwitch);
	unsigned int* getReceivedRawdata(RCSWITCH_t * RCSwitch);

	void handleInterrupt(void* arg);

	void setProtocol(RCSWITCH_t * RCSwitch, int nProtocol);
	void setProtocolPulseLength(RCSWITCH_t * RCSwitch, int nProtocol, int nPulseLength);
	void setPulseLength(RCSWITCH_t * RCSwitch, int nPulseLength);
	void setRepeatTransmit(RCSWITCH_t * RCSwitch, int nRepeatTransmit);
	void setReceiveTolerance(RCSWITCH_t * RCSwitch, int nPercent);

	void enableTransmit(RCSWITCH_t * RCSwitch, int nTransmitterPin);
	void disableTransmit(RCSWITCH_t * RCSwitch);

	char* getCodeWordA(const char* sGroup, const char* sDevice, bool bStatus);
	char* getCodeWordB(int nAddressCode, int nChannelCode, bool bStatus);
	char* getCodeWordC(char sFamily, int nGroup, int nDevice, bool bStatus);
	char* getCodeWordD(char group, int nDevice, bool bStatus);
	void sendTriState(RCSWITCH_t * RCSwitch, const char* sCodeWord);
	void sendCode(RCSWITCH_t * RCSwitch, unsigned long code, unsigned int length);

	void transmit(RCSWITCH_t * RCSwitch, HighLow pulses);

#endif /* MAIN_RXB6_RECEIVER_H_ */
