#!/bin/env bash
DIR="$(dirname $(readlink -f $0))"
PREFIX="$(readlink -f ${DIR}/..)"

if [ $# -eq 0 ]; then
    echo "$(basename $0) -p <port> -c <ra_bootstrap_conf> [-l <ra_log_conf> -f <file_system> -d <work_dir>]" >&2
    exit 1
fi

set -x
LD_LIBRARY_PATH=${PREFIX}/lib:${PREFIX}/lib64 ${PREFIX}/bin/red_alert_app ${@}
