#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define LC_LONIG_PACKET_LEN 32
#define LC_ID_LEN 4
#define USER_ID_LEN 28

#define DATA_PACKET_LEN 4
#define DATA_PACKET_CRC32_LEN 4

/*
  Name  : CRC-32
  Poly  : 0x04C11DB7    x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 
                       + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
  Init  : 0xFFFFFFFF
  Revert: true
  XorOut: 0xFFFFFFFF
  Check : 0xCBF43926 ("123456789")
  MaxLen: 268 435 455 байт (2 147 483 647 бит) - обнаружение
   одинарных, двойных, пакетных и всех нечетных ошибок
*/
boost::uint32_t Crc32(const unsigned char *buf, unsigned len);

#endif // _PROTOCOL_H_
