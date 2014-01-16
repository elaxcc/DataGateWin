#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define LC_LONIG_PACKET_LEN 32
#define LC_ID_LEN 4
#define USER_ID_LEN 28

#define DATA_PACKET_LEN 4
#define DATA_PACKET_CRC32_LEN 4

const std::string g_hs_login_answer = "urwerhHELLOWdjdn";

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

class hc_to_lc_parser
{
public:
	hc_to_lc_parser();
	~hc_to_lc_parser();

	bool is_complete() { return is_complete_; }
	bool is_bad_packet() { return is_bad_packet_; }
	void parse(std::vector<char> data);
	void reset();
	void flush();

	const std::vector<char>& get_lc_id() { return lc_id_; }
	const std::vector<char>& get_data() { return data_; }

	static void prepare_data_for_hs(const std::vector<char>& lc_id_,
		const std::vector<char>& data, std::vector<char>& out_buffer);

private:
	bool is_complete_;
	bool is_bad_packet_;

	std::vector<char> lc_id_;
	unsigned int data_len_;
	std::vector<char> data_;
	unsigned int crc_;

	bool got_lc_id_;
	bool got_data_len_;
	bool got_data_;
	bool got_crc_;

	std::vector<char> buffer_;
	std::vector<char> buffer_all_data_;
};

#endif // _PROTOCOL_H_
