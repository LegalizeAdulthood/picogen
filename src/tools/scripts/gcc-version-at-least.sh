g++ gcc-version-at-least.cc -o GCC-VERSION-AT-LEAST.bin > /dev/null
./GCC-VERSION-AT-LEAST.bin $1 $2
ret=$?
rm ./GCC-VERSION-AT-LEAST.bin
exit $ret