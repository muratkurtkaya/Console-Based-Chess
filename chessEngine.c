from numpy import arange, array
from matplotlib.pyplot import plot, show
from pylab import *
from math import *
import numpy as np


def f(x, y):
    return tan(x) + e ** (y / 2)


def rKutta2(f, x0, y0):
    h = 0.1
    xs = 8
    a1 = 1 / 3
    a2 = 2 / 3
    p = 3 / 4
    q = 3 / 4
    y = []
    while x0 < xs:
        y.append(y)
        x = x0 + h
        k1 = f(x0, y0)
        k2 = f(x0 + p * h, y0 + q * h * k1)
        y = y0 + h * (a1 * k1 + a2 * k2)
        x0 = x
        y0 = y
    return y


def rKutta4(f, x0, y0):
    h = 1
    xs = 8
    y = []
    while x0 < xs:
        y.append(y)
        x = x0 + h
        k1 = f(x0, y0)
        k2 = f(x0 + h / 2, y0 + (h / 2) * k1)
        k3 = f(x0 + h / 2, y0 + (h / 2) * k2)
        k4 = f(x0 + h, y0 + h * k3)
        y = y0 + (h / 6) * (k1 + 2 * k2 + 2 * k3 + k4)
        x0 = x
        y0 = y
    return y


plot(arange(1,8,0.1),rKutta2(f,1,2))
plot(arange(1,8,0.1),rKutta4(f,1,2))