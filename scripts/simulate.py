#!/usr/bin/env python

import os, shutil, subprocess, tarfile, threading

# ------------------------------------------------------------------------------
class simulation_thread(threading.Thread):
    def __init__(self, build_cmd, work_path, sema, quiet):
        threading.Thread.__init__(self)
        self.build_cmd = build_cmd
        self.work_path = work_path
        self.sema      = sema
        self.quiet     = quiet

    def run(self):
        if self.quiet:
            fp_stdout = open(os.path.join(self.work_path, 'stdout.txt'), 'w')
            fp_stderr = open(os.path.join(self.work_path, 'stderr.txt'), 'w')
        else:
            fp_stdout = None
            fp_stderr = None

        self.sema.acquire()
        print self.getName(), 'spawning', self.build_cmd, 'in', self.work_path
        child = subprocess.Popen(self.build_cmd, cwd=self.work_path,
                                 stdout=fp_stdout, stderr=fp_stderr)
        child.wait()
        print self.getName(), 'completed'
        self.sema.release()

# ------------------------------------------------------------------------------
def create_symlinks(src_path, names):
    for name in names:
        os.symlink(os.path.relpath(os.path.join(src_path, name)), name)

# ------------------------------------------------------------------------------
def compress_results(results):
    tar = tarfile.open('simulation_results.tar.bz2', 'w:bz2')
    for name in results:
        print 'adding', name, 'to archive...'
        tar.add(name)
    tar.close()

# ------------------------------------------------------------------------------
def parse_command_line():
    import argparse, sys

    p = argparse.ArgumentParser(description='Parallelize design simulation')
    p.add_argument('-p', '--proj', metavar='arg', required=True,
                   help='specify the top level design directory')
    p.add_argument('-w', '--work', metavar='arg', default='.',
                   help='specify the working directory for simulation')
    p.add_argument('-i', '--inst', metavar='arg', type=int, default=1,
                   help='number of simulator instances to run')
    p.add_argument('-d', '--dop', metavar='arg', type=int, default=0,
                   help='specify the maximum degree of parallelism')
    p.add_argument('-s', '--seed', metavar='arg',
                   help='specify the initial seed value for simulation')
    p.add_argument('-u', '--unit', metavar='arg',
                   help='design unit to simulate')
    p.add_argument('-v', '--vplus', metavar='arg',
                   help='verilog plusargs to pass to the testbench')
    p.add_argument('-t', '--target', metavar='arg', default='vsim',
                   help='make target to build')
    p.add_argument('--amd64', action='store_true',
                   help='set system architecture to amd64 for synopsys tools')
    p.add_argument('--bzip2', action='store_true',
                   help='add results to a bzip2 compressed archive')
    p.add_argument('--quiet', action='store_true',
                   help='write process output to a file')
    
    # parse and validate the command line arguments
    args = p.parse_args()

    if not os.path.isdir(args.proj):
        print 'project path is not a directory:', args.proj
        sys.exit(1)
    if args.inst < 1:
        print 'must specify at least one instance to run'
        sys.exit(1)
    if args.dop < 0 or args.dop > args.inst:
        print 'invalid degree of parallelism specified:', args.dop
        sys.exit(1)
    if not os.path.isdir(args.work):
        print 'creating top level work directory:', args.work
        os.makedirs(args.work)

    return args

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    args = parse_command_line()

    # construct the command line to invoke (make target and design unit)
    build_cmd = ['make', args.target, 'AMD64=1' if args.amd64 else 'AMD64=0']
    if args.seed != None:
        build_cmd.append('SEED=' + args.seed)
    if args.unit != None:
        build_cmd.append('UNIT=' + args.unit)
    if args.vplus != None:
        build_cmd.append('PLUS="' + args.vplus + '"')
    
    proj_abs = os.path.abspath(args.proj)
    os.chdir(args.work)
    
    threads = []
    results = []

    # create a semaphore to enforce the maximum degree of parallelism allowed
    max_dop = min([args.dop, args.inst]) if args.dop > 0 else args.inst
    sema = threading.BoundedSemaphore(value=max_dop)

    for i in range(args.inst):
        # create the temporary working directory, and link the specified design
        work_path = 'work_' + str(i)
        work_abs  = os.path.abspath(work_path)

        if os.path.isdir(work_path):
            print 'Deleting existing work directory:', work_path
            shutil.rmtree(work_path)
    
        os.mkdir(work_path)
        os.chdir(work_path)
        create_symlinks(proj_abs, ['Makefile', 'lib', 'rtl', 'scr', 'tb'])
        os.chdir('..')
    
        # spawn the subprocess thread to kick off the simulation
        t = simulation_thread(build_cmd, work_abs, sema, args.quiet)
        t.start()
        threads.append(t)
    
    for i in range(args.inst):
        threads[i].join()
        sim_path = 'work_' + str(i) + '/simulation.txt'
        pow_path = 'work_' + str(i) + '/power_waveform.out'
        if os.path.isfile(sim_path):
            os.rename(sim_path, 'simulation_' + str(i) + '.txt')
            results.append('simulation_' + str(i) + '.txt')
        if os.path.isfile(pow_path):
            os.rename(pow_path, 'power_waveform_' + str(i) + '.out')
            results.append('power_waveform_' + str(i) + '.out')

    if args.bzip2:
        compress_results(results)

