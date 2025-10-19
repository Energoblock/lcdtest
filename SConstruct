# SConstruct for lcdtest
# $Id: SConstruct 62 2010-01-27 07:29:43Z eric $
# Copyright 2004, 2005, 2006, 2009, 2010 Eric Smith <eric@brouhaha.com>

# lcdtest is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.  Note that I am not
# granting permission to redistribute or modify lcdtest under the
# terms of any later version of the General Public License.

# lcdtest is distributed in the hope that they will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

release = '1.18'

conf_file = 'lcdtest.conf'

#-----------------------------------------------------------------------------
# Options
#-----------------------------------------------------------------------------

AddOption ('--prefix',
           dest = 'prefix',
           type = 'string',
           nargs = 1,
           action = 'store',
           metavar = 'DIR',
           help = 'installation path prefix (absolute)',
           default = '/usr/local')

AddOption ('--bindir',
           dest = 'bindir',
           type = 'string',
           nargs = 1,
           action = 'store',
           metavar = 'DIR',
           help = 'binary executable directory (relative to prefix)',
           default = 'bin')

AddOption ('--mandir',
           dest = 'mandir',
           type = 'string',
           nargs = 1,
           action = 'store',
           metavar = 'DIR',
           help = 'manual page directory (relative to prefix)',
           default = 'share/man')

AddOption ('--datadir',
           dest = 'datadir',
           type = 'string',
           nargs = 1,
           action = 'store',
           metavar = 'DIR',
           help = 'application data directory (relative to prefix)',
           default = 'share')

AddOption ('--buildroot',
           dest = 'buildroot',
           type = 'string',
           nargs = 1,
           action = 'store',
           metavar = 'DIR',
           help = 'installation virtual root directory (for packaging)',
           default = '')

env = Environment (RELEASE = release,
                   PREFIX = GetOption ('prefix'),
                   BINDIR = GetOption ('bindir'),
                   MANDIR = GetOption ('mandir'),
                   DATADIR = GetOption ('datadir'),
                   BUILDROOT = GetOption ('buildroot'))
Export ('env')

#-----------------------------------------------------------------------------
# Add some builders to the environment:
#-----------------------------------------------------------------------------
import sys
import os

SConscript ('scons/tarball.py')

#-----------------------------------------------------------------------------
# package a release source tarball
#-----------------------------------------------------------------------------

bin_dist_files = Split ("""README COPYING""")

src_dist_files = Split ("""SConstruct INSTALL""")

source_release_tarball = env.Tarball ('lcdtest-' + release,
                                      src_dist_files + bin_dist_files)

env ['source_release_tarball'] = source_release_tarball

env.Alias ('srcdist', source_release_tarball)

#-----------------------------------------------------------------------------
# package a source snapshot tarball
#-----------------------------------------------------------------------------

import time

snap_date = time.strftime ("%Y.%m.%d")

source_snapshot_tarball = env.Tarball ('lcdtest-' + snap_date,
                                       src_dist_files)

env ['source_snapshot_tarball'] = source_snapshot_tarball

env.Alias ('srcsnap', source_snapshot_tarball)

#-----------------------------------------------------------------------------
# code
#-----------------------------------------------------------------------------

build_dir = 'build/'

SConscript ('src/SConscript',
            build_dir = build_dir,
            duplicate = 0,
	    exports = {'env' : env})

#-----------------------------------------------------------------------------
# man page
#-----------------------------------------------------------------------------

SConscript ('man/SConscript')

#-----------------------------------------------------------------------------
# desktop environment files
#-----------------------------------------------------------------------------

SConscript ('desktop/SConscript',
	   build_dir = build_dir + '/desktop',
	   duplicate = 0)

#-----------------------------------------------------------------------------
# scons directory, which contains various scons builders we use, and the
# scons-local tarball for those that don't want to install SCons
#-----------------------------------------------------------------------------

SConscript ('scons/SConscript')
