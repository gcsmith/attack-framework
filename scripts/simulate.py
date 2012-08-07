#!/usr/bin/env python

import datetime, os, shutil, subprocess, tarfile, time, threading

# ------------------------------------------------------------------------------
class AbstractWorkerThread(threading.Thread):
    def __init__(self, index, run_cmd, work_path, sema, quiet, bzip2):
        super(AbstractWorkerThread, self).__init__()
        self.index     = index              # thread index
        self.run_cmd   = run_cmd            # the command to execute
        self.work_path = work_path          # working directory for process
        self.sema      = sema               # semaphore to limit maximum DOP
        self.quiet     = quiet              # redirect output to a file
        self.bzip2     = bzip2              # compress the simulation results
        self.lock      = threading.Lock()   # synchronize access to status
        self.status    = 'allocated'        # current worker status to report

    def set_status(self, status):
        with self.lock:
            self.status = status

    def get_status(self):
        with self.lock:
            return self.status

# ------------------------------------------------------------------------------
class LocalWorker(AbstractWorkerThread):
    def __init__(self, index, run_cmd, work_path, sema, quiet, bzip2):
        super(LocalWorker, self).__init__(index, run_cmd, work_path, sema, quiet, bzip2)

    def run(self):
        if self.quiet:
            fp_stdout = open(os.path.join(self.work_path, 'stdout.txt'), 'w')
            fp_stderr = open(os.path.join(self.work_path, 'stderr.txt'), 'w')
        else:
            fp_stdout = None
            fp_stderr = None

        self.sema.acquire()
        print self.name, 'spawning', self.run_cmd, 'in', self.work_path
        child = subprocess.Popen(self.run_cmd, cwd=self.work_path,
                                 stdout=fp_stdout, stderr=fp_stderr)
        self.set_status('simulating')
        child.wait()

        os.rename(os.path.join(self.work_path, 'simulation.txt'),
                  'simulation_' + str(self.index) + '.txt')
        os.rename(os.path.join(self.work_path, 'power_waveform.out'),
                  'power_waveform_' + str(self.index) + '.out')

        if self.bzip2:
            self.set_status('compressing')
            with tarfile.open('simulation_results_' + str(self.index) + '.tar.bz2', 'w:bz2') as tar:
                tar.add('simulation_' + str(self.index) + '.txt')
                tar.add('power_waveform_' + str(self.index) + '.out')

        print self.name, 'completed'
        self.set_status('done')
        self.sema.release()

# ------------------------------------------------------------------------------
class GridWorker(AbstractWorkerThread):
    def __init__(self, index, run_cmd, work_path, sema, quiet, bzip2):
        super(GridWorker, self).__init__(index, run_cmd, work_path, sema, quiet, bzip2)

    def gen_script(self, path):
        sim_txt = 'simulation_{}.txt'.format(self.index)
        pow_out = 'power_waveform_{}.out'.format(self.index)
        with open(path, 'w') as fp:
            fp.write('#!/bin/bash\n')
            fp.write('#$ -N sim-' + self.name + '\n')
            fp.write('#$ -o qsub-output.log\n')
            fp.write('#$ -j y\n')
            fp.write('#$ -wd ' + self.work_path + '\n')
            fp.write(' '.join(self.run_cmd) + '\n')
            fp.write('mv simulation.txt {}\n'.format(sim_txt))
            fp.write('mv power_waveform.out {}\n'.format(pow_out))
            if self.bzip2:
                dest = 'simulation_results_{}.tar.bz2'.format(self.index)
                fp.write('tar cvjf {} {} {} > compress.log\n'.format(dest, sim_txt, pow_out))
                fp.write('mv {} {} {} ../\n'.format(dest, sim_txt, pow_out))
            else:
                fp.write('mv {} {} ../\n'.format(sim_txt, pow_out))
            fp.write('touch .complete\n')

    def run(self):
        exec_path = os.path.join(self.work_path, 'qsub-exec.sh')
        self.gen_script(exec_path)

        self.sema.acquire()
        print self.name, 'queueing', self.run_cmd

        try:
            subprocess.call([ 'qsub', '-V', exec_path ])
            self.set_status('submitted')
            while not os.path.exists(os.path.join(self.work_path, '.complete')):
                time.sleep(1)
                if os.path.exists(os.path.join(self.work_path, 'compress.log')):
                    self.set_status('compressing')
                elif os.path.exists(os.path.join(self.work_path, 'simulate.log')):
                    self.set_status('simulating')
                elif os.path.exists(os.path.join(self.work_path, 'compile.log')):
                    self.set_status('compiling')
            self.set_status('done')
        except:
            self.set_status('error')
            print self.name, '- error: unable to launch qsub'

        self.sema.release()

# ------------------------------------------------------------------------------
def create_symlinks(src_path, names):
    for name in names:
        os.symlink(os.path.relpath(os.path.join(src_path, name)), name)

# ------------------------------------------------------------------------------
def compress_results(results):
    with tarfile.open('simulation_results.tar.bz2', 'w:bz2') as tar:
        for name in results:
            print 'adding', name, 'to archive...'
            tar.add(name)

# ------------------------------------------------------------------------------
def parse_command_line():
    import argparse, sys

    p = argparse.ArgumentParser(description='Parallelize design simulation')
    p.add_argument('-p', '--proj', metavar='arg', required=True,
                   help='specify the top level design directory')
    p.add_argument('-w', '--work', metavar='arg', default='.',
                   help='specify the working directory for simulation')
    p.add_argument('-i', '--index', metavar='arg', type=int, default=0,
                   help='starting thread index')
    p.add_argument('-n', '--inst', metavar='arg', type=int, default=1,
                   help='number of simulator instances to run')
    p.add_argument('-d', '--dop', metavar='arg', type=int, default=0,
                   help='specify the maximum degree of parallelism')
    p.add_argument('-s', '--seed', metavar='arg',
                   help='specify the initial seed value for simulation')
    p.add_argument('-u', '--unit', metavar='arg',
                   help='design unit to simulate')
    p.add_argument('-v', '--vplus', metavar='arg',
                   help='verilog plusargs to pass to the testbench')
    p.add_argument('-V', '--vdef', metavar='arg',
                   help='verilog definitions to pass to the compiler')
    p.add_argument('-t', '--target', metavar='arg', default='vsim',
                   help='make target to build')
    p.add_argument('--amd64', action='store_true',
                   help='set system architecture to amd64 for synopsys tools')
    p.add_argument('--bzip2', action='store_true',
                   help='add results to a bzip2 compressed archive')
    p.add_argument('--quiet', action='store_true',
                   help='write process output to a file')
    p.add_argument('--qsub', action='store_true',
                   help='distribute jobs using the sun grid engine')
    
    # parse and validate the command line arguments
    args = p.parse_args()

    if not os.path.isdir(args.proj):
        print 'project path is not a directory:', args.proj
        sys.exit(1)
    if args.index < 0:
        print 'must specify non-zero starting index'
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
    run_cmd = ['make', args.target, 'AMD64=1' if args.amd64 else 'AMD64=0']
    if args.seed != None:
        run_cmd.append('SEED=' + args.seed)
    if args.unit != None:
        run_cmd.append('UNIT=' + args.unit)
    if args.vplus != None:
        run_cmd.append('PLUS="' + args.vplus + '"')
    if args.vdef != None:
        run_cmd.append('VDEF="' + args.vdef + '"')
    
    proj_abs = os.path.abspath(args.proj)
    os.chdir(args.work)
    
    # create a semaphore to enforce the maximum degree of parallelism allowed
    max_dop = min([args.dop, args.inst]) if args.dop > 0 else args.inst
    sema = threading.BoundedSemaphore(value=max_dop)

    threads = []
    for i in range(args.index, args.index + args.inst):
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
        if (args.qsub):
            t = GridWorker(i, run_cmd, work_abs, sema, args.quiet, args.bzip2)
        else:
            t = LocalWorker(i, run_cmd, work_abs, sema, args.quiet, args.bzip2)
        t.start()
        threads.append(t)

    # wait for running threads to complete, print status every 5 seconds
    while threading.active_count() > 1:
        print '\nwaiting on', threading.active_count() - 1, 'running tasks'
        print datetime.datetime.now(), ': sleeping for 5 seconds...\n';
        time.sleep(5)

        print 'task-name           status                                  '
        print '------------------------------------------------------------'
        for thread in threads:
            print '{:<20}{}'.format(thread.name, thread.get_status())

