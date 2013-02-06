#!/usr/bin/env python
import argparse
import os
import sys
import sqlite

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Plot contours of PSF quality')

    parser.add_argument('registryFile', type=str, help="The registry in question")
    
    args = parser.parse_args()

    if os.path.exists(args.registryFile) and not os.path.isdir(args.registryFile):
        registryFile = args.registryFile
    else:
        registryFile = os.path.join(args.registryFile, "registry.sqlite3")
        if not os.path.exists(registryFile):
            print >> sys.stderr, "Unable to open %s" % args.registryFile
            sys.exit(1)

    conn = sqlite.connect(registryFile)
    cursor = conn.cursor()
    print "%-7s %-20s %6s %7s %3s" % ("filter", "field", "visit", "expTime", "nCCD")

    query = """
SELECT field, visit, filter, expTime, count(ccd)
FROM raw
GROUP BY visit
ORDER BY filter
"""
    for line in cursor.execute(query):
        field, visit, filter, expTime, nCCD = line

        print "%-7s %-20s %6d %7.1f %3d" % (filter, field, visit, expTime, nCCD)

    conn.close()
