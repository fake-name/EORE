
import time
import serial

HEADER    = "\x5D"
WRITE_CMD = "\xC0"

class OscControl(object):


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

	# Write frequency value {value} out the serial port in the proper command structure
	def __writeOscillator(self, value):

		pkt = "%s%s" % (HEADER, WRITE_CMD)
		for dummy_x in range(4):
			pkt += chr(value & 0xFF)
			value >>= 8

		cksum = self.__calculateChecksum(pkt)
		pkt += cksum
		print("Checksum", ord(cksum))
		self.port.write(pkt)
		time.sleep(0.05)
		rx = self.exhaust()
		if rx:
			print("Hardware Response: '%s'" % rx)

	# Read all data in the rx buffer. Return it as a string with leading and trailing whitespace/newlines stripped
	def exhaust(self):
		ret = ""
		while self.port.inWaiting():
			ret += self.port.read()
		return ret.strip()


	# Set oscillator frequency
	def writeOscillator(self, freq):
		print("Writing oscillator", freq)
		freq = int(freq)
		if freq < 10e6 or freq > 810e6:
			raise ValueError("Frequency %s is not valid. Valid available frequencies are 10 Mhz - 810 Mhz." % freq)

		self.__writeOscillator(freq)


	# Needs hardware support, not currently supported
	def enableOscillator(self, powState=True):
		pass

	# Convenience method
	def disableOscillator(self):
		self.enableOscillator(False)



def go():
	print("Starting")
	port = OscControl("COM29")
	# port.port.write("\x5Dasdasdasd")
	print("Port opened, written to")
	print(port.exhaust())
	time.sleep(2)
	print(port.exhaust())

	print("Writing oscillator")
	port.writeOscillator(50e6)
	print("Written")

	while 1:

		port.writeOscillator(50e6)
		time.sleep(1)
		port.writeOscillator(60e6)
		time.sleep(1)
		port.writeOscillator(70e6)
		time.sleep(1)


if __name__ == "__main__":
	go()
