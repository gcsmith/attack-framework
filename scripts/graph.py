#!/usr/bin/env python

import array, math, string
import numpy as np
import matplotlib.pyplot as plt

# ------------------------------------------------------------------------------
def read_csv(path):
    data = np.genfromtxt(path, delimiter=',', skip_header=0, unpack=True)
    cols = data.shape[0]
    rows = data.shape[1]
    print "read {} rows, {} cols".format(rows, cols)
    return data, cols, rows

# ------------------------------------------------------------------------------
def guess_key_from_conf(data):
    key_idx = 1
    last = data.shape[1] - 1
    for i in range(1, data.shape[0]):
        if data[i][last] > data[key_idx][last]:
            key_idx = i
    return key_idx-1

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
    return key_idx-1, curr_max, data[0][sample]

# ------------------------------------------------------------------------------
def plot_conf(path):
    """ Render a plot of confidence ratio over the # of traces collected. """
    data, cols, rows = read_csv(path)
    key_guess = guess_key_from_conf(data)
    print "key with highest confidence in last interval:", key_guess

    plt.figure().canvas.set_window_title('Figure - Confidence (' + path + ')')
    for i in range(1, cols): plt.plot(data[0], data[i])

    plt.title('Result Confidence')
    plt.xlabel('Traces Applied')
    plt.ylabel('Confidence Ratio')
    plt.gca().yaxis.grid(color='gray', linestyle='dashed')
    plt.show()

# ------------------------------------------------------------------------------
def plot_corr(path):
    """ Render a plot of the distribution of sensitive data values. """
    data, cols, rows = read_csv(path)

    plt.figure().canvas.set_window_title('Figure - Correlation (' + path + ')')
    for i in range(1, cols): plt.plot(data[0], data[i])
    
    plt.title('Correlation')
    plt.xlabel('Time (ns)')
    plt.ylabel('Correlation Coefficient')
    plt.gca().yaxis.grid(color='gray', linestyle='dashed')
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
    plt.xlabel('Time (ns)')
    plt.ylabel('Difference')
    plt.gca().yaxis.grid(color='gray', linestyle='dashed')
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
    plt.gca().yaxis.grid(color='gray', linestyle='dashed')
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
    plt.gca().yaxis.grid(color='gray', linestyle='dashed')
    plt.show()

# ------------------------------------------------------------------------------
def plot_pow(path):
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

    args = p.parse_args()

    for path in args.conf: plot_conf(path)
    for path in args.corr: plot_corr(path)
    for path in args.diff: plot_diff(path)
    for path in args.rpow: plot_relpow(path)
    for path in args.dist: plot_dist(path)
    for path in args.power: plot_pow(path)

