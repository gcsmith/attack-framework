#!/usr/bin/env python

import argparse, os, sys, shutil, subprocess, threading

class simulation_thread(threading.Thread):
    def __init__(self, build_cmd, work_path, quiet):
        threading.Thread.__init__(self)
        self.build_cmd = build_cmd
        self.work_path = work_path
        self.quiet = quiet

    def run(self):
        if self.quiet:
            fp_stdout = open(os.path.join(self.work_path, 'stdout.txt'), 'w')
            fp_stderr = open(os.path.join(self.work_path, 'stderr.txt'), 'w')
        else:
            fp_stdout = None
            fp_stderr = None

        child = subprocess.Popen(self.build_cmd, cwd=self.work_path,
                                 stdout=fp_stdout, stderr=fp_stderr)
        child.wait()
        print self.getName(), 'completed'

p = argparse.ArgumentParser(description='Parallelize design simulation')
p.add_argument('-p', '--proj', metavar='arg', required=True,
               help='specify the top level design directory')
p.add_argument('-w', '--work', metavar='arg', required=True,
               help='specify the working directory for simulation')
p.add_argument('-i', '--inst', metavar='arg', type=int, default=1,
               help='number of simulator instances to run')
p.add_argument('-d', '--design', metavar='arg',
               help='design unit to simulate')
p.add_argument('-t', '--target', metavar='arg', default='vsim',
               help='make target to build')
p.add_argument('--quiet', action="store_true",
               help='write process output to a file')

# parse and validate the command line arguments
args = p.parse_args()

if not os.path.isdir(args.proj):
    print 'project path is not a directory:', args.proj
    sys.exit(1)

if args.inst < 1:
    print 'must specify at least one instance to run'
    sys.exit(1)

if not os.path.isdir(args.work):
    os.makedirs(args.work)

# construct the command line to invoke (make target and design unit)
build_cmd = ['make', args.target]
if args.design != None:
    build_cmd.append('UNIT=' + args.design)

proj_abs = os.path.abspath(args.proj)
os.chdir(args.work)

threads = []
for i in range(args.inst):
    # create and cd into the temporary working directory
    work_path = 'work_' + str(i)
    if os.path.isdir(work_path):
        print 'Deleting existing work directory:', work_path
        shutil.rmtree(work_path)

    os.mkdir(work_path)
    os.chdir(work_path)

    # create symlinks to each necessary design subdirectory
    os.symlink(os.path.relpath(os.path.join(proj_abs, 'Makefile')), 'Makefile')
    os.symlink(os.path.relpath(os.path.join(proj_abs, 'lib')), 'lib')
    os.symlink(os.path.relpath(os.path.join(proj_abs, 'rtl')), 'rtl')
    os.symlink(os.path.relpath(os.path.join(proj_abs, 'scr')), 'scr')
    os.symlink(os.path.relpath(os.path.join(proj_abs, 'tb')), 'tb')

    # cd back into the original working directory
    os.chdir('..')

    # spawn the subprocess to kick off the simulation
    t = simulation_thread(build_cmd, os.path.abspath(work_path), args.quiet)
    t.start()
    threads.append(t)

for i in range(args.inst):
    threads[i].join()
    sim_path = 'work_' + str(i) + '/simulation.txt'
    pow_path = 'work_' + str(i) + '/power_waveform.txt'
    if os.path.isfile(sim_path):
        os.rename(sim_path, 'simulation_' + str(i) + ".txt")
    if os.path.isfile(pow_path):
        os.rename(pow_path, 'power_waveform_' + str(i) + ".txt")

