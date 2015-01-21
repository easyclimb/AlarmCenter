/*
* ademco_func.h
*/
#ifndef ___ADEMCO_FUNC_H____
#define ___ADEMCO_FUNC_H____

#pragma once

#include <math.h>

namespace Ademco
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
	//static const char* ACCT_M	= "18240888102";

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

	static const int EVENT_ARM				= 3400;
	static const int EVENT_DISARM			= 1400;
	static const int EVENT_HALFARM			= 3456;
	static const int EVENT_EMERGENCY		= 1120;
	static const int EVENT_BURGLAR			= 1130;
	static const int EVENT_FIRE				= 1110;
	static const int EVENT_DURESS			= 1121;
	static const int EVENT_GAS				= 1151;
	static const int EVENT_WATER			= 1113;
	static const int EVENT_TEMPER			= 1137;
	static const int EVENT_LOWBATTERY		= 1384;
	static const int EVENT_SOLARDISTURB		= 1387;
	static const int EVENT_DISCONNECT		= 1381;
	static const int EVENT_SERIAL485DIS		= 1485;
	static const int EVENT_SERIAL485CONN	= 3485;
	static const int EVENT_DOORRINGING		= 1134;

	static const int gc_AdemcoEvent[] = {
		EVENT_ARM,
		EVENT_DISARM,
		EVENT_HALFARM,
		EVENT_EMERGENCY,
		EVENT_BURGLAR,
		EVENT_FIRE,
		EVENT_DURESS,
		EVENT_GAS,
		EVENT_WATER,
		EVENT_TEMPER,
		EVENT_LOWBATTERY,
		EVENT_SOLARDISTURB,
		EVENT_DISCONNECT,
		EVENT_SERIAL485DIS,
		EVENT_SERIAL485CONN,
		EVENT_DOORRINGING,
	};


	typedef struct _AdemcoData
	{
		char data[32];
		int GetLength() const { return strlen(data); }
		const char* GetBuffer() { return data; }
		_AdemcoData(int acct, int ademco_event, int zone)
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

	enum AttachmentReturnValue
	{
		ARV_OK,
		ARV_PACK_NOT_ENOUGH,
		ARV_PACK_DATA_ERROR,
	};

	class CAdemcoFunc
	{
	private:
		CAdemcoFunc() {}
	public:

		// 2014Äê11ÔÂ26ÈÕ 17:04:02 add
		static inline DWORD MakeConnID(BYTE conn_id1, BYTE conn_id2, BYTE conn_id3)
		{
			return MAKELONG(MAKEWORD(conn_id3, conn_id2), MAKEWORD(conn_id1, 0));
		}

		static const char* GetAdemcoEventString(int ademco_event);

		static inline bool IsCloseEvent(int ademco_event)
		{
			return ademco_event == EVENT_ARM || ademco_event == EVENT_HALFARM;
		}

		static inline bool IsStatusEvent(int ademco_event)
		{
			return ademco_event == EVENT_ARM || ademco_event == EVENT_HALFARM || ademco_event == EVENT_DISARM;
		}

		static inline int GetDecDigits(int dec)
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
		static inline int HexChar2Dec(char hex);

		// e.g.
		// input: "10"
		// output: 16
		static int HexCharArrayToDec(const char *hex, int len);

		// e.g.
		// input: "10"
		// output: 10
		static int NumStr2Dec(const char* str, int str_len);

		// e.g.
		// input: "10"
		// output: "16"
		static const char* HexCharArrayToStr(const char* hex, int len, unsigned char mask = (char)0x0f);

		// like upper, but cat result to dst. 
		static const char* HexCharArrayToStr(char* dst, const char* hex, int len,
											 unsigned char mask = (char)0x0f);

		// format number to hex char array(max dec is 0x0fff
		// e.g.
		// input: 16
		// output: 0010
		static void Dec2HexCharArray_4(int dec, char* hex, bool bMax0FFF = true);

		// format phone number to hex char array(N bytes)
		// e.g.
		// input: "18240888101"
		// output: 18 24 08 88  10 1f ff ff  ff
		static void NumStr2HexCharArray_N(const char* str, char* hexarr, int max_hex_len = 9);

		static unsigned int CalculateCRC(const char* buff, int len);

		static bool is_null_data(const char* id, unsigned int len)
		{
			return (strncmp(AID_NULL, id, min(len, strlen(AID_NULL))) == 0);
		}

		static bool ParseAdmCid(const char* pack, unsigned int pack_len, ADMCID& data);

		static AttachmentReturnValue ParsePacket(const char* pack, unsigned int pack_len,
												 AdemcoPrivateProtocal& app, DWORD *lpBytesCommited,
												 BOOL deal_private_cmd = FALSE);

		static int GenerateConnTestPacket(int conn_id, char* buff, int max_buff_len,
										  BOOL bResponce = TRUE, BOOL has_private_cmd = FALSE);

		static int GenerateConnIDPacket(const AdemcoPrivateProtocal& app,
										char* buff, int max_buff_len);

		static int GenerateEventPacket(char* pack, int max_pack_len,
									   int ademco_id, LPCSTR acct,
									   int ademco_event, int zone, const char* psw = NULL,
									   BOOL has_private_cmd = FALSE, int conn_id = 0);

		static int GenerateNullPacket(char* pack, int max_pack_len,
									   LPCSTR acct);


		static int GenerateOnlinePackage(char* dst, size_t dst_len, int conn_id,
										 const char* csr_acct, size_t csr_acct_len);

		static DWORD GenerateAckOrNakEvent(BOOL bAck, int conn_id, char* buff, int max_buff_len,
										   const char* acct, int acct_len, BOOL has_private_cmd = FALSE);

		static int GenerateRegRspPackage(char* dst, size_t dst_len, int ademco_id);
	};
};

//#include "ademco_func_implementation.h"

#endif /* ___ADEMCO_FUNC_H____ */