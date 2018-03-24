#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rs.h"
#include "fx25tag.h"

#define BUF_SIZE 1024
#define TAG_BYTE_LEN 8
#define RS_CODE_SIZE 255

int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  FILE *efp;
  uint8_t buf[BUF_SIZE];
  uint8_t rs_buf[RS_CODE_SIZE];
  int bit_length;
  int byte_length;
  uint64_t fx25tag;
  int tag_no;
  int offset;
  int i;
  int rs_code_byte, rs_info_byte;
  int err;

  // init tag
  fx25tag_init();

  efp = fopen("rs_decode_err.af", "w");

  while ((bit_length = afio_read(ifp, buf, BUF_SIZE)) >= 0) {
    byte_length = (bit_length + 7) / 8;
    if (byte_length < TAG_BYTE_LEN) {
      fprintf(stderr, "correlation tag not found\n");
      continue;
    }

    memcpy(&fx25tag, buf, sizeof(uint64_t));

    // find tag
    tag_no = 0;
    for (i = CO_TAG_01; i <= CO_TAG_0B; i++) {
      if (fx25tag == tags[i].tag) {
	tag_no = i;
	break;
      }
    }
    if (tag_no == 0) {
      fprintf(stderr, "TAG value not match: %016llX\n", fx25tag);
      continue;
    }

    rs_code_byte = tags[i].rs_code;
    rs_info_byte = tags[i].rs_info;

    if (byte_length != rs_code_byte + TAG_BYTE_LEN) {
      fprintf(stderr, "packet length wrong\n");
      continue;
    }

    if (rs_init(RS_CODE_SIZE, RS_CODE_SIZE - (rs_code_byte - rs_info_byte))) {
      fprintf(stderr, "rs_init error\n");
      exit(1);
    }
    
    // RS decode
    bzero(rs_buf, RS_CODE_SIZE);
    offset = RS_CODE_SIZE - rs_code_byte;
    memcpy(&rs_buf[offset], &buf[TAG_BYTE_LEN], rs_code_byte);

    if ((err = rs_decode(rs_buf)) < 0) {
      fprintf(stderr, "rs_decode error\n");
      afio_write(efp, &buf[TAG_BYTE_LEN], rs_code_byte, 0);
      fflush(efp);
      continue;
    }
    if (err > 0) {
      fprintf(stderr, "rs_decode: %d errors corrected\n", err);
      afio_write(efp, &buf[TAG_BYTE_LEN], rs_code_byte, 0);
      fflush(efp);
    }

    afio_write(ofp, &rs_buf[offset], rs_info_byte, 0);
  }

  fclose(efp);

  return 0;
}
