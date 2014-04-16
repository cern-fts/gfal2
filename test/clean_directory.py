#!/usr/bin/env python
import gfal2
import logging
import optparse
import stat
import sys

log = logging.getLogger('gfal2.clean_directory')


class Cleaner(object):

    def __init__(self, abort_on_error=False, recursive=False, only_files=False, chmod=False):
        self.abort_on_error = abort_on_error
        self.recursive = recursive
        self.only_files = only_files
        self.chmod = chmod
        self.context = gfal2.creat_context()

    def _get_list(self, surl):
        files = []
        directories = []
        dh = self.context.opendir(surl)
        d_entry, d_stat = dh.readpp()
        while d_entry:
            full_path = surl + '/' + d_entry.d_name
            if stat.S_ISREG(d_stat.st_mode):
                files.append((full_path, d_stat))
            elif stat.S_ISDIR(d_stat.st_mode):
                directories.append((full_path, d_stat))
            d_entry, d_stat = dh.readpp()
        return files, directories

    def __call__(self, surl):
        log.info("Cleaning %s" % surl)

        try:
            files, directories = self._get_list(surl)
        except gfal2.GError, e:
            if self.abort_on_error:
                raise
            logging.error("Could not list %s (%s)" % (surl, e.message))
            return 0, 0

        n_files, n_directories = 0, 0

        for file, f_stat in files:
            try:
                self.context.unlink(file)
                log.info("Unlink %s" % file)
                n_files += 1
            except gfal2.GError, e:
                if self.abort_on_error:
                    raise
                log.error("Could not unlink %s (%s)" % (file, e.message))

        for directory, d_stat in directories:
            if self.chmod and not (d_stat.st_mode & 0666):
                try:
                    self.context.chmod(directory, 0775)
                    log.info("Chmod for %s" % directory)
                except gfal2.GError, e:
                    log.warn("Failed chmod for %s (%s)" % (directory, e.message))

            sub_files, sub_directories = self(directory)
            n_files += sub_files
            n_directories += sub_directories
            if not self.only_files:
                try:
                    log.info("Rmdir %s" % directory)
                    self.context.rmdir(directory)
                    n_directories += 1
                except gfal2.GError, e:
                    if self.abort_on_error:
                        raise
                    log.error("Failed to rmdir %s (%s)" % (directory, e.message))

        return n_files, n_directories


if __name__ == '__main__':
    parser = optparse.OptionParser(usage='usage: %prog [options] surl')
    parser.add_option('-x', '--abort', dest='abort_on_error', default=False,
                      action='store_true', help='Abort cleaning on the first error')
    parser.add_option('-r', '--recursive', dest='recursive', default=False,
                      action='store_true', help='Traverse directories recursively')
    parser.add_option('-f', '--files', dest='only_files', default=False,
                      action='store_true', help='Unlink only files')
    parser.add_option('-c', '--chmod', dest='chmod', default=False,
                      action='store_true', help='Attempt a chmod when a directory is not writeable')

    (options, args) = parser.parse_args()

    if len(args) != 1:
        parser.error('Wrong number of arguments')

    stdout_handler = logging.StreamHandler(sys.stdout)
    stdout_handler.setFormatter(logging.Formatter('[%(levelname)s] %(message)s'))
    log.addHandler(stdout_handler)
    log.setLevel(logging.INFO)

    cleaner = Cleaner(options.abort_on_error, options.recursive, options.only_files, options.chmod)
    n_files, n_directories = cleaner(args[0])
    logging.info("Removed %d files and %d directories" % (n_files, n_directories))
