CC= vc '+ninec'
CFLAGS= -O2
BINS=basename cal cat cksum cmp comm crc cut echo expand grep ln ls \
	mkdir oldgrep oldls pwd rm sh try usertests wc roff od \
	less cp mv head tail

all: $(BINS)
	cp $(BINS) ../../Build/bin

clean:
	rm -f $(BINS) *.o *.s map bla
