
import time
import serial


SWITCH_SHUTDOWN      = 0x07
# 0x07 puts the switch in "shutdown" state. I /assume/ this means all
# the switches are open, but it's not definitively documented in the datasheet.

HEADER    = "\x5D"

PACKET_HEADER = "\x5D"
WRITE_ATTEN_1 = "\xAA"
WRITE_ATTEN_2 = "\xAB"
WRITE_ATTEN_3 = "\xAC"
WRITE_SWITCH  = "\x0C"
WRITE_FREQ    = "\xC0"

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

		self.__sendCommand(WRITE_SWITCH, chan)



	def __sendCommand(self, cmd, value):


		pkt = "%s%s" % (HEADER, cmd)
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

		if atten == 0:
			self.__sendCommand(WRITE_ATTEN_1, value)
		elif atten == 1:
			self.__sendCommand(WRITE_ATTEN_2, value)
		elif atten == 2:
			self.__sendCommand(WRITE_ATTEN_3, value)
		else:
			raise ValueError("Valid attenuators are 0-2")


	# Set oscillator frequency
	def writeOscillator(self, osc, freq):


		if osc != 0:
			raise ValueError("Only oscillator number 0 is supported at this time.")

		print("Writing oscillator", freq)
		freq = int(freq)
		if freq < 10e6 or freq > 810e6:
			raise ValueError("Frequency %s is not valid. Valid available frequencies are 10 Mhz - 810 Mhz." % freq)

		self.__sendCommand(WRITE_FREQ, freq)


	# Needs hardware support, not currently supported
	def enableOscillator(self, powState=True):
		pass

	# Convenience method
	def disableOscillator(self):
		self.enableOscillator(False)



def go():
	print("Starting")
	port = EoreController("COM4")
	time.sleep(2)
	port.port.write("\x5Dasdasdasd")
	print("Port opened, written to")

	x = 0
	port.writeAtten(0, 0)
	port.writeAtten(1, 0)
	port.writeAtten(2, 0)

	vals = [0,1,2,3,4,5]

	port.writeSwitch(0, vals[0])

	port.writeAtten(2, 31.5)


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


		x = 0
		while 1:
			x = (x + 1) % len(vals)
			port.writeSwitch(0, vals[x])
			port.writeOscillator(0, 50e6+(10e6*vals[x]))
			time.sleep(1)

		port.writeAtten(0, 0)
		time.sleep(1)

		port.writeAtten(0, 0)
		time.sleep(1)


		# x = raw_input("Enter value (0-6): ")
		x += 0.5
		if x > 31.5:
			x = 0
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
