#!/usr/bin/env python

import math, os, shutil, subprocess, sys
import numpy as np

# ------------------------------------------------------------------------------
class result_collector:
    def __init__(self, path):
        if os.path.exists(path):
            print 'deleting previous results:', path
            shutil.rmtree(path)
        os.mkdir(path)

        self.result_dir  = path;
        self.guess_diffs = None             # estimate differential matrix
        self.known_diffs = None             # known key differential matrix
        self.guess_confs = None             # estimate confidence matrix
        self.known_confs = None             # known key confidence matrix
        self.valid_index = []
        self.guess_key   = np.array([])     # estimate key guess

    def examine_results(self, exp_key, result_dir, attack_dir):
        # extract the best key guess
        data = np.genfromtxt(os.path.join(attack_dir, 'differentials.csv'),
                             delimiter=',', skip_header=0, unpack=True)
        print 'diff: {} rows, {} cols'.format(data.shape[0], data.shape[1])

        curr_max = key_idx = sample = 0
        for k in range(1, data.shape[0]):
            for i in range(0, data.shape[1]):
                if math.fabs(data[k][i]) > curr_max:
                    curr_max = math.fabs(data[k][i])
                    key_idx = k
                    sample = i;

        # build a list of the computed key guesses 
        self.guess_key = np.append(self.guess_key, [ key_idx - 1 ])
        print self.guess_key

        # build a matrix of the differentials for each computed & known key
        if self.guess_diffs == None:
            self.guess_diffs = data[0]
        self.guess_diffs = np.vstack([self.guess_diffs, data[key_idx]])

        if self.known_diffs == None:
            self.known_diffs = data[0]
        self.known_diffs = np.vstack([self.known_diffs, data[exp_key + 1]])

        # extract the confidence interval for that key guess
        data = np.genfromtxt(os.path.join(attack_dir, 'confidence_interval.csv'),
                             delimiter=',', skip_header=0, unpack=True)
        print 'conf: {} rows, {} cols'.format(data.shape[0], data.shape[1])

        # build a matrix of the confidence for each computed & known key
        if self.guess_confs == None:
            self.guess_confs = data[0]
        self.guess_confs = np.vstack([self.guess_confs, data[key_idx]])

        if self.known_confs == None:
            self.known_confs = data[0]
        self.known_confs = np.vstack([self.known_confs, data[exp_key + 1]])

        valid_idx = -1
        for i in range(len(data[exp_key + 1])):
            if data[exp_key + 1][i] > 1.0:
                valid_idx = i
                break

        self.valid_index.append(valid_idx * self.report_int)
        print self.valid_index, ' --> ', max(self.valid_index)

    def run(self, num_bytes):
        for i in range(0, num_bytes):
            parm_str = self.parameters + ',byte=' + str(i)
            out_path = os.path.join(self.result_dir, 'attack_{:02d}'.format(i))
            run_args = [ './attack', '-i', self.trace_path, '-o', out_path,
                         '-c', self.crypto_str,
                         '-a', self.attack_str,
                         '-r', str(self.report_int),
                         '-n', str(self.num_traces),
                         '-p', parm_str,
                         '--threads', str(self.num_thread) ]

            if 0 != subprocess.call(run_args):
                print 'failed to execute:', run_args
                sys.exit(1)

            self.examine_results(self.known_key[i], self.result_dir, out_path)

        self.valid_index.append(max(self.valid_index))

        np.savetxt(os.path.join(self.result_dir, 'guess_intervals.csv'),
                   np.array(self.valid_index), fmt='%d', delimiter=',');
        np.savetxt(os.path.join(self.result_dir, 'guess_diffs.csv'),
                   self.guess_diffs.transpose(), delimiter=',');
        np.savetxt(os.path.join(self.result_dir, 'known_diffs.csv'),
                   self.known_diffs.transpose(), delimiter=',');
        np.savetxt(os.path.join(self.result_dir, 'guess_confs.csv'),
                   self.guess_confs.transpose(), delimiter=',');
        np.savetxt(os.path.join(self.result_dir, 'known_confs.csv'),
                   self.known_confs.transpose(), delimiter=',');
        np.savetxt(os.path.join(self.result_dir, 'guess_key.csv'),
                   self.guess_key.transpose(), fmt='%02X', delimiter=',');

# ------------------------------------------------------------------------------
def experiment(result_path, trace_path, crypto_str, attack_str, ntraces, nreports, params, nbytes, nthread, key):
    rc = result_collector(result_path)
    rc.trace_path = trace_path
    rc.crypto_str = crypto_str
    rc.attack_str = attack_str
    rc.num_traces = ntraces
    rc.report_int = nreports
    rc.parameters = params
    rc.num_thread = nthread
    rc.known_key  = key
    rc.run(nbytes)

# ------------------------------------------------------------------------------
if __name__ == '__main__':

    rijndael_sbox_key = [ 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF ]
    aes_iterative_key = range(16)
    grostl_serial_key = range(64)

    experiment('results_rijndael_sbox_dpa_sb',   'trace_4k_rijndael_sbox.bin',   'aes_hd_r0',          'dpa_dp', 4000, 5, 'bits=1,offset=0,thresh=0', 16, 5, rijndael_sbox_key)
    experiment('results_rijndael_sbox_dpa_mb',   'trace_4k_rijndael_sbox.bin',   'aes_hd_r0',          'dpa_dp', 4000, 5, 'bits=8,offset=0,thresh=5', 16, 5, rijndael_sbox_key)
    experiment('results_rijndael_sbox_cpa_sb',   'trace_4k_rijndael_sbox.bin',   'aes_hd_r0',          'cpa_dp', 4000, 5, 'bits=1,offset=0',          16, 5, rijndael_sbox_key)
    experiment('results_rijndael_sbox_cpa_mb',   'trace_4k_rijndael_sbox.bin',   'aes_hd_r0',          'cpa_dp', 4000, 5, 'bits=8,offset=0',          16, 5, rijndael_sbox_key)

#   experiment('results_rijndael_sbox_m_dpa_sb', 'trace_4k_rijndael_sbox_m.bin', 'aes_hd_r0',          'dpa_dp', 50000, 100, 'bits=1,offset=0,thresh=0', 16, 4, rijndael_sbox_key)
#   experiment('results_rijndael_sbox_m_dpa_mb', 'trace_4k_rijndael_sbox_m.bin', 'aes_hd_r0',          'dpa_dp', 50000, 100, 'bits=8,offset=0,thresh=5', 16, 4, rijndael_sbox_key)
#   experiment('results_rijndael_sbox_m_cpa_sb', 'trace_4k_rijndael_sbox_m.bin', 'aes_hd_r0',          'cpa_dp', 50000, 100, 'bits=1,offset=0',          16, 4, rijndael_sbox_key)
#   experiment('results_rijndael_sbox_m_cpa_mb', 'trace_4k_rijndael_sbox_m.bin', 'aes_hd_r0',          'cpa_dp', 50000, 100, 'bits=8,offset=0',          16, 4, rijndael_sbox_key)

    experiment('results_aes_iterative_dpa_sb',   'trace_4k_aes_iterative.bin',   'aes_hw_r0',          'dpa_dp', 4000, 5, 'bits=1,offset=0,thresh=0', 16, 5, aes_iterative_key)
    experiment('results_aes_iterative_dpa_mb',   'trace_4k_aes_iterative.bin',   'aes_hw_r0',          'dpa_dp', 4000, 5, 'bits=8,offset=0,thresh=5', 16, 5, aes_iterative_key)
    experiment('results_aes_iterative_cpa_sb',   'trace_4k_aes_iterative.bin',   'aes_hw_r0',          'cpa_dp', 4000, 5, 'bits=1,offset=0',          16, 5, aes_iterative_key)
    experiment('results_aes_iterative_cpa_mb',   'trace_4k_aes_iterative.bin',   'aes_hw_r0',          'cpa_dp', 4000, 5, 'bits=8,offset=0',          16, 5, aes_iterative_key)

#   experiment('results_aes_iterative_m_dpa_sb', 'trace_4k_aes_iterative_m.bin', 'aes_hw_r0',          'dpa_dp', 50000, 100, 'bits=1,offset=0,thresh=0', 16, 4, aes_iterative_key)
#   experiment('results_aes_iterative_m_dpa_mb', 'trace_4k_aes_iterative_m.bin', 'aes_hw_r0',          'dpa_dp', 50000, 100, 'bits=8,offset=0,thresh=5', 16, 4, aes_iterative_key)
#   experiment('results_aes_iterative_m_cpa_sb', 'trace_4k_aes_iterative_m.bin', 'aes_hw_r0',          'cpa_dp', 50000, 100, 'bits=1,offset=0',          16, 4, aes_iterative_key)
#   experiment('results_aes_iterative_m_cpa_mb', 'trace_4k_aes_iterative_m.bin', 'aes_hw_r0',          'cpa_dp', 50000, 100, 'bits=8,offset=0',          16, 4, aes_iterative_key)

    experiment('results_grostl_dp512_dpa_sb',    'trace_4k_grostl_dp512.bin',    'grostl_dp512_hd_r0', 'dpa_dp', 4000, 5, 'bits=1,offset=0,thresh=0', 64, 5, grostl_serial_key)
    experiment('results_grostl_dp512_dpa_mb',    'trace_4k_grostl_dp512.bin',    'grostl_dp512_hd_r0', 'dpa_dp', 4000, 5, 'bits=8,offset=0,thresh=5', 64, 5, grostl_serial_key)
    experiment('results_grostl_dp512_cpa_sb',    'trace_4k_grostl_dp512.bin',    'grostl_dp512_hd_r0', 'cpa_dp', 4000, 5, 'bits=1,offset=0',          64, 5, grostl_serial_key)
    experiment('results_grostl_dp512_cpa_mb',    'trace_4k_grostl_dp512.bin',    'grostl_dp512_hd_r0', 'cpa_dp', 4000, 5, 'bits=8,offset=0',          64, 5, grostl_serial_key)

#   experiment('results_grostl_dp512_m_dpa_sb',  'trace_4k_grostl_dp512_m.bin',  'grostl_dp512_hd_r0', 'dpa_dp', 50000, 100, 'bits=1,offset=0,thresh=0', 64, 4, grostl_serial_key)
#   experiment('results_grostl_dp512_m_dpa_mb',  'trace_4k_grostl_dp512_m.bin',  'grostl_dp512_hd_r0', 'dpa_dp', 50000, 100, 'bits=8,offset=0,thresh=5', 64, 4, grostl_serial_key)
#   experiment('results_grostl_dp512_m_cpa_sb',  'trace_4k_grostl_dp512_m.bin',  'grostl_dp512_hd_r0', 'cpa_dp', 50000, 100, 'bits=1,offset=0',          64, 4, grostl_serial_key)
#   experiment('results_grostl_dp512_m_cpa_mb',  'trace_4k_grostl_dp512_m.bin',  'grostl_dp512_hd_r0', 'cpa_dp', 50000, 100, 'bits=8,offset=0',          64, 4, grostl_serial_key)

    experiment('results_grostl_dp64_dpa_sb',     'trace_4k_grostl_dp64.bin',     'grostl_dp64_hd_r0',  'dpa_dp', 4000, 5, 'bits=1,offset=0,thresh=5', 64, 5, grostl_serial_key)
    experiment('results_grostl_dp64_dpa_mb',     'trace_4k_grostl_dp64.bin',     'grostl_dp64_hd_r0',  'dpa_dp', 4000, 5, 'bits=8,offset=0,thresh=5', 64, 5, grostl_serial_key)
    experiment('results_grostl_dp64_cpa_sb',     'trace_4k_grostl_dp64.bin',     'grostl_dp64_hd_r0',  'cpa_dp', 4000, 5, 'bits=1,offset=0',          64, 5, grostl_serial_key)
    experiment('results_grostl_dp64_cpa_mb',     'trace_4k_grostl_dp64.bin',     'grostl_dp64_hd_r0',  'cpa_dp', 4000, 5, 'bits=8,offset=0',          64, 5, grostl_serial_key)

#   experiment('results_grostl_dp64_m_dpa_sb',   'trace_4k_grostl_dp64_m.bin',   'grostl_dp64_hd_r0',  'dpa_dp', 50000, 100, 'bits=1,offset=0,thresh=0', 64, 4, grostl_serial_key)
#   experiment('results_grostl_dp64_m_dpa_mb',   'trace_4k_grostl_dp64_m.bin',   'grostl_dp64_hd_r0',  'dpa_dp', 50000, 100, 'bits=8,offset=0,thresh=5', 64, 4, grostl_serial_key)
#   experiment('results_grostl_dp64_m_cpa_sb',   'trace_4k_grostl_dp64_m.bin',   'grostl_dp64_hd_r0',  'cpa_dp', 50000, 100, 'bits=1,offset=0',          64, 4, grostl_serial_key)
#   experiment('results_grostl_dp64_m_cpa_mb',   'trace_4k_grostl_dp64_m.bin',   'grostl_dp64_hd_r0',  'cpa_dp', 50000, 100, 'bits=8,offset=0',          64, 4, grostl_serial_key)

