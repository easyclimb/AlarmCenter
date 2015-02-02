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


	typedef struct AdemcoData
	{
		char data[32];
		int GetLength() const { return strlen(data); }
		const char* GetBuffer() { return data; }
		AdemcoData(int acct, int ademco_event, int zone)
		{
			memset(data, 0, sizeof(data));
			data[0] = '[';
			data[1] = '#';
			_snprintf_s(&data[2], 5, 4, "%04d", acct);
			data[6] = '|';
			data[7] = '1';
			data[8] = '8';
			data[9] = ' ';
			//data[10] = IsCloseEvent(event) ? '3' : '1';
			_snprintf_s(&data[10], 5, 4, "%04d", ademco_event);
			data[14] = ' ';
			data[15] = '0';
			data[16] = '0';
			data[17] = ' ';
			_snprintf_s(&data[18], 4, 3, "%03d", zone);
			data[21] = ']';
			data[22] = 0;
		}
	}AdemcoData;

	typedef struct ADMCID
	{
		unsigned int	len;
		unsigned int	acct;
		unsigned char	mt;
		//unsigned char	q;
		unsigned int	ademco_event;
		unsigned char	gg;
		unsigned int	zone;
	}ADMCID;

	typedef struct AdemcoPrivateProtocal
	{
		const char* id;
		int id_len;
		const char* seq;
		int seq_len;
		const char* Rrcvr;
		int Rrcvr_len;
		const char* Lpref;
		int Lpref_len;
		const char* acct;
		int acct_len;
		char timestamp[24];
		char acct_machine[19];	// 9 bytes
		char passwd_machine[9];	// 8 bytes
		char phone[19];			// 9 bytes
		char ip_csr[16];		// 4 bytes
		ADMCID admcid;
		const char* xdata;
		int xdata_len;
		const char* private_cmd;
		int ademco_cmd_len;
		int private_cmd_len;
		char level;
		unsigned short port_csr;
	}AdemcoPrivateProtocal;

	typedef struct ConnID
	{
		char _1;
		char _2;
		char _3;
		ConnID(int conn_id) {
			FromInt(conn_id);
		}
		void FromInt(int conn_id) {
			_1 = LOBYTE(HIWORD(conn_id));
			_2 = HIBYTE(LOWORD(conn_id));
			_3 = LOBYTE(LOWORD(conn_id));
		}
	}ConnID;

	typedef struct PrivateCmd
	{
		static const int DEFAULT_CAPACITY = 32;
		char* _data;
		int _len;
		int _size;
		PrivateCmd() : _data(NULL), _len(0) {
			_len = 0;
			_size = DEFAULT_CAPACITY;
			_data = new char[_size];
		}
		~PrivateCmd() { delete[] _data; }
		void Append(char cmd) {
			if (_len + 1 == _size) {
				char* old_data = new char[_len];
				memcpy(old_data, _data, _len);
				_size *= 2;
				delete[] _data;
				_data = new char[_size];
				memcpy(_data, old_data, _len);
				delete old_data;
			}
			_data[++_len] = cmd;
		}
	}PrivateCmd;

	class PrivatePacket
	{
	private:
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
		PrivatePacket() { memset(this, 0, sizeof(this)); }
		void Make(char big_type, char lit_type, const char* cmd, int cmd_len);
		int GetLength() const { return sizeof(this) - 8 + _cmd._len; }
	};


	// 2014Äê11ÔÂ26ÈÕ 17:04:02 add
	inline DWORD MakeConnID(BYTE conn_id1, BYTE conn_id2, BYTE conn_id3)
	{
		return MAKELONG(MAKEWORD(conn_id3, conn_id2), MAKEWORD(conn_id1, 0));
	}

	const char* GetAdemcoEventString(int ademco_event);

	inline bool IsCloseEvent(int ademco_event)
	{
		return ademco_event == EVENT_ARM || ademco_event == EVENT_HALFARM;
	}

	inline bool IsStatusEvent(int ademco_event)
	{
		return ademco_event == EVENT_ARM || ademco_event == EVENT_HALFARM || ademco_event == EVENT_DISARM;
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

	bool ParseAdmCid(const char* pack, unsigned int pack_len, ADMCID& data);

	AttachmentReturnValue ParsePacket(const char* pack, unsigned int pack_len,
											 AdemcoPrivateProtocal& app, DWORD *lpBytesCommited,
											 BOOL deal_private_cmd = FALSE);

	int GenerateConnTestPacket(int conn_id, char* buff, int max_buff_len,
									  BOOL bResponce = TRUE, BOOL has_private_cmd = FALSE);

	int GenerateEventPacket(char* pack, int max_pack_len,
								   int ademco_id, LPCSTR acct,
								   int ademco_event, int zone, const char* psw = NULL,
								   BOOL has_private_cmd = FALSE, int conn_id = 0);

	int GenerateNullPacket(char* pack, int max_pack_len,
								  LPCSTR acct);


	int GenerateOnlinePackage(char* dst, size_t dst_len, int conn_id,
									 const char* csr_acct, size_t csr_acct_len);

	DWORD GenerateAckOrNakEvent(BOOL bAck, int conn_id, char* buff, int max_buff_len,
									   const char* acct, int acct_len, BOOL has_private_cmd = FALSE);

	int GenerateRegRspPackage(char* dst, size_t dst_len, int ademco_id);
};

//#include "ademco_func_implementation.h"

#endif /* ___ADEMCO_FUNC_H____ */