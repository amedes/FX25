#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

#include "rs.h"
#include "afio.h"
#include "fx25tag.h"

/*
Tag_01 0xB74DB7DF8A532F3E RS(255, 239) 16-byte check value, 239 information bytes
Tag_02 0x26FF60A600CC8FDE RS(144,128) - shortened RS(255, 239), 128 info bytes
Tag_03 0xC7DC0508F3D9B09E RS(80,64) - shortened RS(255, 239), 64 info bytes
Tag_04 0x8F056EB4369660EE RS(48,32) - shortened RS(255, 239), 32 info bytes
Tag_05 0x6E260B1AC5835FAE RS(255, 223) 32-byte check value, 223 information bytes
Tag_06 0xFF94DC634F1CFF4E RS(160,128) - shortened RS(255, 223), 128 info bytes
Tag_07 0x1EB7B9CDBC09C00E RS(96,64) - shortened RS(255, 223), 64 info bytes
Tag_08 0xDBF869BD2DBB1776 RS(64,32) - shortened RS(255, 223), 32 info bytes
Tag_09 0x3ADB0C13DEAE2836 RS(255, 191) 64-byte check value, 191 information bytes
Tag_0A 0xAB69DB6A543188D6 RS(192, 128) - shortened RS(255, 191), 128 info bytes
Tag_0B 0x4A4ABEC4A724B796 RS(128, 64) - shortened RS(255, 191), 64 info bytes
 */

#define BUF_SIZE 1024
#define FX25_DATA_SIZE (4 + 8 + 255 + 1) // preamble + co_tag + RS_code + postamble
#define RS_CODE_SIZE 255
#define PARITY_SYMBOLS 16 // number of parity symbols, 16, 32, 64

union CO_TAG {
  uint64_t val;
  uint8_t byte[sizeof(uint64_t)];
} tag = {
  .val = 0xB74DB7DF8A532F3E
};

#define AX25_FLAG 0x7e
#define FX25_FLAG 0x7e
#define FX25_PREAMBLE 4
#define FX25_POSTAMBLE 1

int fx25_encode(uint8_t fx25_data[], uint8_t buf[], int info_len, int parity)
{
  int rs_code_byte;
  int rs_info_byte;
  int index;
  int tag_no = 0;
  int i;
  uint8_t rs_buf[RS_CODE_SIZE];
  int offset;

  fx25tag_init();

  for (i = CO_TAG_01; i <= CO_TAG_0B; i++) {
    if (info_len <= tags[i].rs_info && parity == (tags[i].rs_code - tags[i].rs_info)) {
      tag_no = i;
    }
  }

  if (tag_no == 0) return -1;

  rs_code_byte = tags[tag_no].rs_code;
  rs_info_byte = tags[tag_no].rs_info;

  if (rs_init(RS_CODE_SIZE, RS_CODE_SIZE - parity) < 0) {
    fprintf(stderr, "rs_init error\n");
    return -1;
  }

  index = 0;
  memset(fx25_data, AX25_FLAG, FX25_DATA_SIZE);

  // preamble
  for (i = 0; i < FX25_PREAMBLE; i++) {
    fx25_data[index++] = FX25_FLAG;
  }

  // correlation tag
  for (i = 0;i < sizeof(tag); i++) {
    fx25_data[index++] = tags[tag_no].byte[i];
  }

  // calculate RS parity
  bzero(rs_buf, RS_CODE_SIZE - rs_code_byte);
  offset = RS_CODE_SIZE - rs_code_byte;
  memset(&rs_buf[offset], AX25_FLAG, rs_info_byte);

  // copy data to RS work
  for (i = 0; i < info_len; i++) {
    rs_buf[offset + i] = buf[i];
  }

  // generate RS parity
  if (rs_encode(rs_buf) < 0) {
    fprintf(stderr, "rs_encode error\n");
    return -1;
  }

  // copy RS code
  for (i = 0; i < rs_code_byte; i++) {
    fx25_data[index++] = rs_buf[offset + i];
  }

  // postamble
  for (i = 0; i < FX25_POSTAMBLE; i++) {
    fx25_data[index++] = FX25_FLAG;
  }

  return index;
}


int main(int argc, char *argv[])
{
  FILE *ifp = stdin;
  FILE *ofp = stdout;
  uint8_t buf[BUF_SIZE];
  uint8_t fx25_data[FX25_DATA_SIZE];
  int bit_length;
  int byte_size;
  int fx25_len;

  while ((bit_length = afio_read(ifp, buf, BUF_SIZE)) >= 0) {
    byte_size = (bit_length + 7) / 8;
    fx25_len = fx25_encode(fx25_data, buf, byte_size, PARITY_SYMBOLS);
    if (fx25_len > 0) afio_write(ofp, fx25_data, fx25_len, 0);
  }

  return 0;
}
