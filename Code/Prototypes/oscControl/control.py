
import time
import serial
import binascii
import random

def _BV(bit):
	return 1 << bit

SWITCH_POWER_BIT     = _BV(0)
SWITCH_CONTROL_BIT_1 = _BV(1)
SWITCH_CONTROL_BIT_2 = _BV(2)
SWITCH_CONTROL_BIT_3 = _BV(3)
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

		if not (value & SWITCH_POWER_BIT) and (value & SWITCH_ADDR_MASK):
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

		dat = (outChan & SWITCH_ADDR_MASK) | SWITCH_POWER_BIT
		self.__writeSwitch(dat)

	def switchPower(self, powerOn):
		if powerOn:
			self.switchChannel(self.switch_addr)
		else:
			self.__writeSwitch(0x00)




def go():
	print("Starting")
	port = SwitchControl("COM29")
	time.sleep(2)
	port.port.write("\x5Dasdasdasd")
	print("Port opened, written to")
	while 1:

		x = raw_input("Enter channel (0-6): ")
		try:
			chan = int(x)
			if chan == 6:
				chan = SWITCH_SHUTDOWN
			val = port.exhaust()
			port.readSwitch()
			port.switchChannel(chan)
		except KeyboardInterrupt:
			raise
		except:
			print("Invalid value")

		time.sleep(1)


if __name__ == "__main__":
	go()
