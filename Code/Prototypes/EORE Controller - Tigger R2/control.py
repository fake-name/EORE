
import time
import serial


# 0x07 puts the switch in "shutdown" state. I /assume/ this means all
# the switches are open, but it's not definitively documented in the datasheet.
HEADER             = "\x5D"

PACKET_HEADER      = "\x5D"
WRITE_ATTEN        = "\xAA"
WRITE_SWITCH       = "\x0C"
WRITE_FREQ         = "\xC0"
WRITE_MISC         = "\xDD"


MAIN_ANTENNA        = 0x00
SWITCHED_TONE_INPUT = 0x01
EMI_ANTENNA         = 0x02
AUX_TONE_INPUT      = 0x03
TERMINATION         = 0x04
NOISE_SOURCE        = 0x05

SWITCH_SHUTDOWN    = 0x07


MAIN_TONE_ATTEN       = 0x00

AUX_TONE_ATTEN        = 0x01   #
NOISE_DIODE_ATTEN     = 0x02   #
SWITCH_SWR_TONE_ATTEN = 0x03   #
SWITCH_TONE_ATTEN     = 0x04   #
MID_AMP_ATTEN         = 0x05   #



#define WRITE_FREQ    0xC0

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
		if chan > 0x07 or chan < 0x00 or chan == 0x06:
			raise ValueError("Invalid switch channel '%s'!" % chan)

		if switchNo != 0:
			raise ValueError("Only switch 0 is supported at this time.")

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
		pass

	# Convenience method
	def disableOscillator(self):
		self.enableOscillator(False)



def go():
	print("Starting")
	port = EoreController("COM40")
	time.sleep(2)
	port.port.write("\x5Dasdasdasd")
	print("Port opened, written to")

	x = 0
	port.writeAtten(0, 0)
	port.writeAtten(1, 0)
	port.writeAtten(2, 0)
	port.writeAtten(3, 0)
	port.writeAtten(4, 0)
	port.writeAtten(5, 0)
	port.writeAtten(MID_AMP_ATTEN, 15)
	port.writeAtten(AUX_TONE_INPUT, 30)

	vals = [0,1,2,3,4,5]

	port.writeSwitch(0, 4)

	# port.writeAtten(2, 31.5)
	port.noiseDiodePowerCtl(1)
	port.oscPowerCtl()

	port.writeOscillator(0, 50e6)
	while 1:


		# try:
		# 	atten = float(x)

		# 	val = port.exhaust()
		# 	# port.readSwitch()
		# 	port.writeAtten(0, atten)
		# 	# port.writeAtten(1, atten)
		# 	port.writeAtten(2, atten)
		# 	# port.writeAtten(0, 31.5-atten)


		# except KeyboardInterrupt:
		# 	raise


		# # port.writeOscillator(0, 100e6)
		# port.writeSwitch(0, NOISE_SOURCE)
		# time.sleep(5)

		# port.writeSwitch(0, EMI_ANTENNA)
		# time.sleep(5)

		# x = 0
		while 1:
			x = (x + 1) % len(vals)
			port.writeSwitch(0, vals[x])
			# port.writeOscillator(0, 50e6+(10e6*vals[x]))

			port.writeOscillator(0, 50e6)
			time.sleep(1)

			port.writeOscillator(0, 0)
			time.sleep(1)

			port.noiseDiodePowerCtl(1)
			time.sleep(1)

			port.noiseDiodePowerCtl(0)
			time.sleep(1)

		# port.writeAtten(0, 0)
		# time.sleep(1)

		# port.writeAtten(0, 0)
		# time.sleep(1)


		# # x = raw_input("Enter value (0-6): ")
		# x += 0.5
		# if x > 31.5:
		# 	x = 0
		# x = raw_input("Enter value (0-6): ")

		# try:
		# 	x = int(x)
		# 	port.writeSwitch(0, x)
		# except ValueError:
		# 	print("Wat?")
		# 	pass
		# except TypeError:
		# 	print("Wat?")
		# 	pass

		# x += 1
		# if x > 7:
		# 	x = 0

if __name__ == "__main__":
	go()
