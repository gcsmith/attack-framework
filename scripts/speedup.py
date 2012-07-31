#!/usr/bin/env python

import array, math, string
import numpy as np
import matplotlib.pyplot as plt

r = 10
axis = np.array([ 0, 1, 2, 3, 4, 5, 6 ])
# data = np.array([  75.676624, 75.571659, 39.127862, 27.147198, 22.726426, 22.952425, 23.166627 ])
# data = np.array([  81.235585, 75.974564, 39.860928, 29.090069, 26.190650, 26.879242, 27.446050 ])
data = np.array([ 130.459094, 79.370314, 64.729750, 70.619566, 76.634129, 82.630745, 88.189279 ])

fig = plt.figure()
fig.canvas.set_window_title('Multithreaded Performance')

ax1 = fig.add_subplot(121)
ax1.set_title('Parallelism vs. Time (r={})'.format(r))
ax1.set_xlabel('Number of Worker Threads')
ax1.set_ylabel('Time (seconds)')
ax1.yaxis.grid(color='gray', linestyle='dashed')
ax1.set_xticklabels(['1*', '1', '2', '3', '4', '5', '6'])
ax1.plot(axis, data)

ax2 = fig.add_subplot(122)
ax2.set_title('Parallelism vs. Speedup (r={})'.format(r))
ax2.set_xlabel('Number of Worker Threads')
ax2.set_ylabel('Speedup')
ax2.yaxis.grid(color='gray', linestyle='dashed')
ax2.set_xticklabels(['1*', '1', '2', '3', '4', '5', '6'])
ax2.plot(axis, data[0] / data)

plt.show()

