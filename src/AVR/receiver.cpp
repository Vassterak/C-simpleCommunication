#include "myFunctions.h"

#define DATA_TRANSFER_PIN 2 //pin number in register D

//for first bit indicating incomming packet (define tolerances for signal duration)
#define START_TOLERANCE 490 //us
#define END_TOLERANCE 510 //us
#define DELAY_BEFORE_READ 2000 //us
#define DELAY_BEFORE_FIRST_VALUE 500 //us
#define DELAY_BETWEEN_READS 1000 //us

static uint8_t dataPacket = 0;

uint16_t readIncommingPacket()
{
	uint16_t outputData = 0;
	_delay_us(DELAY_BEFORE_FIRST_VALUE);

	for (uint8_t i = 0; i < 12; i++)
	{
		if (readValueAVR(DATA_TRANSFER_PIN) == 1)
		{
			_delay_us(DELAY_BETWEEN_READS);
			if (readValueAVR(DATA_TRANSFER_PIN) == 0)
			{
				outputData |= 0 << i;
				_delay_us(DELAY_BETWEEN_READS);
			}
		}

		else
		{
			_delay_us(DELAY_BETWEEN_READS);
			if (readValueAVR(DATA_TRANSFER_PIN) == 1)
			{
				outputData |= 1 << i;
				_delay_us(DELAY_BETWEEN_READS);
			}
		}
	}	
	return outputData;
}

uint8_t integrityCheck(uint16_t receivedPacket)
{
	uint8_t data = (receivedPacket & 0xFF);
	uint8_t checksum = (receivedPacket >> 8);
	uint8_t numberOfOnesData = 0, dataOneBitsNum = data;

    while (dataOneBitsNum > 0)
	{
        if (dataOneBitsNum & 1)
            numberOfOnesData++;
		
        dataOneBitsNum = dataOneBitsNum >> 1;
    }

	if (numberOfOnesData == checksum)
	{
		dataPacket = data;
		return 0;
	}
	else
		return 1;
}

int main (void)
{
	//AVR timing  
	unsigned long timeShift, timeShift2;

	//pinout setup AVR
	setup(DATA_TRANSFER_PIN, 0);

	//Serialline setup AVR
	init();
	Serial.begin(9600);

	while (1)
	{
		//Check if value on port is high
   		if (readValueAVR(DATA_TRANSFER_PIN) == 1)
		{
			//get time of first signal rise (get cpu time in microseconds)
			timeShift = micros();

			//Do nothing while port value is high
			while (readValueAVR(DATA_TRANSFER_PIN) == 1)
				;
			
			//get time of signal fall change 1 -> 0 (get cpu time in microseconds)
			timeShift2 = micros();

			//Check if data transmission started (each packet has starting value, used for sync)
			if (timeShift2 - timeShift <= END_TOLERANCE && timeShift2 - timeShift > START_TOLERANCE)
			{
				_delay_us(DELAY_BEFORE_READ);
				if (integrityCheck(readIncommingPacket()) == 0 && dataPacket < 127)
					Serial.println((char)dataPacket);
				else
					Serial.println((char)63);
			}
			else
				continue;
		}
	}
	return 0;
}
