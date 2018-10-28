build_dir=build
client_dir=/usr/sap/hdbclient
header_dir=/usr/sap/hdbclient/sdk/odbc/incl

if [ ! -d "$client_dir" ]; then
   echo "please install hana client"
   exit
fi
if [ ! -d "$build_dir" ]; then
    mkdir -p $build_dir
fi
export LIBRARY_PATH="${client_dir}:$LIBRARY_PATH"
if [ -d "$build_dir" ]; then
    cp -rf source/* $build_dir
    cd $build_dir
    cp -rf $header_dir/* common/odbc/
    #export LIBRARY_PATH="./common/odbc:$LIBRARY_PATH"
    phpize && ./configure
    make && make install
fi

