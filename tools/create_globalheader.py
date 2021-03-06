#!/usr/bin/env python3
#
# Copyright (C) 2014 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Jussi Pakkanen <jussi.pakkanen@canonical.com>
#              Michi Henning <michi.henning@canonical.com>

from glob import glob
import sys, os

def build_header(outfile, prefix, incroots):
    ofile = open(outfile, 'w')
    ofile.write("#pragma once\n\n")
    headers = []
    for r in incroots:
        headers += glob(os.path.join(r, '*.h'))
    headers = [os.path.split(f)[1] for f in headers]
    headers.sort()
    for f in headers:
        line = '#include <%s>\n' % os.path.join(prefix, f)
        ofile.write(line)

if __name__ == '__main__':
    if len(sys.argv) <= 3:
        print(sys.argv[0], 'outfile prefix include_roots')
        sys.exit(1)
    outfile = sys.argv[1]
    prefix = sys.argv[2]
    incroots = sys.argv[3:]
    build_header(outfile, prefix, incroots)
