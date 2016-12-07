#pragma once

#define DEFAULT_LEN		64
#define USEING_LEN		DEFAULT_LEN

typedef void	(CALLBACK *LPCALLBACKCMD)(WCHAR *wstr);

class Inputer_W {
private:
	LPCALLBACKCMD lpcallbackcmd;
public:
	int len;
	int pos;
	WCHAR *wstr;
	Inputer_W() {
		len = USEING_LEN;
		wstr = new WCHAR[USEING_LEN];
		wstr[0] = 0;
		pos = 0;
	}
	~Inputer_W() { if (wstr) delete[] wstr; }

	inline void input(WCHAR wch)
	{
		if (wch >= 'A' && wch <= 'Z'
			|| wch >= 'a' && wch <= 'z'
			|| wch >= '0' && wch <= '9'
			|| wch == ' ')//'.':'¾'
		{
			if (pos < len - 1)
			{
				wstr[pos++] = wch;
				wstr[pos] = 0;
			}
		}
		else if (wch == '\b')
		{
			if (pos > 0)
			{
				wstr[--pos] = 0;
			}
		}
		else if (wch == '\r')
		{
			if (lpcallbackcmd)
			{
				WCHAR sendwstr[USEING_LEN] = { 0 };
				wcscpy_s(sendwstr, wstr);
				lpcallbackcmd(sendwstr);
			}
			clear();
		}
		else if (wch == 190)
		{
			if (pos < len - 1)
			{
				wstr[pos++] = '.';
				wstr[pos] = 0;
			}
		}
	}
	inline void clear()
	{
		wstr[0] = 0;
		//wcscpy(wstr, L"");
		pos = 0;
	}

	void WINAPI SetCallbackCMD(LPCALLBACKCMD pCallback);
};
