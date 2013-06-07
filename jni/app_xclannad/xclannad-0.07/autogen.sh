#!/bin/sh
autoheader259
aclocal19 -I . -I /usr/local/share/aclocal
autoconf259
# gcc 3.0.2 用の修正
# gtkmm の autoconf が変でエラーが出るので
# 強制的にエラーの元になる行を削除
#sed -e 's/^extern "C" void exit(int);//' -e "s/',//" configure > configure.new
#cp configure.new configure
#rm -f configure.new
./configure $*
