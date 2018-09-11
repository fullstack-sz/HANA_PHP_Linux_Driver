build_dir=build
if [ ! -d "$build_dir" ]; then
    mkdir -p $build_dir
fi


if [ -d "$build_dir" ]; then
    cp -rf source/* $build_dir
    cd $build_dir
    #export LIBRARY_PATH="./common/odbc:$LIBRARY_PATH"
    phpize && ./configure
    make && make install
fi
