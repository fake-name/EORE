# -*- coding: UTF-8 -*-


# pylint: disable=R0913, R0912, W0603

import logSetup
import logging

import multiprocessing as mp
import threading
import abc
import time

class TimedJob(object):

	# This is an abstract class
	__metaclass__ = abc.ABCMeta

	#: Number of seconds this job wants to run for.
	@abc.abstractmethod
	def runDuration(self):
		pass

	#: Maximum timeout for a job to halt before an error is raised
	@abc.abstractmethod
	def haltTimeout(self):
		pass

	# Should this job halt as soon as convenient.
	shouldRun = True

	def __init__(self, printQ, ctrlNs):
		self.printQ = printQ
		self.ctrlNs = ctrlNs


	def go(self):
		# Launch a job in a thread, with a second thread maintaining the timeout.
		self.shouldRun = True
		self.thread = threading.Thread(target=self._go)

		self.thread.start()
		time.sleep(self.runDuration)

		self.shouldRun = False
		for dummy_x in xrange(self.haltTimeout):
			self.thread.join(1)
			if not self.thread.isAlive():
				return

		raise ValueError("Thread failed to exit within the shutdown timeout!")


	@abc.abstractmethod
	def _go(self):
		pass

	@classmethod
	def run(cls, printQ, ctrlNs):
		instance = cls(printQ, ctrlNs)
		instance.go()

class PrintOh(TimedJob):

	runDuration = 3
	haltTimeout = 2

	def _go(self):
		while self.shouldRun:
			print('Oh')
			time.sleep(1)

class PrintHai(TimedJob):

	runDuration = 3
	haltTimeout = 2

	def _go(self):
		while self.shouldRun:
			print('Hai')
			time.sleep(1)


# def go():


# 	printQueue = mp.Queue()

# 	logSetup.initLogging(printQ = printQueue)


# 	ctrlManager = mp.Manager()
# 	ctrlNs = ctrlManager.Namespace()
# 	ctrlNs.run = True
# 	ctrlNs.acqRunning = True
# 	ctrlNs.apiRunning = True
# 	ctrlNs.logRunning = True
# 	ctrlNs.stopped = False



# 	ctrlManager.shutdown()
# 	sys.exit()


def go():

	while 1:
		PrintOh.run(None, None)
		PrintHai.run(None, None)

if __name__ == "__main__":

	go()