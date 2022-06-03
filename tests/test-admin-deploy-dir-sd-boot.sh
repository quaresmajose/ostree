#!/bin/bash
#
# Copyright (C) 2011,2014 Colin Walters <walters@verbum.org>
# Copyright (C) 2013 Javier Martinez <javier.martinez@collabora.co.uk>
#
# SPDX-License-Identifier: LGPL-2.0+
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

set -euo pipefail

. $(dirname $0)/libtest.sh

# Exports OSTREE_SYSROOT so --sysroot not needed.
kver="3.6.0"
modulesdir="usr/lib/modules/${kver}"
setup_os_repository "archive" "sdboot" ${modulesdir}

find sysroot

extra_admin_tests=1

. $(dirname $0)/admin-test-dir.sh

assert_has_file sysroot/boot/loader/loader.conf
cat << 'EOF' > sysroot/boot/loader/loader.conf
timeout 10
editor no
EOF
${CMD_PREFIX} ostree admin deploy --karg=root=LABEL=MOO --karg=quiet --os=testos testos:testos/buildmain/x86_64-runtime
assert_file_has_content sysroot/boot/loader/loader.conf "timeout"
assert_file_has_content sysroot/boot/loader/loader.conf "editor"

echo "ok deploying loader.conf"
