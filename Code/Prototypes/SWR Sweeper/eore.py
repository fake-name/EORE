
import time
import serial

# Packet structure definitions
HEADER             = "\x5D"
PACKET_HEADER      = "\x5D"
WRITE_ATTEN        = "\xAA"
WRITE_SWITCH       = "\x0C"
WRITE_FREQ         = "\xC0"
WRITE_MISC         = "\xDD"

MAIN_SW  = 0
TONE_SW  = 1

# Convenience definitions for the switch inputs for the SP6T switch ("sw 0")
SW_MAIN_INPUT            = 0
SW_DIRECT_REF_OSC        = 1
SW_EMI_ANTENNA_IN        = 2
SW_PILOT_TONE_IN         = 3
SW_50R_TERMINATION       = 4
SW_NOISE_DIODE           = 5
# 0x07 puts the switch in "shutdown" state. I /assume/ this means all
# the switches are open, but it's not definitively documented in the datasheet.
SWITCH_SHUTDOWN          = 7

# Tone switch ("sw 1") defintions.
SW_TONE_SWR_COUPLER      = 3
SW_TONE_PASS_THROUGH     = 2
SW_TONE_OFF              = 0


# Attenuator definitions
MAIN_TONE_ATTEN       = 0x00   # Atteuator on the oscillator board
AUX_TONE_ATTEN        = 0x01   # Attenuator on the direct tone input
NOISE_DIODE_ATTEN     = 0x02   # Self explanitory
SWITCH_SWR_TONE_ATTEN = 0x03   # Attenuator for the swr coupler on the eore board
SWITCH_TONE_ATTEN     = 0x04   # Attenuator on the switched tone/swr input pass-through
MID_AMP_ATTEN         = 0x05   # attenuator between the amps



class EoreController(object):



	switch_power = False
	nop_val  = 0

	def __init__(self, portName):
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
	# 	rx = self.exhaust()
	# 	# print("RX Packet len", len(pkt), binascii.hexlify(pkt))
	# 	if rx:
	# 		# print("Received response!")
	# 		print(rx)


	# switchChannel implicitly turns on the switch.
	def writeSwitch(self, switchNo, chan):
		if (chan > 0x07 or chan < 0x00 or chan == 0x06) and switchNo == 0:
			raise ValueError("Invalid switch channel '%s' for switch 0!" % chan)

		elif switchNo == 1 and (chan > 3 or chan == 1):
			# for switch 1, the valid channels are 0 (everything off), 2 (swr connection) and 3 (direct pass-through)
			raise ValueError("Invalid switch channel '%s' for switch 1!" % chan)


		elif switchNo > 1:
			raise ValueError("Only switches 0 and 1 are supported at this time. Passed channel '%s'." % chan)

		self.__sendCommand(WRITE_SWITCH, switchNo, chan)


	def __sendCommand(self, cmd, target, value):


		pkt = "%s%s%s" % (HEADER, cmd, chr(target))
		for dummy_x in range(4):
			pkt += chr(value & 0xFF)
			value >>= 8


		pkt += self.__calculateChecksum(pkt)
		self.port.write(pkt)
		time.sleep(0.05)
		rx = self.exhaust()
		if rx:
			print(rx)

	# Read all data in the rx buffer.
	def exhaust(self):
		ret = ""
		while self.port.inWaiting():
			ret += self.port.read()
		return ret.strip()


	def writeAtten(self, atten, value):
		if value > 31.5 or value < 00:
			raise ValueError("Valid values 31.5 dB to 0 dB. Specified: '%s'!" % value)

		value = int(value*2+0.5)  # Round to nearest value

		if atten < 7:
			self.__sendCommand(WRITE_ATTEN, atten, value)
		else:
			raise ValueError("Valid attenuators are 0-7")


	def noiseDiodePowerCtl(self, on=False):
		if on:
			self.__sendCommand(WRITE_MISC, 0, 1)
		else:
			self.__sendCommand(WRITE_MISC, 0, 0)

	def oscPowerCtl(self, on=False):

		if not on:
			self.writeOscillator(0, 0)


	# Set oscillator frequency
	def writeOscillator(self, osc, freq):


		if osc != 0:
			raise ValueError("Only oscillator number 0 is supported at this time.")

		print("Writing oscillator", freq)
		freq = int(freq)
		if (freq < 10e6 or freq > 810e6) and freq != 0:
			raise ValueError("Frequency %s is not valid. Valid available frequencies are 10 Mhz - 810 Mhz." % freq)

		self.__sendCommand(WRITE_FREQ, 0, freq)


	# Needs hardware support, not currently supported
	def enableOscillator(self, powState=True):
		print("NOT SUPPORTED YET")
		pass

	# Convenience method
	def disableOscillator(self):
		self.enableOscillator(False)

def test():

	port = EoreController("COM40")
	port.writeSwitch(1, 3)
	port.writeSwitch(0, 0)
	while 1:

		port.writeOscillator(0, 100e6)
		time.sleep(1)
		port.writeOscillator(0, 105e6)
		time.sleep(1)


if __name__ == "__main__":
	test()
