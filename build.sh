
function nyb
{
    mkdir -p build
    cd build
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
    make
    cd ..
}

function nyr
{
    mkdir -p bin
    cd bin
    ./nyas
    cd ..
}



