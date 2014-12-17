# -*- coding: UTF-8 -*-


##  pylint: // disable=R0913, R0912, W0603

import errno
import multiprocessing as mp
import threading
import abc
import time

def interruptable(arg_func):
	'''
	unfortunately, `time.sleep()` catches SIGINT, even through it's masked.
	Because it will then raise an IOError, we have a wrapper that calls sleep(),
	and catches any cases of IOError where the flag is `EINTR`, and simply returns
	immediately if this occurs.

	This is a decorator because it has to be applied in multiple places, as
	some library functions use
	'''

	def wrapper(*args, **kwargs):
		try:
			arg_func(*args, **kwargs)
		except IOError, e:
			if e.errno != errno.EINTR:
				raise
			return
	return wrapper

@interruptable
def interruptibleSleep(duration):
	time.sleep(duration)

@interruptable
def interruptableJoin(thread, timeout):
	'''
	`thread.join({duration})` uses time.sleep() internally, which
	is broken by the fact that it obeys ctrl+c.
	Therefore, a wrapper that handles ctrl+c events properly
	'''
	thread.join(timeout)

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

	def __init__(self, printQ, shouldRun):

		# We're multi-processing. Catch and ignore signals.
		# signal.signal(signal.SIGINT, signal.SIG_IGN)

		self.printQ = printQ
		self.running = shouldRun

		# Multiprocessing shared value to trigger a graceful halt
		self.plzhalt = mp.Value('i')
		self.plzhalt.value = 1

	def log(self, *args, **kwargs):
		self.printQ.put(*args, **kwargs)

	def continueRunning(self):
		try:
			if self.plzhalt.value == 0:
				return False
		except Exception:
			self.log("Shouldrun flag manager killed?")
			return False
		try:
			if not self.running.value:
				return False
		except Exception:
			self.log("Control Namespace killed?")
			return False
		return True


	def _go(self):
		'''
		Launch a job (`go()` in a sub-class) in a thread, with a second thread maintaining the timeout.
		When the duration (`haltTimeout`) has elapsed, the maintenance thread will set `shouldRun` = False
		The child-thread should query `self.shouldRun()` frequently, which will return False if the timeout
		has passed, or a shutdown signal (Ctrl-C) has been received.
		'''


		self.plzhalt.value = 1

		self.thread = threading.Thread(target=self._go_thread)

		self.thread.start()
		interruptibleSleep(self.runDuration)

		# Since `interruptibleSleep()` will be interrupted by Ctrl+C, execution will reach
		# this point immediately after ctrl+c has been triggered.
		# The haltTimeout must allow enough time for the child-task to exit without error

		# To some extent, this renders the requirement for the ctrlNs.alive variable, since
		# the job will short-circuit to self.shouldRun = False on ctrl+c, but I'm not
		# completely confident that will be reliable with more threads and/or multiprocessing.
		self.plzhalt.value = 0
		for dummy_x in xrange(self.haltTimeout):
			interruptableJoin(self.thread, 1)
			self.log("Joining", dummy_x)
			if not self.thread.isAlive():
				return

		raise ValueError("Thread failed to exit within the shutdown timeout!")


	def _go_thread(self):
		'''
		Internal wrapper around `go()` that does some thread-local cleanup
		'''
		self.go()
		# self.log("Halting background processes")
		# self.printQ.close()



	@abc.abstractmethod
	def go(self):
		'''
		Scheduled task goes here. Must be overridden by a child-class.
		'''
		pass

	@classmethod
	def run(cls, printQ, runState):
		'''
		ClassMethod callable that instantiates a scheduler, and calls
		go on it.
		'''
		instance = cls(printQ, runState)
		instance._go()
		return True


class BaseScheduler(object):
	# This is an abstract class
	__metaclass__ = abc.ABCMeta

	def __init__(self, printQueue, runState):
		self.printQueue = printQueue
		self.runState   = runState


	#: Number of seconds this job wants to run for.
	@abc.abstractmethod
	def getNextJob(self):
		pass

	def go(self):

		try:
			for job in self.getNextJob():
				job.run(self.printQueue, self.runState)

				try:
					if not self.runState.value:
						print("Exiting due to runstate = ", self.runState.value)
						break
				except IOError:
					print("Manager killed?")
					break
				print("Runstate = ", self.runState.value)


		except Exception:
			print("Error")
			import traceback
			traceback.print_exc()


########################################################
#
# Testing:
#
########################################################

class PrintOh(TimedJob):

	runDuration = 3
	haltTimeout = 3

	def go(self):
		while self.continueRunning():
			self.log('Oh')
			interruptibleSleep(1)


class PrintHai(TimedJob):

	runDuration = 3
	haltTimeout = 3

	def go(self):
		while self.continueRunning():
			self.log('Hai')
			interruptibleSleep(1)


class TestScheduler(BaseScheduler):

	jobs = [PrintOh, PrintHai]
	jobIndex = 0

	def getNextJob(self):
		while 1:
			yield self.jobs[self.jobIndex]
			self.jobIndex += 1
			self.jobIndex = self.jobIndex % len(self.jobs)

def runScheduler(printQueue, runState):
	sched = TestScheduler(printQueue, runState)
	sched.go()
