# -*- coding: UTF-8 -*-


# Pylint cannot determine contents of multiprocessing.Manager, so turn off the warning.
# pylint: disable=E1101

import multiprocessing as mp
import threading

import time

import signal
import sys
import errno


def interruptibleSleep(duration):
	'''
	unfortunately, `time.sleep()` catches SIGINT, even through it's masked.
	Because it will then raise an IOError, we have a wrapper that calls sleep(),
	and catches any cases of IOError where the flag is `EINTR`, and simply returns
	immediately if this occurs.
	'''
	try:
		time.sleep(duration)
	except IOError, e:
		if e.errno != errno.EINTR:
			raise
		print("Sleep interupted!")
		return


class Threader(object):
	def __init__(self, ctrlNs):
		self.ctrlNs = ctrlNs

	def startThread(self):
		self.thread = threading.Thread(target=self.testThread)
		self.thread.start()

	def testThread(self):
		while self.ctrlNs.alive:
			interruptibleSleep(1)

	def join(self):
		self.thread.join()


def go():




	signal.signal(signal.SIGINT, signal.SIG_IGN)


	ctrlManager = mp.Manager()
	ctrlNs = ctrlManager.Namespace()
	ctrlNs.alive = True

	th = Threader(ctrlNs)
	th.startThread()

	def signal_handler(dummy_signal, dummy_frame):
		print("Halting")
		ctrlNs.alive = False


	signal.signal(signal.SIGINT, signal_handler)


	while ctrlNs.alive:
		interruptibleSleep(1)



	print("Halting manager")
	th.join()
	ctrlManager.shutdown()
	sys.exit()


if __name__ == "__main__":

	go()