#!/usr/bin/env python
"""
Enables running hadd on a large number of files w/out crashing hadd.

Author: D. Austin Belknap, UW-Madison
"""

import sys
import os
import argparse


def hadd(outfile, infiles):
    """This makes a simple call to hadd"""
    cmd = "hadd " + outfile + " " + " ".join(infiles)
    os.system(cmd)


def batch_hadd(outfile, infiles, n_files=200):
    """
    This runs hadd on smaller chunks of files and merges them into intermediate
    files. Then hadd is run to merge the intermidate files into the final file.
    """

    # split the list of input files into sublists
    split_infiles = [infiles[x:x+n_files]
                     for x in xrange(0, len(infiles), n_files)]

    intermediate_files = []

    user_name = os.environ['USER']

    for i, file_list in enumerate(split_infiles):
        # Use unique names for the intermediate files to avoid collisions
        intm_file = ("/tmp/tmp_" + user_name + "_" + str(i) + "_" +
                     os.path.basename(outfile))
        print intm_file
        intermediate_files.append(intm_file)
        hadd(intm_file, file_list)

    # merge the intermediate files together
    hadd(outfile, intermediate_files)

    # remove the intermediate files
    os.system("rm " + " ".join(intermediate_files))


def parse_command_line(argv):
    parser = argparse.ArgumentParser(description='Run hadd on a large number '
                                                 'of ROOT files')
    parser.add_argument('outfile', type=str,
                        help='The name of the output ROOT file.')
    parser.add_argument('infiles', nargs='+',
                        help='A list of input ROOT files to merge.')
    parser.add_argument('--files-per-job', type=int, default=200,
                        help='Number of files to merge with hadd at one time. '
                             'Default is 200.')
    args = parser.parse_args(argv)

    return args


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]

    args = parse_command_line(argv)

    batch_hadd(args.outfile, args.infiles, args.files_per_job)

    return 0


if __name__ == "__main__":
    status = main()
    sys.exit(status)
