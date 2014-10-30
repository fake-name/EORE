
import time

from controlApi import EoreController
import SignalHound as sh
import logSetup



ACQ_SPAN               = 27e6
ACQ_REF_LEVEL_DB       = 20
ACQ_ATTENUATION_DB     = 30
ACQ_GAIN_SETTING       = 0
ACQ_RBW                = 2.465e3
ACQ_VBW                = ACQ_RBW
ACQ_SWEEP_TIME_SECONDS = 0.0100
ACQ_WINDOW_TYPE        = "hamming"
ACQ_UNITS              = "power"
ACQ_TYPE               = "sweeping"
ACQ_MODE               = "average"
ACQ_Y_SCALE            = "log-scale"

# The offset from center to locate the sampling bin at.
SAMPLE_OFFSET          = 5e6

class SweeperSignalHound(sh.SignalHound):


	def setupSignalhound(self, freq=100e6):
		self.configureAcquisition(ACQ_MODE, ACQ_Y_SCALE)
		self.configureCenterSpan(center = freq+5e6, span = ACQ_SPAN)
		self.configureLevel(ref = ACQ_REF_LEVEL_DB, atten = ACQ_ATTENUATION_DB)
		self.configureGain(gain = ACQ_GAIN_SETTING)
		self.configureSweepCoupling(rbw = ACQ_RBW, vbw = ACQ_VBW, sweepTime = ACQ_SWEEP_TIME_SECONDS, rbwType = "native", rejection = "no-spur-reject")
		self.configureWindow(window = ACQ_WINDOW_TYPE)
		self.configureProcUnits(units = ACQ_UNITS)
		self.configureTrigger(trigType = "none", edge = "rising-edge", level = 0, timeout = 5)
		# self.configureIO("dc", "int-ref-out", "out-logic-low")
		# self.configureDemod("fm", 102.3e6, 250e3, 12e3, 20, 50)

		# self.configureRawSweep(100, 8, 2)
		self.initiate(mode = ACQ_TYPE, flag = "ignored")
		print(self.getCurrentAcquisitionSettings())

	def setAcqCenterFreq(self, freq):
		self.configureCenterSpan(center = freq, span = ACQ_SPAN)
		self.initiate(mode = ACQ_TYPE, flag = "ignored")

	def getPowerAtFreq(self, freq):

		self.setAcqCenterFreq(freq-SAMPLE_OFFSET)

		trace = self.fetchTrace()
		traceSize = len(trace['max'])
		# Binsize is span divided by number of returned points
		acqBinSize = ACQ_SPAN / traceSize

		# the offset of the frequency we're interested in is
		binFreqOffset = int(SAMPLE_OFFSET / acqBinSize)

		binFreqCenter = (traceSize / 2) + binFreqOffset

		# Return the max of the three frequency bins around the point of interest
		ret = max(trace['max'][binFreqCenter-1:binFreqCenter+4])

		# print(traceSize, trace['max'].argmax(), binFreqCenter)
		# print(ret, max(trace['max']))
		# print(trace['max'][binFreqCenter-1:binFreqCenter+4])

		return ret

def frange(x, y, jump):
	while x <= y:
		yield x
		x += jump

def go():
	print("Starting")
	# logSetup.initLogging()
	print("Loggers initialized")
	port = EoreController("COM4")
	print("Port opened")
	# hound = SweeperSignalHound()
	# print("SignalHound connected")
	# hound.setupSignalhound()
	# print("SignalHound configured")


	x = 0
	port.writeAtten(0, 0)
	port.writeAtten(1, 0)
	port.writeAtten(2, 0)

	vals = [0,1,2,3,4,5]

	port.writeSwitch(0, 0)

	port.writeAtten(2, 31.5)

	ret = port.writeOscillator(0, 15e6)

	START = 50e6
	STOP  = 250e6

	START = 199.95e6
	STOP  = 200.05e6
	STEP  = 1e3

	# with open("log - %s.csv" % time.time(), "w") as fp:

	while (1):
		# fp.write("# Start: %s\n" % START)
		# fp.write("# Stop:  %s\n" % STOP)
		# fp.write("# Step:  %s\n" % STEP)


		for x in frange(START, STOP, STEP):

			ret = port.writeOscillator(0, x)
			time.sleep(0.020)
			if not ret:
				print("Wat?")

			print(x)
			# print(x, trace)
			# trace = hound.getPowerAtFreq(x)
			# fp.write("%s, %s\n" % (x, trace))





if __name__ == "__main__":
	go()
