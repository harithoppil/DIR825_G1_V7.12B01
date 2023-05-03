# Create device files
cd ${FSROOT}
rm dev -rf
mkdir dev
cd dev
#usb node directory
mkdir usb
for i in $(seq 0 4)
do
	mknod -m 666 ttyp$i c 3 $i
done

for i in $(seq 0 4)
do
	mknod -m 666 ptyp$i c 2 $i
done


for i in $(seq 0 5)
do
mknod -m 664 mtd$i c 90 $(expr $i + $i)
mknod -m 664 mtdblock$i b 31 $i
done

mknod -m 600 mem    c  1 1
mknod -m 666 null   c  1 3
mknod -m 666 zero   c  1 5
mknod -m 644 random c  1 8
mknod -m 600 tty0   c  4 0
mknod -m 600 tty1   c  4 1
mknod -m 600 tty5   c  4 1
mknod -m 600 ttyS0  c  4 64
mknod -m 666 tty    c  5 0
mknod -m 600 console  c  5 1
mknod -m 600 ppp c 108 0
mknod -m 666 urandom c 1 9
mknod -m 666 mxp c 241 0
mknod -m 666 tiuhw c 240 0
mknod -m 666 led c 242 0
mknod -m 600 dk0 c 63 0
mknod -m 600 dk1 c 63 1
mknod -m 600 ttyUSB0 c 188 0
mknod -m 600 ttyUSB1 c 188 1
mknod -m 600 ttyUSB2 c 188 2
mknod -m 600 ttyUSB3 c 188 3
mknod -m 600 ttyUSB4 c 188 4
mknod -m 600 ttyUSB5 c 188 5
mknod -m 600 ttyUSB6 c 188 6
mknod -m 600 ttyUSB7 c 188 7
mknod -m 600 ttyACM0 c 166 0
mknod -m 600 ttyACM1 c 166 1
mknod -m 600 ttyACM2 c 166 2
mknod -m 600 ttyACM3 c 166 3
mknod -m 600 ttyACM4 c 166 4
mknod -m 600 ttyACM5 c 166 5
mknod -m 600 ttyACM6 c 166 6
mknod -m 600 ttyACM7 c 166 7

#mkdir pty
#for i in $(seq 0 31)
#do
#mknod -m 666 pty/m$i c 2 $i 
#done
mknod -m 666 ptmx c 5 2
mkdir pts
for i in $(seq 0 255)
do
mknod -m 622 pts/$i c 136 $i
done
#mkdir tts
#mknod -m 666 tts/0 c 4 64

mknod -m 666 dci0 c 121 0
mknod -m 666 csm0 c 122 0
mknod -m 666 adm0 c 126 0
mknod -m 666 adm1 c 126 1
mknod -m 666 sch0 c 126 2
mknod -m 666 sch1 c 126 3
mknod -m 666 m821xx0 c 246 0
mknod -m 666 m83xxx0 c 247 0
mknod -m 666 m829xx0 c 245 0
#mknod -m 666 sti c 242 0
mknod -m 666 wan0 c 125 0
mknod -m 666 pri c 251 0
mknod -m 666 bt8370_0 c 124 0
#mknod -m 666 bt847x c 240 0
mknod -m 666 gateway c 120 0

mknod -m 666 fuse c 10 229
mkdir misc
mknod -m 666 misc/fuse c 10 229

ln -s ../var/devlog log 
exit

