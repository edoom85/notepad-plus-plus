// This file is part of Notepad++ project
// Copyright (C)2021 Don HO <don.h@free.fr>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


#pragma once

#include <windows.h>

#include <commctrl.h>

#include <algorithm>
#include <locale>
#include <string>
#include <unordered_set>
#include <vector>


NppString folderBrowser(HWND parent, const NppString & title = "", int outputCtrlID = 0, const NppChar *defaultStr = NULL);
NppString getFolderName(HWND parent, const NppChar *defaultDir = NULL);

void printInt(int int2print);
void printStr(const NppChar *str2print);
NppString commafyInt(size_t n);

void writeLog(const NppChar* logFileName, const char* log2write);
void writeLog(const NppChar* logFileName, const NppChar* log2write);
int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep);
NppString purgeMenuItemString(const NppChar* menuItemStr, bool keepAmpersand = false);
std::vector<NppString> tokenizeString(const NppString & tokenString, const char delim);

void ClientRectToScreenRect(HWND hWnd, RECT* rect);
void ScreenRectToClientRect(HWND hWnd, RECT* rect);

NppString string2wstring(const std::string& rString, UINT codepage = CP_UTF8);
std::string wstring2string(const NppString& rwString, UINT codepage = CP_UTF8);
bool isInList(const NppChar* token, const NppChar* list);
NppString BuildMenuFileName(int filenameLen, unsigned int pos, const NppString &filename, bool ordinalNumber = true);

std::string getFileContent(const NppChar* file2read, bool* pbFailed = nullptr);
NppString relativeFilePathToFullFilePath(const NppChar *relativeFilePath);
void writeFileContent(const NppChar *file2write, const char *content2write);
bool matchInList(const NppChar *fileName, const std::vector<NppString> & patterns);
bool matchInExcludeDirList(const NppChar* dirName, const std::vector<NppString>& patterns, size_t level);
bool allPatternsAreExclusion(const std::vector<NppString>& patterns);
HRESULT openInExplorerAndSelect(const NppChar* path);

class WcharMbcsConvertor final
{
public:
	static WcharMbcsConvertor& getInstance() {
		static WcharMbcsConvertor instance;
		return instance;
	}

	const NppChar* char2wchar(const char* mbcs2Convert, size_t codepage, int lenMbcs = -1, int* pLenWc = nullptr, int* pBytesNotProcessed = NULL);
	const NppChar* char2wchar(const char* mbcs2Convert, size_t codepage, intptr_t* mstart, intptr_t* mend, int mbcsLen = 0);
	size_t getSizeW() const { return _wideCharStr.size(); }
	const char* wchar2char(const NppChar* wcharStr2Convert, size_t codepage, int lenWc = -1, int* pLenMbcs = nullptr);
	const char* wchar2char(const NppChar* wcharStr2Convert, size_t codepage, intptr_t* mstart, intptr_t* mend, int wcharLenIn = 0, int* lenOut = nullptr);
	size_t getSizeA() const { return _multiByteStr.size(); }

	const char* encode(UINT fromCodepage, UINT toCodepage, const char* txt2Encode, int lenIn = -1, int* pLenOut = NULL, int* pBytesNotProcessed = NULL) {
		int lenWc = 0;
		const NppChar* strW = char2wchar(txt2Encode, fromCodepage, lenIn, &lenWc, pBytesNotProcessed);
		return wchar2char(strW, toCodepage, lenWc, pLenOut);
	}

protected:
	WcharMbcsConvertor() = default;
	~WcharMbcsConvertor() = default;

	// Since there's no public ctor, we need to void the default assignment operator and copy ctor.
	// Since these are marked as deleted does not matter under which access specifier are kept
	WcharMbcsConvertor(const WcharMbcsConvertor&) = delete;
	WcharMbcsConvertor& operator= (const WcharMbcsConvertor&) = delete;

	// No move ctor and assignment
	WcharMbcsConvertor(WcharMbcsConvertor&&) = delete;
	WcharMbcsConvertor& operator= (WcharMbcsConvertor&&) = delete;

	template <class T> class StringBuffer final
	{
	public:
		~StringBuffer() { if (_allocLen) delete[] _str; }

		void sizeTo(size_t size) {
			if (_allocLen < size + 1)
			{
				if (_allocLen)
					delete[] _str;
				_allocLen = std::max<size_t>(size + 1, initSize);
				_str = new T[_allocLen]{};
			}
			_dataLen = size;
		}

		void empty() {
			static T nullStr = 0; // routines may return an empty string, with null terminator, without allocating memory; a pointer to this null character will be returned in that case
			if (_allocLen == 0)
				_str = &nullStr;
			else
				_str[0] = 0;
			_dataLen = 0;
		}

		size_t size() const { return _dataLen; }
		operator T* () { return _str; }
		operator const T* () const { return _str; }

	protected:
		static constexpr int initSize = 1024;
		size_t _allocLen = 0;
		size_t _dataLen = 0;
		T* _str = nullptr;
	};

	StringBuffer<char> _multiByteStr;
	StringBuffer<NppChar> _wideCharStr;
};

NppString pathRemoveFileSpec(NppString & path);
NppString pathAppend(NppString &strDest, const NppString & str2append);
COLORREF getCtrlBgColor(HWND hWnd);
NppString stringToUpper(NppString strToConvert);
NppString stringToLower(NppString strToConvert);
NppString stringReplace(NppString subject, const NppString& search, const NppString& replace);
void stringSplit(const NppString& input, const NppString& delimiter, std::vector<NppString>& output);
bool str2numberVector(NppString str2convert, std::vector<size_t>& numVect);
void stringJoin(const std::vector<NppString>& strings, const NppString& separator, NppString& joinedString);
NppString stringTakeWhileAdmissable(const NppString& input, const NppString& admissable);
double stodLocale(const NppString& str, _locale_t loc, size_t* idx = NULL);

const std::locale& getSysLocale();

bool str2Clipboard(const NppString &str2cpy, HWND hwnd);
NppString strFromClipboard();
class Buffer;
bool buf2Clipboard(const std::vector<Buffer*>& buffers, bool isFullPath, HWND hwnd);

NppString GetLastErrorAsString(DWORD errorCode = 0);

NppString intToString(int val);
NppString uintToString(unsigned int val);

HWND createToolTip(int toolID, HWND hDlg, HINSTANCE hInst, NppChar* pszText, bool isRTL);
HWND createToolTipRect(int toolID, HWND hWnd, HINSTANCE hInst, NppChar* pszText, const RECT rc);

bool isCertificateValidated(const NppString & fullFilePath, const NppString & subjectName2check);
bool isAssoCommandExisting(LPCWSTR FullPathName);

bool deleteFileOrFolder(const NppString& f2delete);

void getFilesInFolder(std::vector<NppString>& files, const NppString& extTypeFilter, const NppString& inFolder);

template<typename T> size_t vecRemoveDuplicates(std::vector<T>& vec, bool isSorted = false, bool canSort = false)
{
	if (!isSorted && canSort)
	{
		std::sort(vec.begin(), vec.end());
		isSorted = true;
	}

	if (isSorted)
	{
		typename std::vector<T>::iterator it;
		it = std::unique(vec.begin(), vec.end());
		vec.resize(distance(vec.begin(), it));  // unique() does not shrink the vector
	}
	else
	{
		std::unordered_set<T> seen;
		auto newEnd = std::remove_if(vec.begin(), vec.end(), [&seen](const T& value)
			{
				return !seen.insert(value).second;
			});
		vec.erase(newEnd, vec.end());
	}
	return vec.size();
}

void trim(NppString& str);

int nbDigitsFromNbLines(size_t nbLines);

NppString getDateTimeStrFrom(const NppString& dateTimeFormat, const SYSTEMTIME& st);

HFONT createFont(const NppChar* fontName, int fontSize, bool isBold, HWND hDestParent);
bool removeReadOnlyFlagFromFileAttributes(const NppChar* fileFullPath);
bool toggleReadOnlyFlagFromFileAttributes(const NppChar* fileFullPath, bool& isChangedToReadOnly);

bool isWin32NamespacePrefixedFileName(const NppString& fileName);
bool isWin32NamespacePrefixedFileName(const NppChar* szFileName);
bool isUnsupportedFileName(const NppString& fileName);
bool isUnsupportedFileName(const NppChar* szFileName);
bool isUncPath(const NppString& path);
bool isUncFileUrl(const NppString& url);

class Version final
{
public:
	Version() = default;
	explicit Version(const NppString& versionStr);

	void setVersionFrom(const NppString& filePath);
	NppString toString() const;
	static bool isNumber(const NppString& s) {
		static const auto& loc = std::locale::classic();
		return !s.empty() &&
			find_if(s.begin(), s.end(), [](auto c) { return !std::isdigit(c, loc); }) == s.end();
	}

	int compareTo(const Version& v2c) const;

	bool operator < (const Version& v2c) const {
		return compareTo(v2c) == -1;
	}

	bool operator <= (const Version& v2c) const {
		int r = compareTo(v2c);
		return r == -1 || r == 0;
	}

	bool operator > (const Version& v2c) const {
		return compareTo(v2c) == 1;
	}

	bool operator >= (const Version& v2c) const {
		int r = compareTo(v2c);
		return r == 1 || r == 0;
	}

	bool operator == (const Version& v2c) const {
		return compareTo(v2c) == 0;
	}

	bool operator != (const Version& v2c) const {
		return compareTo(v2c) != 0;
	}

	bool empty() const {
		return _major == 0 && _minor == 0 && _patch == 0 && _build == 0;
	}

	bool isCompatibleTo(const Version& from, const Version& to) const;

private:
	unsigned long _major = 0;
	unsigned long _minor = 0;
	unsigned long _patch = 0;
	unsigned long _build = 0;
};


BOOL getDiskFreeSpaceWithTimeout(const NppChar* dirPath, ULARGE_INTEGER* freeBytesForUser,
	DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);
BOOL getFileAttributesExWithTimeout(const NppChar* filePath, WIN32_FILE_ATTRIBUTE_DATA* fileAttr,
	DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr, DWORD* pdwWin32ApiError = nullptr);

bool doesFileExist(const NppChar* filePath, DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);
bool doesDirectoryExist(const NppChar* dirPath, DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);
bool doesPathExist(const NppChar* path, DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);


// check if the window rectangle intersects with any currently active monitor's working area
bool isWindowVisibleOnAnyMonitor(const RECT& rectWndIn);

bool isCoreWindows();


class ControlInfoTip final
{
public:
	ControlInfoTip() = default;
	~ControlInfoTip() {
		if (_hWndInfoTip) {
			hide();
		}
	}

	bool init(HINSTANCE hInst, HWND ctrl2attached, HWND ctrl2attachedParent, const NppString& tipStr, bool isRTL, unsigned int remainTimeMillisecond = 0, int maxWidth = 200); // remainTimeMillisecond = 0: no timeout

	bool isValid() const {
		return _hWndInfoTip != nullptr;
	}

	HWND getTipHandle() const {
		return _hWndInfoTip;
	}

	enum showPosition {beginning, middle, end};
	void show(showPosition pos = middle) const;
	
	void hide();

private:
	HWND _hWndInfoTip = nullptr;
	TOOLINFO _toolInfo = {};

	ControlInfoTip(const ControlInfoTip&) = delete;
	ControlInfoTip& operator=(const ControlInfoTip&) = delete;
};

DWORD invokeNppUacOp(const NppString& strCmdLineParams);
bool fileTimeToYMD(const FILETIME& ft, int& yyyymmdd);
void expandEnv(NppString& path2Expand);

class ScopedCOMInit final // never use this in DllMain
{
public:
	ScopedCOMInit() {
		HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // attempt STA init 1st (older CoInitialize(NULL))
		if (hr == RPC_E_CHANGED_MODE) {
			hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED); // STA init failed, switch to MTA
		}
		if (SUCCEEDED(hr)) {
			// S_OK or S_FALSE, both needs subsequent CoUninitialize()
			_bInitialized = true;
		}
	}

	~ScopedCOMInit() {
		if (_bInitialized) {
			_bInitialized = false;
			::CoUninitialize();
		}
	}

	bool isInitialized() const {
		return _bInitialized;
	}

private:
	bool _bInitialized = false;

	ScopedCOMInit(const ScopedCOMInit&) = delete;
	ScopedCOMInit& operator=(const ScopedCOMInit&) = delete;
};


bool needsElevation4Access(const NppString& path2check, bool bWriteAccess);
