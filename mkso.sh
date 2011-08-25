echo "-----------------make clean---------------\n\n"
make clean
echo "------------------  phpize  --------------\n\n"
phpize
echo "-----------------configure ---------------\n\n"
./configure --enable-dataserv
echo "-----------------    make  ---------------\n\n"
make
echo "-----------------make install---------------\n\n"
make install
#echo "-----------------apache restart-------------\n\n"
#/etc/init.d/apache2 restart
