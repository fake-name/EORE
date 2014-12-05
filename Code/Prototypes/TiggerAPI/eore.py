
# -*- encoding: utf-8 -*-

import time
import serial


HEADER             = "\x5D"
PACKET_HEADER      = "\x5D"
WRITE_ATTEN        = "\xAA"
WRITE_SWITCH       = "\x0C"
WRITE_FREQ         = "\xC0"
WRITE_MISC         = "\xDD"


#: Constant for the 6PST switch - Main input
MAIN_ANTENNA        = 0x00
#: Constant for the 6PST switch - SWR tone pass-through input
SWITCHED_TONE_INPUT = 0x01
#: Constant for the 6PST switch - Local EMI antenna input
EMI_ANTENNA         = 0x02
#: Constant for the 6PST switch - Auxiliary tone/pilot tone input
AUX_TONE_INPUT      = 0x03
#: Constant for the 6PST switch - On-Board 50Ω Termination
TERMINATION         = 0x04
#: Constant for the 6PST switch - Noise diode input
NOISE_SOURCE        = 0x05

#: Constant for the 6PST switch - Switch disabled (in reflective mode?)
SWITCH_SHUTDOWN    = 0x07
# 0x07 puts the switch in "shutdown" state. I /assume/ this means all
# the switches are open, but it's not definitively documented in the datasheet.


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


class EoreController(object):
	'''
	Base class for interfacing with the EORE Controller PCB (E.g. the "tigger" board)


	'''


	_curFreq = 0

	def __init__(self, portName):
		'''
		Create instance of the ``EoreController`` class for the remote controller connected to ``portName``.

		``portName`` is an ascii string containing the windows com-port name, e.g. ``COM4``, ``COM15``, etc...

		If ``portName`` is not a valid port, a ``serial.serialutil.SerialException`` will be raised.

		'''
		self.port = serial.Serial(portName, 115200)




	# Calculate checksum for string {instr}
	# this is a very simple checksum, not a CRC or anything.
	# For the short packet sizes used here, it's sufficent.
	def __calculateChecksum(self, instr):
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
		rx = self._exhaust()
		if rx:
			print(rx)

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

		``chan`` and ``switchNo`` should be one of the constants defined in the ``eore`` module.

		Raises ValueError if the switch number or channel is invalid.

		'''

		if chan > 0x07 or chan < 0x00 or chan == 0x06:
			raise ValueError("Invalid switch channel '%s'!" % chan)

		if switchNo != 0:
			raise ValueError("Only switch 0 is supported at this time.")

		self.__sendCommand(WRITE_SWITCH, switchNo, chan)



	def writeAtten(self, atten, value):
		'''
		Send command to set the attenuator ``atten`` to an attenuation level of ``value``.

		``atten`` should be one of the constants defined in the ``eore`` module.

		``value`` is the desired attenuation value in dB. The valid range is 0-31.5, with a resolution of 0.5 dB.
		Specified values are rounded to the nearest 0.5 dB interval.
		Values outside the 0-31.5 dB range will result in a ``ValueError``.

		If the ``atten`` value is invalid, it will also raise a ``ValueError``

		'''
		if value > 31.5 or value < 00:
			raise ValueError("Valid values 31.5 dB to 0 dB. Specified: '%s'!" % value)

		value = int(value*2+0.5)  # Round to nearest value

		if atten < 7:
			self.__sendCommand(WRITE_ATTEN, atten, value)
		else:
			raise ValueError("Valid attenuators are 0-7")


	def noiseDiodePowerCtl(self, on=False):
		'''
		Send command to enable/disable the noise-diode power supply.

		Tf ``on`` is ``True``, the noise diode will be enabled. If ``on`` is false, it will be turned off.

		'''
		if on:
			self.__sendCommand(WRITE_MISC, 0, 1)
		else:
			self.__sendCommand(WRITE_MISC, 0, 0)



	# Set oscillator frequency
	def writeOscillator(self, osc, freq):
		'''
		Set oscillator ``osc`` to output frequency ``freq``.

		``freq`` must be a value between 10e6 and 810e6 (10-810 MHz), or the special-case value of 0, which shuts the oscillator
		off entirely.

		Note that the ``disableOscillator`` call actually just calls this function, with the frequency hard-coded to ``0``.

		Raises ``ValueError`` for frequencies outside the valid range, or invalid oscillator values.

		'''


		if osc != 0:
			raise ValueError("Only oscillator number 0 is supported at this time.")

		print("Writing oscillator", freq)
		freq = int(freq)
		if (freq < 10e6 or freq > 810e6) and freq != 0:
			raise ValueError("Frequency %s is not valid. Valid available frequencies are 10 Mhz - 810 Mhz." % freq)

		self.__sendCommand(WRITE_FREQ, 0, freq)


	# Convenience method
	def disableOscillator(self, osc=0):
		'''
		Convenience method. Turns off the oscillator ``osc``.

		To re-enable the oscillator, you call ``writeOscillator`` with a non-zero frequency.
		'''
		self.writeOscillator(osc, 0)
