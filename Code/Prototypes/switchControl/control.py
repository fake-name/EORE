
import time
import serial
import binascii
import random

def _BV(bit):
	return 1 << bit

POWER_BIT     = _BV(0)
SWITCH_CONTROL_BIT_1 = _BV(2)
SWITCH_CONTROL_BIT_2 = _BV(1)
SWITCH_CONTROL_BIT_3 = _BV(3)

ATTEN_CTRL_CLK = _BV(2)
ATTEN_CTRL_DAT = _BV(3)
ATTEN_CTRL_LE  = _BV(1)
SWITCH_ADDR_MASK     = SWITCH_CONTROL_BIT_1 | SWITCH_CONTROL_BIT_2 | SWITCH_CONTROL_BIT_3

SWITCH_SHUTDOWN      = 0x07
# 0x07 puts the switch in "shutdown" state. I /assume/ this means all
# the switches are open, but it's not definitively documented in the datasheet.

READ_CMD  = "\x25"
WRITE_CMD = "\xAA"
HEADER    = "\x5D"

class SwitchControl(object):

	switch_power = False
	switch_addr  = 0

	def __init__(self, portName):
		self.port = serial.Serial(portName, 115200)


	def calculateChecksum(self, instr):
		cksum = 0
		for char in instr:
			cksum += ord(char)

		return chr(cksum & 0xFF)

	def readSwitch(self):
		pkt = "%s%s\x00" % (HEADER, READ_CMD)
		pkt += self.calculateChecksum(pkt)
		self.port.write(pkt)
		rx = self.exhaust()
		# print("RX Packet len", len(pkt), binascii.hexlify(pkt))
		if rx:
			# print("Received response!")
			print(rx)



	def __writeSwitch(self, value):

		if not (value & POWER_BIT) and (value & SWITCH_ADDR_MASK):
			raise ValueError("You cannot set any of the switch control bits high when the switch is off")

		if value > 0xFF or value < 0:
			raise ValueError("Switch value must be a 8-bit unsigned value.")

		pkt = "%s%s%s" % (HEADER, WRITE_CMD, chr(value))
		pkt += self.calculateChecksum(pkt)
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


	# switchChannel implicitly turns on the switch.
	def switchChannel(self, chan):
		if chan > 0x07 or chan < 0x00 or chan == 0x06:
			raise ValueError("Invalid switch channel '%s'!" % chan)


		outChan = 0

		if chan & _BV(2):
			outChan = outChan | SWITCH_CONTROL_BIT_1

		if chan & _BV(1):
			outChan = outChan | SWITCH_CONTROL_BIT_2

		if chan & _BV(0):
			outChan = outChan | SWITCH_CONTROL_BIT_3

		dat = (outChan & SWITCH_ADDR_MASK) | POWER_BIT
		self.__writeSwitch(dat)

	def switchPower(self, powerOn):
		if powerOn:
			self.switchChannel(self.switch_addr)
		else:
			self.__writeSwitch(0x00)



class AttenControl(object):

	switch_power = False
	nop_val  = 0

	def __init__(self, portName):
		self.port = serial.Serial(portName, 115200)


	def calculateChecksum(self, instr):
		cksum = 0
		for char in instr:
			cksum += ord(char)

		return chr(cksum & 0xFF)

	def readSwitch(self):
		pkt = "%s%s\x00" % (HEADER, READ_CMD)
		pkt += self.calculateChecksum(pkt)
		self.port.write(pkt)
		rx = self.exhaust()
		# print("RX Packet len", len(pkt), binascii.hexlify(pkt))
		if rx:
			# print("Received response!")
			print(rx)



	def __writePins(self, value):

		if not (value & POWER_BIT) and (value & SWITCH_ADDR_MASK):
			raise ValueError("You cannot set any of the switch control bits high when the switch is off")

		if value > 0xFF or value < 0:
			raise ValueError("Switch value must be a 8-bit unsigned value.")

		pkt = "%s%s%s" % (HEADER, WRITE_CMD, chr(value))
		pkt += self.calculateChecksum(pkt)
		self.port.write(pkt)
		time.sleep(0.05)
		rx = self.exhaust()
		# if rx:
		# 	print(rx)

	# Read all data in the rx buffer.
	def exhaust(self):
		ret = ""
		while self.port.inWaiting():
			ret += self.port.read()
		return ret.strip()



		self.__writePins(dat)

	# switchChannel implicitly turns on the switch.
	def writeAtten(self, value):
		if value > 31.5 or value < 00:
			raise ValueError("Valid values 31.5 dB to 0 dB. Specified: '%s'!" % value)

		value = int(value*2+0.5)  # Round to nearest value

		for x in range(6):
			val = POWER_BIT

			if value & (0x01 << 5-x):
				val |= ATTEN_CTRL_DAT

			print(value, val, (0x01 << 5-x), val & (0x01 << 5-x))

			self.__writePins(val)
			val |= ATTEN_CTRL_CLK
			self.__writePins(val)


		self.__writePins(POWER_BIT)
		self.__writePins(POWER_BIT | ATTEN_CTRL_LE)

		self.__writePins(POWER_BIT)




def go():
	print("Starting")
	port = AttenControl("COM29")
	time.sleep(2)
	port.port.write("\x5Dasdasdasd")
	print("Port opened, written to")

	x = 0
	port.writeAtten(0)

	while 1:


		try:
			atten = float(x)

			val = port.exhaust()
			# port.readSwitch()
			port.writeAtten(atten)


		except KeyboardInterrupt:
			raise

		time.sleep(1)


		# x = raw_input("Enter value (0-6): ")
		x += 0.5
		if x > 31.5:
			x = 0

if __name__ == "__main__":
	go()
