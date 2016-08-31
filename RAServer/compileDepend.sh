#!/bin/bash
backPwd="$(pwd)"
curDir="$(cd `dirname $0`; pwd)"
installroot="${curDir}/_external"
if [ $# != 0 ]
then
    installroot="$1"
fi

echo -e "\n"
echo "=========================================================================="
echo "compiling libevent"
cd "${curDir}/deps/libevent" && ./configure --prefix="$installroot" CFLAGS='-g -fPIC' && make && make install
if [ $? != 0 ] 
then
    echo "Fail to compile libevent"
    exit 1
fi

echo -e "\n\n\n"
echo "=========================================================================="
echo "compiling curl"
cd "${curDir}/deps/curl" && ./configure --prefix="$installroot" CFLAGS='-g -fPIC' && make -C lib install && make -C include install
if [ $? != 0 ] 
then
    echo "Fail to compile curl"
    exit 1
fi

echo -e "\n\n\n"
echo "=========================================================================="
echo "compiling jansson"
cd "${curDir}/deps/jansson" && autoreconf -fi && aclocal && ./configure  --prefix="$installroot" CFLAGS='-g -fPIC' && make install
if [ $? != 0 ] 
then
    echo "Fail to compile jansson"
    exit 1
fi

echo -e "\n\n\n"
echo "=========================================================================="
echo "compiling glog"
cd "${curDir}/deps/glog" && autoreconf -fi && aclocal && ./configure  --prefix="$installroot" CFLAGS='-g -fPIC' && make install
if [ $? != 0 ] 
then
    echo "Fail to compile glog"
    exit 1
fi

cd "$backPwd"

