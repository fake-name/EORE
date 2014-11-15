EORE
====

Era of Reionization Experiment

This is the hardware designs for the RF and backend components of a radio-telescope intended to look at the heavily [red-shifted](http://en.wikipedia.org/wiki/Redshift) [21 CM hydrogen line](http://en.wikipedia.org/wiki/Reionization#21-cm_line).  

While the normal 21 cm line is ~1420 MHz, this experiment is targeting the 50-250 MHz spectrum, which is where the 21 cm line from the era of reionization has been red-shifted to. 

Current topology is to use a SignalHound [BB60C](https://www.signalhound.com/products/bb60c/) spectrum analyser for the backend receiver (though my [API wrapper](https://github.com/fake-name/pySignalHound)), while the boards in this repository are front-end amplifiers and various components for tracking local oscillator drift, antenna characterization and so forth.

The eventual system will involve a great many separate nodes spread around the globe, all acting as a large-scale array.
