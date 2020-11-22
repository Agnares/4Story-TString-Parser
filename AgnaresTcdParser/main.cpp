#include <afx.h>
#include <conio.h>
#include <deque>
#include <string>
#include <fstream>
#include <sstream>
#include <tuple>

#define STRFILE_OPEN		_T(".\\TString.tcd")
#define STRFILE_SAVE		_T(".\\TString.txt")
#define STRFILE_SAVE_EDIT   _T(".\\TString_new.tcd")

struct sTcd
{
	WORD wStringID;
	CString strString;
};

class cParser
{
public:
	cParser(const wchar_t* filenameopen, const wchar_t* filenamesave);
	~cParser();
	void ReadTcd();
	void ReadTxt();

private:
	void OutputData(BOOL bType);

private:
	// constructor data
	const wchar_t* filenameopen;
	const wchar_t* filenamesave;
	WORD wCount;

	// read tcd
	std::deque<sTcd> vData;

	// read txt
	std::deque<std::string> strData;
	std::deque<std::tuple<INT, CString>> DataOut;
};

cParser::cParser(const wchar_t* filenameopen, const wchar_t* filenamesave)
{
	this->filenameopen = filenameopen;
	this->filenamesave = filenamesave;
	wCount = 0x0000;
}

cParser::~cParser()
{
	vData.clear();
	strData.clear();
}

void cParser::ReadTcd()
{
	CFile cFile;
	if (cFile.Open(filenameopen, CFile::modeRead | CFile::typeBinary))
	{
		CArchive Archive(&cFile, CArchive::load);
		Archive >> wCount;

		for (WORD i = 0; i < wCount; i++)
		{
			sTcd readStruct;
			Archive >> readStruct.wStringID >> readStruct.strString;
			readStruct.strString.Replace(_T("\\n"), _T("\n"));
			vData.push_back(readStruct);
		}

		printf("Tcd successfully read!\n");
		OutputData(0);
	}
	else
	{
		printf("Could not open %ws\n", filenameopen);
	}
}

void cParser::ReadTxt()
{
	CFile cFile;
	std::string tmpStr;
	if (cFile.Open(filenamesave, CFile::modeRead | CFile::typeBinary))
	{
		for (INT i = 0; i < cFile.GetLength(); i++)
		{
			BYTE tmpChar;
			cFile.Read(&tmpChar, sizeof(BYTE));
			tmpStr.push_back(tmpChar);
		}

		std::istringstream iss(tmpStr);
		std::string line;
		while (std::getline(iss, line))
		{
			strData.push_back(line);
		}

		printf("Txt successfully read!\n");
		OutputData(1);
	}
	else
	{
		printf("Could not open %ws\n", filenamesave);
	}
}

void cParser::OutputData(BOOL bType)
{
	if (!bType)
	{
		CFile cFile;
		if (cFile.Open(filenamesave, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
		{
			for (SIZE_T i = 0; i < vData.size(); i++)
			{
				vData[i].strString.Replace(_T("\n"), _T("\\n"));
				char strID[64];
				sprintf_s(strID, "%d#ID_END\0", (INT)vData[i].wStringID);
				cFile.Write(&strID, sizeof(char) * strlen(strID));
				for (INT j = 0; j < vData[i].strString.GetLength(); j++)
				{
					BYTE bChar = (BYTE)vData[i].strString.GetAt(j);
					cFile.Write(&bChar, sizeof(BYTE));
				}
				cFile.Write("\n", sizeof(BYTE));
			}
		}
		printf("Tcd text successfully dumped!\n");
	}
	else
	{
		for (SIZE_T i = 0; i < strData.size(); i++)
		{
			const char* strEnd = "#ID_END";
			SIZE_T nIdxEnd = strData[i].find(strEnd);
			std::string strTemp = strData[i];
			std::string strOut = strTemp.erase(0, nIdxEnd + strlen(strEnd));
			DataOut.push_back(std::make_tuple(std::stoi(strData[i].substr(0, nIdxEnd)), strOut.c_str()));
		}

		CFile cFile;
		wCount = (WORD)strData.size();
		if (cFile.Open(STRFILE_SAVE_EDIT, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
		{
			CArchive Archive(&cFile, CArchive::store);
			Archive << wCount;
			for (WORD i = 0; i < wCount; i++)
			{
				Archive << (WORD)std::get<0>(DataOut[i]) << std::get<1>(DataOut[i]);
			}
		}
		printf("New tcd has been saved!\n");
	}
}

int main()
{
	cParser Parser(STRFILE_OPEN, STRFILE_SAVE);
	printf("Select your operation: 0 = DECOMPILE, 1 = COMPILE\n");
	switch (_getch())
	{
		case '0':
			Parser.ReadTcd();
			break;
		case '1':
			Parser.ReadTxt();
			break;
		default:
			break;
	}
	_getch();
	system("cls");
	main();
}

