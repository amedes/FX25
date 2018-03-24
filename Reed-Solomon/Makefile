all: rstest gf11test librs.a

clean:
	@rm -f *~ *.o rstest gf11test librs.a

librs.a: gf.o rs.o poly.o
	ar -rcs librs.a gf.o rs.o poly.o

rstest: rstest.o poly.o rs.o gf.o
	$(CC) -o rstest rstest.o rs.o poly.o gf.o

gf11test: gf11test.o poly.o rs.o gf.o
	$(CC) -o gf11test gf11test.o rs.o poly.o gf.o

rstest.o: rstest.c gf.h poly.h rs.h

gf11test.o: gf11test.c gf.h poly.h rs.h

gf.o: gf.c gf.h gf2p8.h gf2p8.c gfp.h gfp.c

gf2p8.o: gf2p8.c gf2p8.h

gf11.o: gf11.c gf11.h

poly.o: poly.c poly.h gf.h

rs.o: rs.c rs.h gf.h poly.h

gfp.o: gfp.c gfp.h gf.h
