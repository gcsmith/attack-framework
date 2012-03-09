#!/usr/bin/env python

import array, math, string
import numpy as np
import matplotlib.pyplot as plt

# ------------------------------------------------------------------------------
def read_csv(path):
    data = np.genfromtxt(path, delimiter=',', skip_header=1, unpack=True)
    cols = data.shape[0]
    rows = data.shape[1]
    print "read %d rows, %d cols" % (rows, cols)
    return data, cols, rows

# ------------------------------------------------------------------------------
def guess_key_from_conf(data):
    key_idx = 0
    last = data.shape[1] - 1
    for i in range(255):
        if data[i][last] > data[key_idx][last]:
            key_idx = i
    return key_idx

# ------------------------------------------------------------------------------
def guess_key_from_diff(data):
    """ Return the key guess with the largest differential spike. """
    curr_max = key_idx = sample = 0
    for k in range(1, data.shape[0]):
        for i in range(0, data.shape[1]):
            if math.fabs(data[k][i]) > curr_max:
                curr_max = math.fabs(data[k][i])
                key_idx = k
                sample = i;
    return key_idx-1, curr_max, sample

# ------------------------------------------------------------------------------
def plot_conf(path):
    """ Render a plot of confidence ratio over the # of traces collected. """
    data, cols, rows = read_csv(path)
    key_guess = guess_key_from_conf(data)
    print "key with highest confidence in last interval:", key_guess

    x = range(0, rows * 10, 10)

    plt.figure().canvas.set_window_title('Figure - Confidence (' + path + ')')
    for i in range(cols): plt.plot(x, data[i])

    plt.title('Result Confidence')
    plt.xlabel('Traces Applied')
    plt.ylabel('Confidence Ratio')
    plt.show()

# ------------------------------------------------------------------------------
def plot_corr(path):
    """ Render a plot of the distribution of sensitive data values. """
    data, cols, rows = read_csv(path)

    plt.figure().canvas.set_window_title('Figure - Correlation (' + path + ')')
    for i in range(1, cols): plt.plot(data[0], data[i])
    
    plt.title('Correlation')
    plt.xlabel('Time (ps)')
    plt.ylabel('Correlation Coefficient')
    plt.show()

# ------------------------------------------------------------------------------
def plot_diff(path):
    """ Render a plot of the difference over the sampled time period. """
    data, cols, rows = read_csv(path)
    key_guess, key_diff, s = guess_key_from_diff(data)
    print 'key with largest differential is', key_guess, '-', key_diff, '@', s
    
    plt.figure().canvas.set_window_title('Figure - Differential (' + path + ')')
    for i in range(1, cols): plt.plot(data[0], data[i])
    
    plt.title('Differential Trace')
    plt.xlabel('Time (ps)')
    plt.ylabel('Difference')
    plt.show()

# ------------------------------------------------------------------------------
def plot_relpow(path):
    """ Render a plot of the average power vs. sensitive data value. """
    data, cols, rows = read_csv(path)

    plt.figure().canvas.set_window_title('Figure - R. Power (' + path + ')')
    for i in range(1, cols): plt.plot(data[0], data[i])
    
    plt.title('Power vs. Sensitive Data Value')
    plt.xlabel('Sensitive Data Value')
    plt.ylabel('Voltage (mV)')
    plt.show()

# ------------------------------------------------------------------------------
def plot_dist(path):
    """ Render a plot of the distribution of sensitive data values. """
    data, cols, rows = read_csv(path)

    plt.figure().canvas.set_window_title('Figure - Distribution (' + path + ')')
    for i in range(1, cols): plt.plot(data[0], data[i])
    
    plt.title('Sensitive Value Distribution')
    plt.xlabel('Sensitive Data Value')
    plt.ylabel('Trace Count')
    plt.show()

# ------------------------------------------------------------------------------
def plot_pow(path):
    """ Render a plot of power consumption over the sampled time period. """
    index = -1
    times = array.array('l')
    power = array.array('d')
    
    fp = open(path, 'rt')
    for line in fp:
        if line == "done":
            break
        items = string.split(line)
        if len(items) == 1:
            index += 1
            times.append(int(items[0]))
            power.append(0)
        elif len(items) == 2:
            power[index] += float(items[1])
        else:
            print 'error: first power event before time stamp'
    
    plt.figure().canvas.set_window_title('Figure - Differential')
    plt.plot(times, power)

    plt.title('Power Trace')
    plt.xlabel('Time (ps)')
    plt.ylabel('Power')
    plt.show()

# ------------------------------------------------------------------------------
def plot_pow2(path):
    """ TODO """
    data, cols, rows = read_csv(path)

    plt.figure().canvas.set_window_title('Figure - Power (' + path + ')')
    for i in range(1, cols): plt.plot(data[0], data[i])
    
    plt.title('Trace Plot')
    plt.xlabel('Time')
    plt.ylabel('Voltage (mV)')
    plt.show()

# ------------------------------------------------------------------------------
if __name__ == "__main__":
    import argparse, sys

    p = argparse.ArgumentParser()
    p.add_argument('-c', '--conf', metavar='STR', action='append', default=[],
                   help="create confidence ratio plot")
    p.add_argument('-C', '--corr', metavar='STR', action='append', default=[],
                   help="create correlation plot")
    p.add_argument('-d', '--diff', metavar='STR', action='append', default=[],
                   help="create differential plot")
    p.add_argument('-r', '--rpow', metavar='STR', action='append', default=[],
                   help="create relative power plot")
    p.add_argument('-D', '--dist', metavar='STR', action='append', default=[],
                   help="create plot of trace sensitive value distribution")
    p.add_argument('-p', '--power', metavar='STR', action='append', default=[],
                   help="create instantaneous power consumption plot")
    p.add_argument('-P', '--power2', metavar='STR', action='append', default=[],
                   help="create instantaneous power consumption plot")

    args = p.parse_args()

    for path in args.conf: plot_conf(path)
    for path in args.corr: plot_corr(path)
    for path in args.diff: plot_diff(path)
    for path in args.rpow: plot_relpow(path)
    for path in args.dist: plot_dist(path)
    for path in args.power: plot_pow(path)
    for path in args.power2: plot_pow2(path)

