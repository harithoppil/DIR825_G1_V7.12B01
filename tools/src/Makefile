
.PHONY: xmltool imgbuilder squashfs_tool testtools install sstrip uboot_mkimage mktree generatepin
CC=gcc
ALL : xmltool imgbuilder squashfs_tool testtools sstrip mktree generatepin
imgbuilder:
	make -C mkimage
uboot_mkimage:
	make -C uboot_mkimage
squashfs_tool:
#	patch -p1<squashfs/sqlzma2u-3.3.patch  -d ./squashfs/squashfs3.3/
	make -C sqlzma
xmltool:
	make -C mkxml
testtools:
	make -C testtools
sstrip:
	make -C sstrip	
mktree:
	make -C mktree
generatepin:
	make -C generatepin
#lzma:
#	mkdir -p lzma/lzma443
#	tar jxf lzma/lzma443.tar.bz2 -C lzma/lzma443
#	find lzma/lzma443 -name makefile.gcc | xargs dos2unix
#	make -C lzma/lzma443/C/7zip/Compress/LZMA_Alone/ -f makefile.gcc
install:testtools imgbuilder squashfs_tool xmltool sstrip uboot_mkimage mktree generatepin
	cp ./mkimage/mkimage ../bin
	cp sqlzma/mksquashfs ../bin
	cp sqlzma/lzma457/CPP/7zip/Compress/LZMA_Alone/lzma ../bin
	cp ./testtools/formatcsv ../bin
	cp ./sstrip/sstrip ../bin
	cp ./uboot_mkimage/mkimage ../bin/uboot_mkimage
	cp ./mktree/mktree ../bin
	cp ./generatepin/generatepin ../bin
clean:
#	rm -f ../bin/mkimage ../bin/mksquashfs ../bin/lzma *.o
#	rm -rf lzma/lzma443 squashfs/squashfs3.2-r2
	make -C mkimage clean
	make -C sqlzma clean
	make -C mkxml clean
	make -C testtools clean
	make -C sstrip clean
	make -C uboot_mkimage clean
	make -C mktree clean
	make -C generatepin clean

