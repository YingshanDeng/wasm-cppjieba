DIR="build"
if [ ! -d "$DIR" ]; then
    ###  Create if $DIR does NOT exists ###
    echo "Create ${DIR} folder"
    mkdir ${DIR}
else
    echo "${DIR} folder exists"
fi

echo "Enter ${DIR} exec cmake .. && make"
cd build
cmake ..
make
