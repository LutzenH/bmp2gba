wget https://justine.lol/cosmopolitan/cosmopolitan-amalgamation-2.0.1.zip
unzip cosmopolitan-amalgamation-2.0.1.zip
gcc -g -Ofast -static -nostdlib -nostdinc -fno-pie -no-pie -mno-red-zone \
  -fno-omit-frame-pointer -pg -mnop-mcount -mno-tls-direct-seg-refs \
  -DB2G_APE -o bmp2gba.com.dbg src/b2g_main.c src/b2g_types.h src/b2g_builder.c src/b2g_builder.h src/b2g_platform.c src/b2g_platform.h -fuse-ld=bfd -Wl,-T,ape.lds -Wl,--gc-sections \
  -include cosmopolitan.h -Ilib/cosmopolitan/libc/isystem -Ilib/cosmopolitan crt.o ape-no-modify-self.o cosmopolitan.a -Ilib -Ilib/STC/include
objcopy -S -O binary bmp2gba.com.dbg bmp2gba.com
rm ape-copy-self.o ape-no-modify-self.o ape.lds ape.o bmp2gba.com.dbg cosmopolitan-amalgamation-2.0.1.zip cosmopolitan.a cosmopolitan.h crt.o
