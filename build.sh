echo "this is to build shaomin zhang's car2car:"

chmod 777 . -Rf
cd ./car
make
mv car ../bin
cd ../gui
make
mv observer simulator test ../bin
cd ../bin

echo "building finished, now try to execute:"
./observer

