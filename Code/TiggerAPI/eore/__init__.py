
# -*- encoding: utf-8 -*-

'''

The EORE system RF control boards provide a single, unified API that allows
configuring of all the hardware functionalities.

Functionally, the entire interface is available through the single class
``EoreController()``, which provides control methods for the various hardware
sections.

Fundamentally, the multiple, functionally disparate component types.
Currently, it has variable attenuators, RF switches, RF Oscillators of several
some varieties, and auxiliary systems (such as thermal stabilization).


Mapping between switch-port/switch/attenuator/oscillator numbers and the
actual hardware is provided by a set of constant variables defined in the
``eore`` module.

'''



import time
import serial


HEADER             = "\x5D"
PACKET_HEADER      = "\x5D"
WRITE_ATTEN        = "\xAA"
WRITE_SWITCH       = "\x0C"
WRITE_FREQ         = "\xC0"
WRITE_MISC         = "\xDD"

WRITE_TEMP         = "\x22"
WRITE_SWEEPER      = "\x66"


#: This is the main, 6 pole switch for the input selector
MAIN_SWITCH        = 0


#: Constant for the 6PST switch - Main input
MAIN_ANTENNA        = 0x00
#: Constant for the 6PST switch - SWR tone pass-through input
SWITCHED_SWEEPER_INPUT = 0x01
#: Constant for the 6PST switch - Local EMI antenna input
EMI_ANTENNA         = 0x02
#: Constant for the 6PST switch - Auxiliary tone/pilot tone input
PILOT_TONE_INPUT      = 0x03
#: Constant for the 6PST switch - On-Board 50Ω Termination
TERMINATION         = 0x04
#: Constant for the 6PST switch - Noise diode input
NOISE_SOURCE        = 0x05

#: Constant for the 6PST switch - Switch disabled (in reflective mode?)
#: 0x07 puts the switch in "shutdown" state. I /assume/ this means all
#: the switches are open, but it's not definitively documented in the datasheet.
SWITCH_SHUTDOWN    = 0x07


#: This is the switch that choses between routing the SWR sweeper through the directional
#: coupler, and directly to the SP6T switch for monitoring uses.
SWR_SWITCH         = 1


#: Variable attenuator selector constants - Main tone attenuator (on tigger board)
MAIN_TONE_ATTEN       = 0x00

#: Variable attenuator selector constants - Secondary tone attenuator (on EORE board)
AUX_TONE_ATTEN        = 0x01   #
#: Variable attenuator selector constants - Noise diode output attenuator
NOISE_DIODE_ATTEN     = 0x02   #
#: Variable attenuator selector constants - Attenuator between SWR switch and directional coupler
SWITCH_SWR_TONE_ATTEN = 0x03   #
#: Variable attenuator selector constants - Attenuator in path between SWR switch and pass-through to 6PST switch
SWITCH_TONE_ATTEN     = 0x04   #
#: Variable attenuator selector constants - Attenuator between the two onboard LNA
MID_AMP_ATTEN         = 0x05   #




class TimeoutError(Exception):
	pass

class CommandError(Exception):
	pass
class UnknownResponseError(Exception):
	pass


class EoreController(object):
	'''
	Base class for interfacing with the EORE Controller PCB (E.g. the "tigger" board)


	'''
	timeout = 1

	_curFreq = 0

	def __init__(self, portName):
		'''
		Create instance of the ``EoreController`` class for the remote controller connected to ``portName``.

		Args:
			portName (string): An ascii string containing the windows com-port name, e.g. ``COM4``, ``COM15``, etc...

		If ``portName`` is not a valid port, a ``serial.serialutil.SerialException`` will be raised.

		'''
		self.port = serial.Serial(portName, 115200)




	def __calculateChecksum(self, instr):
		'''
		Calculate checksum for string {instr}.

		Checksum is the sum of all bytes in the message, & 0xFF.
		this is a very simple checksum, not a CRC or anything.
		For the short packet sizes used here, it's sufficent.
		'''
		cksum = 0
		for char in instr:
			cksum += ord(char)
		return chr(cksum & 0xFF)


	# def readSwitch(self):
	# 	pkt = "%s%s\x00" % (HEADER, READ_CMD)
	# 	pkt += self.calculateChecksum(pkt)
	# 	self.port.write(pkt)
	# 	rx = self._exhaust()
	# 	# print("RX Packet len", len(pkt), binascii.hexlify(pkt))
	# 	if rx:
	# 		# print("Received response!")
	# 		print(rx)


	def __sendCommand(self, cmd, target, value):


		pkt = "%s%s%s" % (HEADER, cmd, chr(target))
		for dummy_x in range(4):
			pkt += chr(value & 0xFF)
			value >>= 8


		pkt += self.__calculateChecksum(pkt)
		self.port.write(pkt)
		time.sleep(0.05)
		rx = self._waitResponse()

		return rx

	# Read all data in the rx buffer.
	def _waitResponse(self):
		ret = ""

		start = time.time()
		while 1:

			while self.port.inWaiting():
				ret += self.port.read()

			if start + self.timeout < time.time():
				raise TimeoutError("Timed out waiting for response from the hardware!")

			if "\n" in ret:
				break

		if 'ERROR' in ret:
			raise CommandError("Response error from command! Response string: '%s'" % ret.strip())
		elif 'OK' in ret:
			return ret.strip()

		raise UnknownResponseError("No known response code in received data: '%s'!" % ret.strip())



	# Read all data in the rx buffer.
	def _exhaust(self):
		ret = ""
		while self.port.inWaiting():
			ret += self.port.read()
		return ret.strip()



	# switchChannel implicitly turns on the switch.
	def writeSwitch(self, switchNo, chan):
		'''
		Send command to set ``switchNo`` to ``chan``.

		Args:
			switchNo (int): One of the switch constants defined in the ``eore`` module, for the switch to control.
			chan (int): One of the channel constants defined in the ``eore`` module for the switch being controlled.
		Returns:
			ASCII text of response from the hardware

		Note: The response string is parsed internally. If the hardware responds with an error (or a unknown response),
		an error will be raised. If there is no raised error, it should be safe to assume that the hardware has responded
		with an 'OK', so checking the response string should not *strictly* be necessary.

		Raises ValueError if the switch number or channel is invalid.

		'''

		# if chan > 0x07 or chan < 0x00 or chan == 0x06:
		# 	raise ValueError("Invalid switch channel '%s'!" % chan)

		# if switchNo < 0 or switchNo > 1:
		# 	raise ValueError("Only switch 0 is supported at this time.")

		return self.__sendCommand(WRITE_SWITCH, switchNo, chan)



	def writeAtten(self, atten, value):
		'''
		Send command to set the attenuator ``atten`` to an attenuation level of ``value``.

		Args:
			atten (int): One of the constants defined in the ``eore`` module for selecting attenuators.
			value (float): The desired attenuation value in dB. The valid range is 0-31.5, with a resolution of 0.5 dB.
		Returns:
			ASCII text of response from the hardware

		Note: The response string is parsed internally. If the hardware responds with an error (or a unknown response),
		an error will be raised. If there is no raised error, it should be safe to assume that the hardware has responded
		with an 'OK', so checking the response string should not *strictly* be necessary.

		Specified values are rounded to the nearest 0.5 dB interval.
		Values outside the 0-31.5 dB range will result in a ``ValueError``.

		If the ``atten`` value is invalid, it will also raise a ``ValueError``

		'''

		if value > 31.5 or value < 00:
			raise ValueError("Valid values 31.5 dB to 0 dB. Specified: '%s'!" % value)

		value = int(value*2+0.5)  # Round to nearest value

		if atten < 7:
			return self.__sendCommand(WRITE_ATTEN, atten, value)
		else:
			raise ValueError("Valid attenuators are 0-7")


	def noiseDiodePowerCtl(self, on=False):
		'''
		Send command to enable/disable the noise-diode power supply.

		Args:
			on (evaluated as a boolean): If ``on`` evaluates as ``True``, the noise diode will be enabled. If ``on`` evaluates as ``False``, it will be turned off.
		Returns:
			ASCII text of response from the hardware

		Note: The response string is parsed internally. If the hardware responds with an error (or a unknown response),
		an error will be raised. If there is no raised error, it should be safe to assume that the hardware has responded
		with an 'OK', so checking the response string should not *strictly* be necessary.

		'''
		if on:
			return self.__sendCommand(WRITE_MISC, 0, 1)
		else:
			return self.__sendCommand(WRITE_MISC, 0, 0)


	def writeOscillator(self, osc, freq):
		'''
		Set oscillator ``osc`` to output frequency ``freq``.

		Args:
			osc (int): Oscillator to control. Should be one of the constants defined in the ``eore`` module.
			freq (int): must be a value between 10e6 and 810e6 (10-810 MHz), or the special-case value of 0, which shuts the oscillator off entirely.
		Returns:
			ASCII text of response from the hardware

		Note: The response string is parsed internally. If the hardware responds with an error (or a unknown response),
		an error will be raised. If there is no raised error, it should be safe to assume that the hardware has responded
		with an 'OK', so checking the response string should not *strictly* be necessary.

		Note that the ``disableOscillator`` call actually just calls this function, with the frequency hard-coded to ``0``.

		Raises ``ValueError`` for frequencies outside the valid range, or invalid oscillator values.

		'''


		if osc != 0:
			raise ValueError("Only oscillator number 0 is supported at this time.")


		freq = int(freq)
		if (freq < 10e6 or freq > 810e6) and freq != 0:
			raise ValueError("Frequency %s is not valid. Valid available frequencies are 10 Mhz - 810 Mhz." % freq)

		return self.__sendCommand(WRITE_FREQ, 0, freq)


	# Convenience method
	def disableOscillator(self, osc=0):
		'''
		Convenience method. Turns off the oscillator ``osc``.

		Args:
			osc (int, default=0): Oscillator to turn off.
		Returns:
			ASCII text of response from the hardware

		Note: The response string is parsed internally. If the hardware responds with an error (or a unknown response),
		an error will be raised. If there is no raised error, it should be safe to assume that the hardware has responded
		with an 'OK', so checking the response string should not *strictly* be necessary.

		To re-enable the oscillator, you call ``writeOscillator`` with a non-zero frequency.
		'''
		return self.writeOscillator(osc, 0)



	def setTemperature(self, temp):
		'''
		Set the thermal stabilization system to a target temperature of ``temp``.

		Args:
			temp (float): Temperature at which to thermally stabilize the RF hardware.
		Returns:
			ASCII text of response from the hardware

		Note: The response string is parsed internally. If the hardware responds with an error (or a unknown response),
		an error will be raised. If there is no raised error, it should be safe to assume that the hardware has responded
		with an 'OK', so checking the response string should not *strictly* be necessary.

		Temp is a float, but the temperature sensor and internal calculations are
		done with fixed-point math with a resolution of 0.0625°C, so adjustments
		finer then this resolution will have no effect.

		Valid temperature range is 0-80°C (though higher temps will work, they may
		damage the hardware).

		A set point of 0°C is a special-case value that will disable the heater output entirely.

		Raises ``ValueError`` for temperatures outside the valid range.

		'''

		if temp > 80:
			raise ValueError("Temperature setpoint too high! Setpoint cannot be above 80°C")
		if temp < 0:
			raise ValueError("Temperature setpoint cannot be below 0.")

		# Communications are done in fixed-point values, where
		# 1 LSB == 0.0625°C
		setpoint = int(temp / 0.0625)

		return self.__sendCommand(WRITE_TEMP, 0, setpoint)


	def getTemperature(self):
		'''
		Read the current temperature of the harware back from the device.

		Args:
			None
		Returns:
			(float): Current temperature

		Returns the current hardware temperature, in degrees Celsius.

		Raises ``ValueError`` if the hardware does not respond in a reasonable
		time.

		'''

		# Command data value is ignored, target == 1 means respond with temp
		return self.__sendCommand(WRITE_TEMP, 1, 0)

	def getTemperatureSetpoint(self):
		'''
		Read the current temperature setpoint of the harware back from the device.

		Args:
			None
		Returns:
			(float): Current temperature setpoint

		Returns the current hardware temperature, in degrees Celsius.

		Raises ``ValueError`` if the hardware does not respond in a reasonable
		time.

		'''


		# Command data value is ignored, target == 1 means respond with setpoint
		return self.__sendCommand(WRITE_TEMP, 2, 0)



	def chirpVco(self, chirps):
		'''
		Trigger a set of VCO chirps from the sweep oscillator.

		Args:
			Number of times to do a up-and-down chirp.
		Returns:
			ASCII text of response from the hardware

		A VCO "Chirp" is a single triangle-like VCO sweep, where the VCO output sweeps from
		~0 Hz to ~500 Mhz, and then back.

		Each chirp takes approximately 120 milliseconds, and there is no way to cancel a set
		of chirps once it is in progress.

		'''

		if chirps < 0 or chirps > 100:
			raise ValueError("Invalid number of chirps. Number must be > 0 and < 100.")

		return self.__sendCommand(WRITE_SWEEPER, 0, chirps)



	def powerDownVco(self):
		'''

		Turn power to the VCO off entirely.

		Args:
			Number of times to do a up-and-down chirp.
		Returns:
			ASCII text of response from the hardware

		If this is called while the VCO output is actively sweeping, it *may* time-out, as
		the system does not pay attention to the serial interface while it's actively sweeping.
		'''

		return self.chirpVco(0)



