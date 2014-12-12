

import pyximport
pyximport.install()


import ptest

ret = ptest.primes(1000)
print ret
