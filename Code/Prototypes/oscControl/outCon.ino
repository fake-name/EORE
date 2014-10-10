

// Definition of interrupt names
#include < avr/io.h >
// ISR interrupt service routine
#include < avr/interrupt.h >

#define SI570_DEBUG

#include <Wire.h>
#include "Si570.h"

Si570 *vfo;
// Arduino crap
#include "Arduino.h"

void loop()
{

}

void setup()
{
	Serial.begin(115200);


	Serial.println("Device init...");
	vfo = new Si570(SI570_I2C_ADDRESS, 56320000);

	if (vfo->status == SI570_ERROR) {
		// The Si570 is unreachable. Show an error for 3 seconds and continue.
		Serial.println("Si570 comm error");
		delay(3000);
	}


	// This will print some debugging info to the serial console.
	vfo->debugSi570();


	#define DELAY_TIME 200



	vfo->setFrequency(100E6);

	while (1)

	{
		// for (long x = 25000000; x < 500000000; x += 1000000)
		// {
		// 	Serial.print("Tuning to ");
		// 	Serial.println(x);

		// 	vfo->setFrequency(x);
		// 	delay(50);

		// }


		// vfo->setFrequency(99.5E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(99.75E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(100.25E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(100.5E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(100.25E6);
		// delay(DELAY_TIME);

		// vfo->setFrequency(100E6);
		// delay(DELAY_TIME);
		// vfo->setFrequency(99.75E6);
		// delay(DELAY_TIME);

	}
}