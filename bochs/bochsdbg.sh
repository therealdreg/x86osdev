#!/bin/bash

set -x

echo "by David Reguera Garcia aka Dreg dreg@fr33project.org"
echo "https://github.com/therealdreg https://www.fr33project.org @therealdreg"
SCRIPT_PATH=`dirname "$0"`; SCRIPT_PATH=`eval "cd \"$SCRIPT_PATH\" && pwd"`
cd $SCRIPT_PATH
rm -f *.lock
rm -f bx_enh_dbg.ini
bochs