cd bsp
rm -rf html
doxygen doxygen.config
cd ../api
rm -rf html
doxygen doxygen.config
cd ..

