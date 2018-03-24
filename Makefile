subdirs = Reed-Solomon

binaries = Reed-Solomon/librs.a gen_pkt_table pkt_count gold_code dump_ts modem_send make_item32 make_test_pattern \
tsd_decode af_text af_i32 nrzi_encode nrzi_decode af_bin af_decode fcs_encode fcs_decode \
fx25_encode fx25_decode rs_decode fx25_decode2 nrzi_decode2

all: $(subdirs) $(binaries)

clean:
	@rm -f *~ *.o $(binaries) *.af test.bin
	$(MAKE) -C Reed-Solomon clean

Reed-Solomon/librs.a:
	$(MAKE) -C Reed-Solomon

gen_pkt_table: gen_pkt_table.o
	$(CC) -o gen_pkt_table gen_pkt_table.o

pkt_count: pkt_count.o packet_table.o
	$(CC) -o pkt_count pkt_count.o packet_table.o

gold_code: gold_code.o
	$(CC) -o gold_code gold_code.o

dump_ts: dump_ts.o
	$(CC) -o dump_ts dump_ts.o

modem_send: modem_send.o
	$(CC) -o modem_send modem_send.o

make_item32: make_item32.o
	$(CC) -o make_item32 make_item32.o

make_test_pattern: make_test_pattern.o
	$(CC) -o make_test_pattern make_test_pattern.o

tsd_decode: tsd_decode.o
	$(CC) -o tsd_decode tsd_decode.o

af_text: af_text.o afio.o
	$(CC) -o af_text af_text.o afio.o

af_i32.o: af_i32.c modem.h afio.h

af_i32: af_i32.o afio.o
	$(CC) -o af_i32 af_i32.o afio.o

afio.o: afio.c afio.h

nrzi_encode: nrzi_encode.o afio.o
	$(CC) -o nrzi_encode nrzi_encode.o afio.o

nrzi_decode.o: nrzi_decode.c modem.h afio.h

nrzi_decode: nrzi_decode.o afio.o
	$(CC) -o nrzi_decode nrzi_decode.o afio.o

nrzi_decode2.o: nrzi_decode2.c afio.h

nrzi_decode2: nrzi_decode2.o afio.o
	$(CC) -o nrzi_decode2 nrzi_decode2.o afio.o

af_bin: af_bin.o afio.o
	$(CC) -o af_bin af_bin.o afio.o

af_decode.o: af_decode.c afio.h

af_decode: af_decode.o afio.o
	$(CC) -o af_decode af_decode.o afio.o

fcs.o: fcs.c fcs.h

fcs_encode.o: fcs_encode.c fcs.h afio.h

fcs_encode: fcs_encode.o fcs.o afio.o
	$(CC) -o fcs_encode fcs_encode.o fcs.o afio.o

fcs_decode.o: fcs_decode.c fcs.h afio.h

fcs_decode: fcs_decode.o fcs.o afio.o
	$(CC) -o fcs_decode fcs_decode.o fcs.o afio.o

fx25_encode.o: fx25_encode.c afio.h afio.c rs.h

fx25tag.o: fx25tag.c fx25tag.h

fx25_encode: fx25_encode.o afio.o fx25tag.o Reed-Solomon
	$(CC) -o fx25_encode fx25_encode.o afio.o fx25tag.o -LReed-Solomon -lrs

fx25_decode.o: fx25_decode.c afio.h fx25tag.h modem.h

fx25_decode: fx25_decode.o afio.o fx25tag.o
	$(CC) -o fx25_decode fx25_decode.o afio.o fx25tag.o

rs_decode.o: rs_decode.c afio.h fx25tag.h

rs_decode: rs_decode.o afio.o fx25tag.o Reed-Solomon
	$(CC) -o rs_decode rs_decode.o afio.o fx25tag.o -LReed-Solomon -lrs

fx25_decode2.o: fx25_decode2.c afio.h fx25tag.h modem.h

fx25_decode2: fx25_decode2.o afio.o fx25tag.o
	$(CC) -o fx25_decode2 fx25_decode2.o afio.o fx25tag.o
