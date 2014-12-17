

import time
import signal
import errno
import multiprocessing as mp

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

def jobFunc(mgr):
	signal.signal(signal.SIGINT, signal.SIG_IGN)
	print("Job")
	interruptibleSleep(4)


def process(mgr):
	signal.signal(signal.SIGINT, signal.SIG_IGN)
	jobs = []
	cnt = 0
	while mgr.run:
		print("process")
		interruptibleSleep(1)

		proc = mp.Process(target=jobFunc, args=(mgr, ))
		proc.start()
		jobs.append(proc)

	for job in jobs:
		job.join()

def go():
	signal.signal(signal.SIGINT, signal.SIG_IGN)

	ctrlManager = mp.Manager()
	ctrlNs = ctrlManager.Namespace()
	ctrlNs.run = True

	def signal_handler(dummy_signal, dummy_frame):
		print("Halting")
		ctrlNs.run = False

	signal.signal(signal.SIGINT, signal_handler)

	proc = mp.Process(target=process, args=(ctrlNs, ))
	proc.start()

	while ctrlNs.run:
		interruptibleSleep(1)
		print("Main")

	proc.join()
	th.join()




if __name__ == "__main__":
	go()

