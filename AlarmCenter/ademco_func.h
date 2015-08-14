/*
* ademco_func.h
*/
#ifndef ___ADEMCO_FUNC_H____
#define ___ADEMCO_FUNC_H____

#pragma once

#include <math.h>
#include "ademco_event.h"

namespace ademco
{
	static const int AID_NUM = 8;
	static const char* AID_NULL = "\"NULL\"";
	static const char* AID_ACK = "\"ACK\"";
	static const char* AID_NAK = "\"NAK\"";
	static const char* AID_DAH = "\"DAH\"";
	static const char* AID_HB = "\"HENG-BO\"";
	static const char* AID_MODULE_REG = "\"MODULE-REG\"";
	static const char* AID_REG_RSP = "\"REG-RSP\"";
	static const char* AID_PWW = "\"PWW\"";

	static const char* SEQ = "0000";
	static const char* RRCVR = "R1234";
	static const char* LPREF = "L6789";
	static const char* ACCOUNT = "18240888101";
	//const char* ACCT_M	= "18240888102";

	static const char* g_aid[AID_NUM] = {
		AID_NULL,
		AID_ACK,
		AID_NAK,
		AID_DAH,
		AID_HB,
		AID_MODULE_REG,
		AID_REG_RSP,
		AID_PWW,
	};

	typedef struct AdemcoDataSegment
	{
		bool			_valid;
		char			_data[32];
		unsigned int	_len;
		unsigned int	_ademco_id;
		//unsigned char	_mt;
		unsigned int	_ademco_event;
		unsigned char	_gg;
		unsigned int	_zone;

		AdemcoDataSegment() { memset(this, 0, sizeof(AdemcoDataSegment)); }

		// maker
		void Make(int ademco_id, int gg, int ademco_event, int zone);

		void Make() { memset(this, 0, sizeof(AdemcoDataSegment)); strcpy_s(_data, "[]"); _len = 2; }

		// parser
		bool Parse(const char* pack, unsigned int pack_len);
	}AdemcoDataSegment;

	typedef struct AdemcoTimeStamp
	{
		char _data[32];
		int _len;
		time_t _time;

		AdemcoTimeStamp() { memset(this, 0, sizeof(AdemcoTimeStamp)); }

		void Make();

		bool Parse(const char* pack, unsigned int pack_len);
	}AdemcoTimeStamp;

	typedef struct AdemcoPacket
	{
		static const char _LF = 0x0A;
		char _crc[5];
		char _len[5];
		char _id[32];
		char _seq[5];
		char _rrcvr[16];
		char _lpref[16];
		char _acct[16];
		AdemcoDataSegment _data;
		char *_xdata;
		int _xdata_len;
		AdemcoTimeStamp _timestamp;
		static const char _CR = 0x0D;

		AdemcoPacket() : _xdata(NULL) { Clear(); }
		~AdemcoPacket() { if (_xdata) delete[] _xdata; }

		void Clear() { if (_xdata) delete[] _xdata; memset(this, 0, sizeof(AdemcoPacket)); }

		size_t GetLength() const;

		void CopyData(char* dst, size_t length);

		// maker
		size_t Make(char* pack, size_t pack_len, const char* id,
					int seq, /*char const* acct, */int ademco_id,
					int ademco_event, int gg, int zone,
					const char* xdata, int xdata_len);

		// parser
		ParseResult Parse(const char* pack, size_t pack_len, size_t& cbCommited);
	}AdemcoPacket;

	typedef struct ConnID
	{
		char _1;
		char _2;
		char _3;

		ConnID() : _1(-1), _2(-1), _3(-1) {}

		ConnID(int conn_id) {
			FromInt(conn_id);
		}

		void FromInt(int conn_id) {
			_1 = LOBYTE(HIWORD(conn_id));
			_2 = HIBYTE(LOWORD(conn_id));
			_3 = LOBYTE(LOWORD(conn_id));
		}

		void FromCharArray(char arr[3])
		{
			_1 = arr[0];
			_2 = arr[1];
			_3 = arr[2];
		}

		int ToInt()
		{
			return MAKELONG(MAKEWORD(_3, _2), MAKEWORD(_1, 0));
		}
	}ConnID;

	typedef struct PrivateCmd
	{
		static const size_t DEFAULT_CAPACITY = 64;
		char* _data;
		size_t _size;
		size_t _capacity;
		
		PrivateCmd() : _data(NULL), _size(0), _capacity() {}
		
		~PrivateCmd() { Clear(); }
		
		void Clear() { if (_data) { delete[] _data; } memset(this, 0, sizeof(PrivateCmd)); }
		
		void Expand() {
			if (_data == NULL) {
				_size = 0;
				_capacity = DEFAULT_CAPACITY;
				_data = new char[_capacity];
			}

			if (_size < _capacity) return;

			char* old_data = _data;
			_data = new char[_capacity <<= 1];
			memcpy(_data, old_data, _size);
			delete[] old_data;
		}
		
		void Append(char cmd) {
			Expand();
			_data[_size++] = cmd;
		}

		void Append(const char* cmd, size_t cmd_len)
		{
			for (size_t i = 0; i < cmd_len; i++) {
				Append(cmd[i]);
			}
		}

		void AppendConnID(const ConnID& connID) {
			Append(connID._1);
			Append(connID._2);
			Append(connID._3);
		}
		
		void Assign(const char* cmd, size_t cmd_len) {
			Clear();
			for (size_t i = 0; i < cmd_len; i++) {
				Append(cmd[i]);
			}
		}

		PrivateCmd& operator=(const PrivateCmd& rhs) {
			Clear();
			for (size_t i = 0; i < rhs._size; i++) {
				Append(rhs._data[i]);
			}
			return *this;
		}

		ConnID GetConnID() const
		{
			ConnID id;
			if (_size >= 3) {
				id.FromCharArray(_data);
			}
			return id;
		}
	}PrivateCmd;

	class PrivatePacket
	{
	public:
		char _len[2];
		char _acct_machine[9];
		char _passwd_machine[4];
		char _acct[9];
		char _level;
		char _ip_csr[4];
		char _port_csr[2];
		char _big_type;
		char _lit_type;
		PrivateCmd _cmd;
		char _crc[4];
	public:
		PrivatePacket() { memset(this, 0, sizeof(PrivatePacket)); }
		size_t GetLength() const;
		size_t Make(char* pack, size_t pack_len, char big_type, char lit_type, 
					const PrivateCmd& cmd);
		ParseResult Parse(const char* pack, size_t pack_len, size_t& cbCommited);
	protected:
		void CopyData(char* dst, size_t length);
	};

	inline bool IsCloseEvent(int ademco_event)
	{
		return ademco_event == EVENT_ARM 
			|| ademco_event == EVENT_HALFARM;
	}

	inline bool IsStatusEvent(int ademco_event)
	{
		return ademco_event == EVENT_ARM 
			|| ademco_event == EVENT_HALFARM 
			|| ademco_event == EVENT_DISARM;
	}

	inline int GetDecDigits(int dec)
	{
		int digits = 0;
		while (dec != 0) {
			dec /= 10;
			digits++;
		}
		return digits;
	}



	// format ascii charactor to integer value.
	// e.g. 
	// input: '0'
	// output: 0
	int HexChar2Dec(char hex);

	// e.g.
	// input: 10
	// output: 'A'
	char Dec2Hex(char d);
	// e.g.
	// input: "10"
	// output: 16
	int HexCharArrayToDec(const char *hex, int len);

	// e.g.
	// input: "10"
	// output: 10
	int NumStr2Dec(const char* str, int str_len);

	// e.g.
	// input: "10"
	// output: "16"
	const char* HexCharArrayToStr(const char* hex, int len, unsigned char mask = (char)0x0f);

	// like upper, but cat result to dst. 
	const char* HexCharArrayToStr(char* dst, const char* hex, int len,
										 unsigned char mask = (char)0x0f);

	// format number to hex char array(max dec is 0x0fff
	// e.g.
	// input: 16
	// output: 0010
	void Dec2HexCharArray_4(int dec, char* hex, bool bMax0FFF = true);

	// format phone number to hex char array(N bytes)
	// e.g.
	// input: "18240888101"
	// output: 18 24 08 88  10 1f ff ff  ff
	void NumStr2HexCharArray_N(const char* str, char* hexarr, int max_hex_len = 9);

	unsigned short CalculateCRC(const char* buff, int len, unsigned short crc = 0);

	static bool is_null_data(const char* id, unsigned int len)
	{
		return (strncmp(AID_NULL, id, min(len, strlen(AID_NULL))) == 0);
	}

	static bool is_null_data(const char* id)
	{
		return (strcmp(AID_NULL, id) == 0)
			|| (strcmp(AID_ACK, id) == 0);
	}

};

//#include "ademco_func_implementation.h"

#endif /* ___ADEMCO_FUNC_H____ */