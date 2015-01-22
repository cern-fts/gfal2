#!/usr/bin/env python
import errno
import gfal2

CHECKSUMS = [
  'CRC32',
  'ADLER32',
  'MD5',
  'SHA1'
]

STORAGES = {
    'HTTP-DPM': 'davs://lxfsra10a01.cern.ch/dpm/cern.ch/home/dteam/',
    #'XROOTD-DPM': '',
    'GSIFTP-DPM': 'gsiftp://lxfsra10a01.cern.ch/dpm/cern.ch/home/dteam/',
    'SRM-DPM': 'srm://lxfsra10a01.cern.ch:8446/srm/managerv2?SFN=/dpm/cern.ch/home/dteam/',

    'HTTP-DCACHE': 'davs://prometheus.desy.de/VOs/dteam/',
    #'XROOTD-DCACHE': '',
    'GSIFTP-DCACHE': 'gsiftp://storage01.lcg.cscs.ch/pnfs/lcg.cscs.ch/dteam/',
    'SRM-DCACHE': 'srm://storage01.lcg.cscs.ch:8443/srm/managerv2?SFN=/pnfs/lcg.cscs.ch/dteam/',

    #'HTTP-STORM': '',
    #'XROOTD-STORM': '',
    #'GSIFTP-STORM': '',
    'SRM-STORM': 'srm://stormfe1.pi.infn.it:8444/srm/managerv2?SFN=/dteam/',

    #'HTTP-EOS': '',
    #'XROOTD-EOS': 'root://eospublic.cern.ch/eos/opstest/aalvarez/',
    #'GSIFTP-EOS': 'gsiftp://eospublicftp.cern.ch/',
    #'SRM-EOS': 'srm://srm-eospublic.cern.ch:8443/srm/v2/server?SFN=/eos/opstest/aalvarez/',

    #'HTTP-CASTOR': '',
    #'XROOTD-CASTOR': '',
    #'GSIFTP-CASTOR': '',
    #'SRM-CASTOR': '',

    #'XRD-HTTP': '',
}


def create_file(context, url):
    context.filecopy('file:///etc/hosts', url)


context = gfal2.creat_context()
context.set_opt_boolean('HTTP PLUGIN', 'INSECURE', True)

results = dict()
for checksum in CHECKSUMS:
    for storage, endpoint in STORAGES.iteritems():
        print "Testing %s\t%-15s\t" % (checksum, storage),
        url = endpoint + 'checksum.test'
        try:
            create_file(context, url)

            try:
                value = context.checksum(url, checksum)
                supported = True
                print "OK"
            except Exception, e:
                supported = False
                print "FAILED"
                print "\t%s" % str(e)
            finally:
                try:
                    context.unlink(url)
                except gfal2.GError, e:
                    if e.code != errno.ENOENT:
                        raise

            impl = results.get(storage, dict())
            impl[checksum] = supported
            results[storage] = impl
        except Exception, e:
            print "FATAL for %s (%s)" % (url, str(e))
            raise

# Pretty printing
out = open('checksum.out', 'w')

print >>out, "|| STORAGE || ",
for checksum in CHECKSUMS:
    print >>out, "%s || " % checksum,
print >>out

for storage in sorted(STORAGES.keys()):
    print >>out, "| %s | " % storage,
    for checksum in CHECKSUMS:
        print >>out, "%s | " % results.get(storage, dict()).get(checksum, '?'),
    print >>out

out.close()
