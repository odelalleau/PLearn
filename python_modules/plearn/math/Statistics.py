"""Descriptive statistical analysis tool.

Orignal version by Chad J. Schroeder, modified by Christian Dorion.
"""
__version__ = "0.1"

__all__ = [ "StatisticsException", "Statistics" ]

from math import sqrt

def compute_stats(observations, weights=None):
    if weights is None:
        weights = [1]*len(observations)
        
    wsum = 0.0
    w2sum = 0.0

    # weighted sum of observations
    xsum = 0.0
    
    # sum of squared observations
    x2sum = 0.0

    for w,x in zip(weights, observations):
        wsum += w    
        w2sum += w**2    

        xsum += w * x
        x2sum += w * x**2
        
    # weighted mean
    wmean = xsum/wsum

    # weighted variance
    wvar = (wsum*x2sum - xsum**2) / (wsum**2 - w2sum)
    wstddev = sqrt(wvar)

    # weighted skewness and kurtosis
    wskew = 0.0
    wkurt = 0.0
    for w,x in zip(weights, observations):
        centered_normalized_obs = (x-wmean)/wstddev 
        wskew += w * centered_normalized_obs**3
        wkurt += w * centered_normalized_obs**4

    wskew /= wsum
    wkurt /= wsum
    wkurt -= 3

    return {"N"        : len(observations),
            "mean"     : wmean,
            "variance" : wvar,
            "skewness" : wskew,
            "kurtosis" : wkurt
            }

def jarque_bera(N, variance, skewness, kurtosis):
   return (N/6.)*(skewness/variance**(1.5))**2\
          + (N/24.)*(kurtosis/variance**2 -3)**2

class StatisticsException(Exception):
    """Statistics Exception class."""
    pass

class Statistics(object):
    """Class for descriptive statistical analysis.
    
    Behavior:
       Computes numerical statistics for a given data set.
    
    Available public methods:
    
       None
    
    Available instance attributes:
    
               N: total number of elements in the data set
             sum: sum of all values (n) in the data set
             min: smallest value of the data set
             max: largest value of the data set
            mode: value(s) that appear(s) most often in the data set
            mean: arithmetic average of the data set
           range: difference between the largest and smallest value in the data set
          median: value which is in the exact middle of the data set
        variance: measure of the spread of the data set about the mean
          stddev: standard deviation - measure of the dispersion of the data set
                  based on variance
     sharperatio: ratio of the mean and the stddev
    
    identification: Instance ID
    
    Raised Exceptions:    
    
       StatisticsException
    
    Bases Classes:
    
       object (builtin)
    
    Example Usage:
    
       x = [ -1, 0, 1 ]
    
       try:
          stats = Statistics(x)
       except StatisticsException, mesg:
          <handle exception>
    
       print "N: %s" % stats.N
       print "SUM: %s" % stats.sum
       print "MIN: %s" % stats.min
       print "MAX: %s" % stats.max
       print "MODE: %s" % stats.mode
       print "MEAN: %0.2f" % stats.mean
       print "RANGE: %s" % stats.range
       print "MEDIAN: %0.2f" % stats.median
       print "VARIANCE: %0.5f" % stats.variance
       print "STDDEV: %0.5f" % stats.stddev
       print "DATA LIST: %s" % stats.sample
    
    """
                                                                                 
    def __init__(self, sample=[], population=False):
       """Statistics class initializer method."""
    
       # Raise an exception if the data set is empty.
       if (not sample):
          raise StatisticsException, "Empty data set!: %s" % sample
    
       # The data set (a list).
       self.sample = sample
    
       # Sample/Population variance determination flag.
       self.population = population
    
       self.N = len(self.sample)
    
       self.sum = float(sum(self.sample))
    
       self.min = min(self.sample)
    
       self.max = max(self.sample)
    
       self.range = self.max - self.min
    
       self.mean = self.sum/self.N
    
       # Inplace sort (list is now in ascending order).
       self.sample.sort()
    
       self.__getMode()
       self.__getMedian()
       self.__getVariance()
       self.__getStandardDeviation()
    
       if self.stddev > 0:
          self.sharperatio = self.mean / self.stddev
       else:
          self.sharperatio = float('nan')
    
       # Instance identification attribute.
       self.identification = id(self)
    
    def __str__(self):
       return '\n'.join([
          "N: %s" % self.N,
          "SUM: %s" % self.sum,
          "MIN: %s" % self.min,
          "MAX: %s" % self.max,
          "MODE: %s" % self.mode,
          "MEAN: %0.2f" % self.mean,
          "RANGE: %s" % self.range,
          "MEDIAN: %0.2f" % self.median,
          "VARIANCE: %0.5f" % self.variance,
          "STDDEV: %0.5f" % self.stddev
          ])
                        
    
    def __getMode(self):
       """Determine the most repeated value(s) in the data set."""
    
       # Initialize a dictionary to store frequency data.
       frequency = {}
    
       # Build dictionary: key - data set values; item - data frequency.
       for x in self.sample:
          if (x in frequency):
             frequency[x] += 1
          else:
             frequency[x] = 1
    
       # Create a new list containing the values of the frequency dict.  Convert
       # the list, which may have duplicate elements, into a set.  This will
       # remove duplicate elements.  Convert the set back into a sorted list
       # (in descending order).  The first element of the new list now contains
       # the frequency of the most repeated values(s) in the data set.
       # mode = sorted(list(set(frequency.values())), reverse=True)[0]
       # Or use the builtin - max(), which returns the largest item of a
       # non-empty sequence.
       mode = max(frequency.values())
    
       # If the value of mode is 1, there is no mode for the given data set.
       if (mode == 1):
          self.mode = []
          return
    
       # Step through the frequency dictionary, looking for values equaling
       # the current value of mode.  If found, append the value and its
       # associated key to the self.mode list.
       self.mode = [(x, mode) for x in frequency if (mode == frequency[x])]
    
    def __getMedian(self):
       """Determine the value which is in the exact middle of the data set."""
    
       if (self.N%2):		# Number of elements in data set is odd.
          self.median = float(self.sample[self.N/2])
       else:
          midpt = self.N/2	# Number of elements in data set is even.
          self.median = (self.sample[midpt-1] + self.sample[midpt])/2.0
    
    def __getVariance(self):
       """Determine the measure of the spread of the data set about the mean.
       Sample variance is determined by default; population variance can be
       determined by setting population attribute to True.
       """
    
       x = 0	# Summation variable.
    
       # Subtract the mean from each data item and square the difference.
       # Sum all the squared deviations.
       for item in self.sample:
          x += (item - self.mean)**2.0
    
       try:
          if (not self.population):
             # Divide sum of squares by N-1 (sample variance).
             self.variance = x/(self.N-1)
          else:
             # Divide sum of squares by N (population variance).
             self.variance = x/self.N
       except:
          self.variance = 0
    
    def __getStandardDeviation(self):
       """Determine the measure of the dispersion of the data set based on the
       variance.
       """
    
       from math import sqrt     # Mathematical functions.
    
       # Take the square root of the variance.
       self.stddev = sqrt(self.variance)

if __name__ == "__main__":

   import os               # Miscellaneous OS interfaces.
   import sys              # System-specific parameters and functions.

   # Self-test

   a = [ -1, 0, 1 ]
   b = [ -1.0, 0.0, 1.1 ]
   c = []
   d = [ 12.23 ]
   e = [ 12.23, 99.543, 66.08 ]
   f = [ -1, 0, 2, -2, 1, 3, 0, -3, 2 ]
   g = [ 0, 9, 1, 8, 2, 7, 3, 6, 4, 5 ]
   h = [ -1, -1 ]

   for x in a, b, c, d, e, f, g, h:
      try:
         stats = Statistics(x)
      except StatisticsException, mesg:
         print; print "Exception caught: %s" % mesg; print
         continue
      print
      print "N: %s" % stats.N
      print "SUM: %s" % stats.sum
      print "MIN: %s" % stats.min
      print "MAX: %s" % stats.max
      print "MODE: %s" % stats.mode
      print "MEAN: %0.2f" % stats.mean
      print "RANGE: %s" % stats.range
      print "MEDIAN: %0.2f" % stats.median
      print "VARIANCE: %0.5f" % stats.variance
      print "STDDEV: %0.5f" % stats.stddev
      print "DATA LIST: %s\n" % stats.sample
      print

   sys.exit(0)
