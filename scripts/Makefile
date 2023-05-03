CC=cc
all:mconf lxdialog/lxdialog conf
mconf:mconf.o zconf.tab.o 
conf:conf.o zconf.tab.o


zconf.tab.o: lex.zconf.c zconf.hash.c confdata.c


clean:
	rm -f *.o mconf conf
	rm -f lxdialog/*.o lxdialog/lxdialog
	
lxdialog/lxdialog:
	make -C lxdialog
