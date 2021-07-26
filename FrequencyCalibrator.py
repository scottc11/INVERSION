import math

# VCOTuner
# https://github.com/TheSlowGrowth/VCOTuner/blob/v0.2.4/Source/VCOTuner.h
# https://modwiggler.com/forum/viewtopic.php?f=17&t=164286



# Frequency = m(Voltage) + b

# send VCO a voltage, then sample the frequency at that voltage
# send VCO previous voltage + 1 volt, then sample the frequency

# y = a*x2 + b*x + c

# this function spits the "a" in the quadratic formula
def calculateA(p1, p2, p3):
  x1, y1 = p1
  x2, y2 = p2
  x3, y3 = p3

  D1 = calculateDenominator(x1, x2, x3)
  D2 = calculateDenominator(x2, x1, x3)
  D3 = calculateDenominator(x3, x1, x2)
  return y1/D1 + y2/D2 + y3/D3

# this function spits the "b" in the quadratic formula
def calculateB(p1, p2, p3):
  x1, y1 = p1
  x2, y2 = p2
  x3, y3 = p3

  D1 = calculateDenominator(x1, x2, x3)
  D2 = calculateDenominator(x2, x1, x3)
  D3 = calculateDenominator(x3, x1, x2)
  return -(y1 * (x2 + x3) / D1) - (y2 * (x1 + x3) / D2) - (y3 * (x1 + x2) / D3)

# this function spits the "c" in the quadratic formula
def calculateC(p1, p2, p3):
  x1, y1 = p1
  x2, y2 = p2
  x3, y3 = p3

  D1 = calculateDenominator(x1, x2, x3)
  D2 = calculateDenominator(x2, x1, x3)
  D3 = calculateDenominator(x3, x1, x2)
  return ((y1 * x2 * x3) / D1) + ((y2 * x1 * x3) / D2) + ((y3 * x1 * x2) / D3)


def calculateDenominator(x1, x2, x3):
  return math.pow(x1, 2) - x1*x3 - x1*x2 + x2*x3


# p1, p2, p3 are tuples (voltage, frequency)
# this function takes three points and outputs the quadratic function which sits on all those points
def makeYSolver(p1, p2, p3):
  a = calculateA(p1, p2, p3)
  b = calculateB(p1, p2, p3)
  c = calculateC(p1, p2, p3)

  def ySolver(x): # x = voltage, return = frequency
    return a*math.pow(x, 2) + b*x + c
  return ySolver



# Given a target frequency and a series of sampled points, returns a voltage
# sample three voltages and their frequencies, pass them into this function as tuples, give a target frequency
# returns the voltage required to achieve that frequency
def makeXSolver(p1, p2, p3, targetFreq):
  a = calculateA(p1, p2, p3)
  b = calculateB(p1, p2, p3)
  c = calculateC(p1, p2, p3)

  plus = (-b + math.sqrt(math.pow(b, 2) - 4*a*(c-targetFreq))) / (2*a)
  minus = (-b - math.sqrt(math.pow(b, 2) - 4*a*(c-targetFreq))) / (2*a)
  return max(plus, minus)


p1 = (5630, 46.6364174)
p2 = (28150, 192.799072)
p3 = (61931, 1616.66663)

output = [19251, 19608, 19969, 20335, 20707, 21084]
inputs = [58.27, 61.74, 65.41, 69.30,  73.42, 77.78, 82.41]


for x in inputs:
  print(makeXSolver(p1, p2, p3, x))
