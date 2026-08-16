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


#include <windows.h>

#include <chrono>
#include <cstdlib>
#include <cwchar>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Common.h"
#include "FileInterface.h"
#include "MiniDumper.h" //Write dump files
#include "Notepad_plus_Window.h"
#include "Notepad_plus_msgs.h"
#include "NppConstants.h"
#include "NppDarkMode.h"
#include "Parameters.h"
#include "Processus.h"
#include "Win32Exception.h" //Win32 exception
#include "dpiManagerV2.h"
#include "resource.h"
#include "verifySignedfile.h"

typedef std::vector<NppString> ParamVector;


namespace
{


void allowPrivilegeMessages(const Notepad_plus_Window& notepad_plus_plus, winVer winVer)
{
	#ifndef MSGFLT_ADD
	const DWORD MSGFLT_ADD = 1;
	#endif
	#ifndef MSGFLT_ALLOW
	const DWORD MSGFLT_ALLOW = 1;
	#endif
	// Tell UAC that lower integrity processes are allowed to send WM_COPYDATA (or other) messages to this process (or window)
	// This (WM_COPYDATA) allows opening new files to already opened elevated Notepad++ process via explorer context menu.
	if (winVer >= WV_VISTA || winVer == WV_UNKNOWN)
	{
		HMODULE hDll = GetModuleHandle("user32.dll");
		if (hDll)
		{
			// According to MSDN ChangeWindowMessageFilter may not be supported in future versions of Windows,
			// that is why we use ChangeWindowMessageFilterEx if it is available (windows version >= Win7).
			if (winVer == WV_VISTA)
			{
				typedef BOOL (WINAPI *MESSAGEFILTERFUNC)(UINT message,DWORD dwFlag);

				MESSAGEFILTERFUNC func = (MESSAGEFILTERFUNC)::GetProcAddress( hDll, "ChangeWindowMessageFilter" );

				if (func)
				{
					func(WM_COPYDATA, MSGFLT_ADD);
					func(NPPM_INTERNAL_RESTOREFROMMINIMIZED, MSGFLT_ADD);
				}
			}
			else
			{
				typedef BOOL (WINAPI *MESSAGEFILTERFUNCEX)(HWND hWnd,UINT message,DWORD action,VOID* pChangeFilterStruct);

				MESSAGEFILTERFUNCEX funcEx = (MESSAGEFILTERFUNCEX)::GetProcAddress( hDll, "ChangeWindowMessageFilterEx" );

				if (funcEx)
				{
					funcEx(notepad_plus_plus.getHSelf(), WM_COPYDATA, MSGFLT_ALLOW, NULL);
					funcEx(notepad_plus_plus.getHSelf(), NPPM_INTERNAL_RESTOREFROMMINIMIZED, MSGFLT_ALLOW, NULL);
				}
			}
		}
	}
}

// parseCommandLine() takes command line arguments part string, cuts arguments by using white space as separator.
// Only white space in double quotes will be kept, such as file path argument or '-settingsDir=' argument (ex.: -settingsDir="c:\my settings\my folder\")
// if '-z' is present, the 3rd argument after -z won't be cut - ie. all the space will also be kept
// ex.: '-notepadStyleCmdline -z "C:\WINDOWS\system32\NOTEPAD.EXE" C:\my folder\my file with whitespace.txt' will be separated to: 
// 1. "-notepadStyleCmdline"
// 2. "-z"
// 3. "C:\WINDOWS\system32\NOTEPAD.EXE"
// 4. "C:\my folder\my file with whitespace.txt" 
void parseCommandLine(const NppChar* commandLine, ParamVector& paramVector)
{
	if (!commandLine)
		return;
	
	NppChar* cmdLine = new NppChar[strlen(commandLine) + 1];
	lstrcpy(cmdLine, commandLine);

	NppChar* cmdLinePtr = cmdLine;

	bool isBetweenFileNameQuotes = false;
	bool isStringInArg = false;
	bool isInWhiteSpace = true;

	int zArg = 0; // for "-z" argument: Causes Notepad++ to ignore the next command line argument (a single word, or a phrase in quotes).
	              // The only intended and supported use for this option is for the Notepad Replacement syntax.

	bool shouldBeTerminated = false; // If "-z" argument has been found, zArg value will be increased from 0 to 1.
	                                 // then after processing next argument of "-z", zArg value will be increased from 1 to 2.
	                                 // when zArg == 2 shouldBeTerminated will be set to true - it will trigger the treatment which consider the rest as a argument, with or without white space(s).

	size_t commandLength = strlen(cmdLinePtr);
	std::vector<NppChar *> args;
	for (size_t i = 0; i < commandLength && !shouldBeTerminated; ++i)
	{
		switch (cmdLinePtr[i])
		{
			case '\"': //quoted filename, ignore any following whitespace
			{
				if (!isStringInArg && !isBetweenFileNameQuotes && i > 0 && cmdLinePtr[i-1] == '=')
				{
					isStringInArg = true;
				}
				else if (isStringInArg)
				{
					isStringInArg = false;
				}
				else if (!isBetweenFileNameQuotes)	//" will always be treated as start or end of param, in case the user forgot to add an space
				{
					args.push_back(cmdLinePtr + i + 1);	//add next param(since zero terminated original, no overflow of +1)
					isBetweenFileNameQuotes = true;
					cmdLinePtr[i] = 0;

					if (zArg == 1)
					{
						++zArg; // zArg == 2
					}
				}
				else //if (isBetweenFileNameQuotes)
				{
					isBetweenFileNameQuotes = false;
					//because we don't want to leave in any quotes in the filename, remove them now (with zero terminator)
					cmdLinePtr[i] = 0;
				}
				isInWhiteSpace = false;
			}
			break;

			case '\t': //also treat tab as whitespace
			case ' ':
			{
				isInWhiteSpace = true;
				if (!isBetweenFileNameQuotes && !isStringInArg)
				{
					cmdLinePtr[i] = 0;		//zap spaces into zero terminators, unless its part of a filename

					size_t argsLen = args.size();
					if (argsLen > 0 && lstrcmp(args[argsLen-1], "-z") == 0)
						++zArg; // "-z" argument is found: change zArg value from 0 (initial) to 1
				}
			}
			break;

			default: //default NppChar, if beginning of word, add it
			{
				if (!isBetweenFileNameQuotes && !isStringInArg && isInWhiteSpace)
				{
					args.push_back(cmdLinePtr + i);	//add next param
					if (zArg == 2)
					{
						shouldBeTerminated = true; // stop the processing, and keep the rest string as it in the vector
					}

					isInWhiteSpace = false;
				}
			}
		}
	}
	paramVector.assign(args.begin(), args.end());
	delete[] cmdLine;
}

// Converts /p or /P to -quickPrint if it exists as the first parameter
// This seems to mirror Notepad's behaviour
void convertParamsToNotepadStyle(ParamVector& params)
{
	for (auto it = params.begin(); it != params.end(); ++it)
	{
		if (lstrcmp(it->c_str(), "/p") == 0 || lstrcmp(it->c_str(), "/P") == 0)
		{
			it->assign("-quickPrint");
		}
	}
}

bool isInList(const NppChar *token2Find, ParamVector& params, bool eraseArg = true)
{
	for (auto it = params.begin(); it != params.end(); ++it)
	{
		if (lstrcmp(token2Find, it->c_str()) == 0)
		{
			if (eraseArg) params.erase(it);
			return true;
		}
	}
	return false;
}

bool getParamVal(NppChar c, ParamVector & params, NppString & value)
{
	value = "";
	size_t nbItems = params.size();

	for (size_t i = 0; i < nbItems; ++i)
	{
		const NppChar * token = params.at(i).c_str();
		if (token[0] == '-' && strlen(token) >= 2 && token[1] == c) //dash, and enough chars
		{
			value = (token+2);
			params.erase(params.begin() + i);
			return true;
		}
	}
	return false;
}

bool getParamValFromString(const NppChar *str, ParamVector & params, NppString & value)
{
	value = "";
	size_t nbItems = params.size();

	for (size_t i = 0; i < nbItems; ++i)
	{
		const NppChar * token = params.at(i).c_str();
		NppString tokenStr = token;
		size_t pos = tokenStr.find(str);
		if (pos != NppString::npos && pos == 0)
		{
			value = (token + strlen(str));
			params.erase(params.begin() + i);
			return true;
		}
	}
	return false;
}

LangType getLangTypeFromParam(ParamVector & params)
{
	NppString langStr;
	if (!getParamVal('l', params, langStr))
		return L_EXTERNAL;
	return NppParameters::getLangIDFromStr(langStr.c_str());
}

NppString getLocalizationPathFromParam(ParamVector & params)
{
	NppString locStr;
	if (!getParamVal('L', params, locStr))
		return "";
	locStr = stringToLower(stringReplace(locStr, "_", "-")); // convert to lowercase format with "-" as separator
	return NppParameters::getLocPathFromStr(locStr);
}

intptr_t getNumberFromParam(char paramName, ParamVector & params, bool & isParamePresent)
{
	NppString numStr;
	if (!getParamVal(paramName, params, numStr))
	{
		isParamePresent = false;
		return -1;
	}
	isParamePresent = true;
	return static_cast<intptr_t>(std::stoll(numStr));
}

NppString getEasterEggNameFromParam(ParamVector & params, unsigned char & type)
{
	NppString EasterEggName;
	if (!getParamValFromString("-qn=", params, EasterEggName))  // get internal easter egg
	{
		if (!getParamValFromString("-qt=", params, EasterEggName)) // get user quote from cmdline argument
		{
			if (!getParamValFromString("-qf=", params, EasterEggName)) // get user quote from a content of file
				return "";
			else
			{
				type = 2; // quote content in file
			}
		}
		else
			type = 1; // commandline quote
	}
	else
		type = 0; // easter egg

	if (EasterEggName.c_str()[0] == '"' && EasterEggName.c_str()[EasterEggName.length() - 1] == '"')
	{
		EasterEggName = EasterEggName.substr(1, EasterEggName.length() - 2);
	}

	if (type == 2)
		EasterEggName = relativeFilePathToFullFilePath(EasterEggName.c_str());

	return EasterEggName;
}

int getGhostTypingSpeedFromParam(ParamVector & params)
{
	NppString speedStr;
	if (!getParamValFromString("-qSpeed", params, speedStr))
		return -1;
	
	int speed = std::stoi(speedStr, 0);
	if (speed <= 0 || speed > 3)
		return -1;

	return speed;
}

const NppChar FLAG_MULTI_INSTANCE[] = "-multiInst";
const NppChar FLAG_NO_PLUGIN[] = "-noPlugin";
const NppChar FLAG_READONLY[] = "-ro"; // for current cmdline file(s) only
const NppChar FLAG_FULL_READONLY[] = "-fullReadOnly"; // user still can manually toggle OFF the R/O-state of N++ tabs, so saving of the tab filebuffers is possible
const NppChar FLAG_FULL_READONLY_SAVING_FORBIDDEN[] = "-fullReadOnlySavingForbidden"; // user cannot toggle R/O-state of N++ tabs, impossible to save opened tab filebuffers
const NppChar FLAG_NOSESSION[] = "-nosession";
const NppChar FLAG_NOTABBAR[] = "-notabbar";
const NppChar FLAG_SYSTRAY[] = "-systemtray";
const NppChar FLAG_LOADINGTIME[] = "-loadingTime";
const NppChar FLAG_HELP[] = "--help";
const NppChar FLAG_ALWAYS_ON_TOP[] = "-alwaysOnTop";
const NppChar FLAG_OPENSESSIONFILE[] = "-openSession";
const NppChar FLAG_RECURSIVE[] = "-r";
const NppChar FLAG_FUNCLSTEXPORT[] = "-export=functionList";
const NppChar FLAG_PRINTANDQUIT[] = "-quickPrint";
const NppChar FLAG_NOTEPAD_COMPATIBILITY[] = "-notepadStyleCmdline";
const NppChar FLAG_OPEN_FOLDERS_AS_WORKSPACE[] = "-openFoldersAsWorkspace";
const NppChar FLAG_SETTINGS_DIR[] = "-settingsDir=";
const NppChar FLAG_TITLEBAR_ADD[] = "-titleAdd=";
const NppChar FLAG_APPLY_UDL[] = "-udl=";
const NppChar FLAG_PLUGIN_MESSAGE[] = "-pluginMessage=";
const NppChar FLAG_MONITOR_FILES[] = "-monitor";
const NppChar FLAG_MONITORING_MODE[] = "-monitoringMode";

void doException(Notepad_plus_Window & notepad_plus_plus)
{
	Win32Exception::removeHandler();	//disable exception handler after exception, we don't want corrupt data structures to crash the exception handler
	::MessageBox(Notepad_plus_Window::gNppHWND, "Notepad++ will attempt to save any unsaved data. However, data loss is very likely.", "Recovery initiating", MB_OK | MB_ICONINFORMATION);

	NppChar tmpDir[1024];
	GetTempPath(1024, tmpDir);
	NppString emergencySavedDir = tmpDir;
	emergencySavedDir += "\\Notepad++ RECOV";

	bool res = notepad_plus_plus.emergency(emergencySavedDir);
	if (res)
	{
		NppString displayText = "Notepad++ was able to successfully recover some unsaved documents, or nothing to be saved could be found.\r\nYou can find the results at :\r\n";
		displayText += emergencySavedDir;
		::MessageBox(Notepad_plus_Window::gNppHWND, displayText.c_str(), "Recovery success", MB_OK | MB_ICONINFORMATION);
	}
	else
		::MessageBox(Notepad_plus_Window::gNppHWND, "Unfortunately, Notepad++ was not able to save your work. We are sorry for any lost data.", "Recovery failure", MB_OK | MB_ICONERROR);
}

// Looks for -z arguments and strips command line arguments following those, if any
void stripIgnoredParams(ParamVector & params)
{
	for (auto it = params.begin(); it != params.end(); )
	{
		if (lstrcmp(it->c_str(), "-z") == 0)
		{
			auto nextIt = std::next(it);
			if ( nextIt != params.end() )
			{
				params.erase(nextIt);
			}
			it = params.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool launchUpdater(const NppString& updaterFullPath, const NppString& updaterDir)
{
	NppParameters& nppParameters = NppParameters::getInstance();
	NppGUI& nppGui = nppParameters.getNppGUI();

	// check if update interval elapsed
	Date today(0);
	if (today < nppGui._autoUpdateOpt._nextUpdateDate)
		return false;

	NppString updaterParams;
	nppParameters.buildGupParams(updaterParams);

	Process updater(updaterFullPath.c_str(), updaterParams.c_str(), updaterDir.c_str());
	updater.run();

	// Update next update date
	if (nppGui._autoUpdateOpt._intervalDays < 0) // Make sure interval days value is positive
		nppGui._autoUpdateOpt._intervalDays = 0 - nppGui._autoUpdateOpt._intervalDays;
	nppGui._autoUpdateOpt._nextUpdateDate = Date(nppGui._autoUpdateOpt._intervalDays);

	return true;
}

DWORD nppUacSave(const NppChar* wszTempFilePath, const NppChar* wszProtectedFilePath2Save)
{
	if ((lstrlenW(wszTempFilePath) == 0) || (lstrlenW(wszProtectedFilePath2Save) == 0)) // safe check (strlen returns 0 for possible nullptr)
		return ERROR_INVALID_PARAMETER;
	if (!doesFileExist(wszTempFilePath))
		return ERROR_FILE_NOT_FOUND;

	DWORD dwRetCode = ERROR_SUCCESS;

	bool isOutputReadOnly = false;
	bool isOutputHidden = false;
	bool isOutputSystem = false;
	WIN32_FILE_ATTRIBUTE_DATA attributes{};
	attributes.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
	if (getFileAttributesExWithTimeout(wszProtectedFilePath2Save, &attributes))
	{
		if (attributes.dwFileAttributes != INVALID_FILE_ATTRIBUTES && !(attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			isOutputReadOnly = (attributes.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
			isOutputHidden = (attributes.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
			isOutputSystem = (attributes.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0;
			if (isOutputReadOnly) attributes.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
			if (isOutputHidden) attributes.dwFileAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
			if (isOutputSystem) attributes.dwFileAttributes &= ~FILE_ATTRIBUTE_SYSTEM;
			if (isOutputReadOnly || isOutputHidden || isOutputSystem)
				::SetFileAttributes(wszProtectedFilePath2Save, attributes.dwFileAttributes); // temporarily remove the problematic ones
		}
	}

	// cannot use simple MoveFile here as it retains the tempfile permissions when on the same volume...
	if (!::CopyFileW(wszTempFilePath, wszProtectedFilePath2Save, FALSE))
	{
		// fails if the destination file exists and has the R/O and/or Hidden attribute set
		dwRetCode = ::GetLastError();
	}
	else
	{
		// ok, now dispose of the tempfile used
		::DeleteFileW(wszTempFilePath);
	}

	// set back the possible original file attributes
	if (isOutputReadOnly || isOutputHidden || isOutputSystem)
	{
		if (isOutputReadOnly) attributes.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
		if (isOutputHidden) attributes.dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
		if (isOutputSystem) attributes.dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;
		::SetFileAttributes(wszProtectedFilePath2Save, attributes.dwFileAttributes);
	}

	return dwRetCode;
}

DWORD nppUacSetFileAttributes(const DWORD dwFileAttribs, const NppChar* wszFilePath)
{
	if (lstrlenW(wszFilePath) == 0) // safe check (strlen returns 0 for possible nullptr)
		return ERROR_INVALID_PARAMETER;
	if (!doesFileExist(wszFilePath))
		return ERROR_FILE_NOT_FOUND;
	if (dwFileAttribs == INVALID_FILE_ATTRIBUTES || (dwFileAttribs & FILE_ATTRIBUTE_DIRECTORY))
		return ERROR_INVALID_PARAMETER;

	if (!::SetFileAttributes(wszFilePath, dwFileAttribs))
		return ::GetLastError();

	return ERROR_SUCCESS;
}

DWORD nppUacMoveFile(const NppChar* wszOriginalFilePath, const NppChar* wszNewFilePath)
{
	if ((lstrlenW(wszOriginalFilePath) == 0) || (lstrlenW(wszNewFilePath) == 0)) // safe check (strlen returns 0 for possible nullptr)
		return ERROR_INVALID_PARAMETER;
	if (!doesFileExist(wszOriginalFilePath))
		return ERROR_FILE_NOT_FOUND;

	if (!::MoveFileEx(wszOriginalFilePath, wszNewFilePath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH))
		return ::GetLastError();
	else
		return ERROR_SUCCESS;
}

DWORD nppUacCreateEmptyFile(const NppChar* wszNewEmptyFilePath)
{
	if (lstrlenW(wszNewEmptyFilePath) == 0) // safe check (strlen returns 0 for possible nullptr)
		return ERROR_INVALID_PARAMETER;
	if (doesFileExist(wszNewEmptyFilePath))
		return ERROR_FILE_EXISTS;

	Win32_IO_File file(wszNewEmptyFilePath);
	if (!file.isOpened())
		return file.getLastErrorCode();

	return ERROR_SUCCESS;
}

} // namespace


std::chrono::steady_clock::time_point g_nppStartTimePoint{};


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ PWSTR pCmdLine, _In_ int /*nShowCmd*/)
{
	g_nppStartTimePoint = std::chrono::steady_clock::now();
	
	// Notepad++ UAC OPS /////////////////////////////////////////////////////////////////////////////////////////////
	if ((lstrlenW(pCmdLine) > 0) && (__argc >= 2)) // safe (if pCmdLine is NULL, strlen returns 0)
	{
		const NppChar* wszNppUacOpSign = __wargv[1];
		if (lstrlenW(wszNppUacOpSign) > lstrlenW("#UAC-#"))
		{
			if ((__argc == 4) && (wcscmp(wszNppUacOpSign, NPP_UAC_SAVE_SIGN) == 0))
			{
				// __wargv[x]: 2 ... tempFilePath, 3  ...  protectedFilePath2Save
				return static_cast<int>(nppUacSave(__wargv[2], __wargv[3]));
			}

			if ((__argc == 4) && (wcscmp(wszNppUacOpSign, NPP_UAC_SETFILEATTRIBUTES_SIGN) == 0))
			{
				// __wargv[x]: 2 ... dwFileAttributes (string), 3  ...  filePath
				try {
					return static_cast<int>(nppUacSetFileAttributes(static_cast<DWORD>(std::stoul(NppString(__wargv[2]))), __wargv[3]));
				}
				catch ([[maybe_unused]] const std::exception& e)
				{
					return static_cast<int>(ERROR_INVALID_PARAMETER); // conversion error (check e.what() for details)
				}
			}

			if ((__argc == 4) && (wcscmp(wszNppUacOpSign, NPP_UAC_MOVEFILE_SIGN) == 0))
			{
				// __wargv[x]: 2 ... originalFilePath, 3  ...  newFilePath
				return static_cast<int>(nppUacMoveFile(__wargv[2], __wargv[3]));
			}

			if ((__argc == 3) && (wcscmp(wszNppUacOpSign, NPP_UAC_CREATEEMPTYFILE_SIGN) == 0))
			{
				// __wargv[x]: 2 ... newEmptyFilePath
				return static_cast<int>(nppUacCreateEmptyFile(__wargv[2]));
			}
		}
	} // Notepad++ UAC OPS////////////////////////////////////////////////////////////////////////////////////////////

	bool TheFirstOne = true;
	::SetLastError(NO_ERROR);
	::CreateMutex(NULL, false, "nppInstance");
	if (::GetLastError() == ERROR_ALREADY_EXISTS)
		TheFirstOne = false;

	NppString cmdLineString = pCmdLine ? pCmdLine : "";
	ParamVector params;
	parseCommandLine(pCmdLine, params);


	// Convert commandline to notepad-compatible format, if applicable
	// For treating "-notepadStyleCmdline" "/P" and "-z"
	stripIgnoredParams(params);
	if ( isInList(FLAG_NOTEPAD_COMPATIBILITY, params) )
	{
		convertParamsToNotepadStyle(params);
	}

	bool isParamePresent;
	bool isMultiInst = isInList(FLAG_MULTI_INSTANCE, params);
	bool doFunctionListExport = isInList(FLAG_FUNCLSTEXPORT, params);
	bool doPrintAndQuit = isInList(FLAG_PRINTANDQUIT, params);

	CmdLineParams cmdLineParams;
	cmdLineParams._displayCmdLineArgs = isInList(FLAG_HELP, params);
	cmdLineParams._isNoTab = isInList(FLAG_NOTABBAR, params);
	cmdLineParams._isNoPlugin = isInList(FLAG_NO_PLUGIN, params);
	cmdLineParams._isReadOnly = isInList(FLAG_READONLY, params);
	cmdLineParams._isFullReadOnly = isInList(FLAG_FULL_READONLY, params);
	cmdLineParams._isFullReadOnlySavingForbidden = isInList(FLAG_FULL_READONLY_SAVING_FORBIDDEN, params);
	cmdLineParams._isNoSession = isInList(FLAG_NOSESSION, params);
	cmdLineParams._isPreLaunch = isInList(FLAG_SYSTRAY, params);
	cmdLineParams._alwaysOnTop = isInList(FLAG_ALWAYS_ON_TOP, params);
	cmdLineParams._showLoadingTime = isInList(FLAG_LOADINGTIME, params);
	cmdLineParams._isSessionFile = isInList(FLAG_OPENSESSIONFILE, params);
	cmdLineParams._isRecursive = isInList(FLAG_RECURSIVE, params);
	cmdLineParams._openFoldersAsWorkspace = isInList(FLAG_OPEN_FOLDERS_AS_WORKSPACE, params);
	cmdLineParams._monitorFiles = isInList(FLAG_MONITOR_FILES, params);
	cmdLineParams._monitoringMode = isInList(FLAG_MONITORING_MODE, params);

	cmdLineParams._langType = getLangTypeFromParam(params);
	cmdLineParams._localizationPath = getLocalizationPathFromParam(params);
	cmdLineParams._easterEggName = getEasterEggNameFromParam(params, cmdLineParams._quoteType);
	cmdLineParams._ghostTypingSpeed = getGhostTypingSpeedFromParam(params);

	NppString pluginMessage;
	if (getParamValFromString(FLAG_PLUGIN_MESSAGE, params, pluginMessage))
	{
		if (pluginMessage.length() >= 2 && (pluginMessage.front() == '"' && pluginMessage.back() == '"'))
		{
			pluginMessage = pluginMessage.substr(1, pluginMessage.length() - 2);
		}
		cmdLineParams._pluginMessage = pluginMessage;
	}

	// getNumberFromParam should be run at the end, to not consuming the other params
	cmdLineParams._line2go = getNumberFromParam('n', params, isParamePresent);
    cmdLineParams._column2go = getNumberFromParam('c', params, isParamePresent);
    cmdLineParams._pos2go = getNumberFromParam('p', params, isParamePresent);
	cmdLineParams._point.x = static_cast<LONG>(getNumberFromParam('x', params, cmdLineParams._isPointXValid));
	cmdLineParams._point.y = static_cast<LONG>(getNumberFromParam('y', params, cmdLineParams._isPointYValid));

	NppParameters& nppParameters = NppParameters::getInstance();

	nppParameters.setCmdLineString(cmdLineString);

	NppString path;
	if (getParamValFromString(FLAG_SETTINGS_DIR, params, path))
	{
		// path could contain double quotes if path contains white space
		if (path.c_str()[0] == '"' && path.c_str()[path.length() - 1] == '"')
		{
			path = path.substr(1, path.length() - 2);
		}
		nppParameters.setCmdSettingsDir(path);
	}

	NppString titleBarAdditional;
	if (getParamValFromString(FLAG_TITLEBAR_ADD, params, titleBarAdditional))
	{
		if (titleBarAdditional.length() >= 2)
		{
			if (titleBarAdditional.front() == '"' && titleBarAdditional.back() == '"')
			{
				titleBarAdditional = titleBarAdditional.substr(1, titleBarAdditional.length() - 2);
			}
		}
		nppParameters.setTitleBarAdd(titleBarAdditional);
	}

	NppString udlName;
	if (getParamValFromString(FLAG_APPLY_UDL, params, udlName))
	{
		if (udlName.length() >= 2)
		{
			if (udlName.front() == '"' && udlName.back() == '"')
			{
				udlName = udlName.substr(1, udlName.length() - 2);
			}
		}
		cmdLineParams._udlName = udlName;
	}

	if (cmdLineParams._localizationPath != "")
	{
		// setStartWithLocFileName() should be called before parameters are loaded
		nppParameters.setStartWithLocFileName(cmdLineParams._localizationPath);
	}

	nppParameters.load();

	NppGUI & nppGui = nppParameters.getNppGUI();

	NppDarkMode::initDarkMode();
	DPIManagerV2::initDpiAPI();

	bool doUpdateNpp = nppGui._autoUpdateOpt._doAutoUpdate != NppGUI::autoupdate_disabled;
	bool updateAtExit = nppGui._autoUpdateOpt._doAutoUpdate == NppGUI::autoupdate_on_exit;
	bool doUpdatePluginList = nppGui._autoUpdateOpt._doAutoUpdate != NppGUI::autoupdate_disabled;

	if (doFunctionListExport || doPrintAndQuit) // export functionlist feature will serialize functionlist on the disk, then exit Notepad++. So it's important to not launch into existing instance, and keep it silent.
	{
		isMultiInst = true;
		doUpdateNpp = doUpdatePluginList = false;
		cmdLineParams._isNoSession = true;
	}

	nppParameters.setFunctionListExportBoolean(doFunctionListExport);
	nppParameters.setPrintAndExitBoolean(doPrintAndQuit);

	// override the settings if notepad style is present
	if (nppParameters.asNotepadStyle())
	{
		isMultiInst = true;
		cmdLineParams._isNoTab = true;
		cmdLineParams._isNoSession = true;
	}

	// override the settings if multiInst is chosen by user in the preference dialog
	const NppGUI & nppGUI = nppParameters.getNppGUI();
	if (nppGUI._multiInstSetting == multiInst)
	{
		isMultiInst = true;
		// Only the first launch remembers the session
		if (!TheFirstOne)
			cmdLineParams._isNoSession = true;
	}

	NppString quotFileName = "";
    // tell the running instance the FULL path to the new files to load
	size_t nbFilesToOpen = params.size();

	for (size_t i = 0; i < nbFilesToOpen; ++i)
	{
		const NppChar * currentFile = params.at(i).c_str();
		if (currentFile[0])
		{
			//check if relative or full path. Relative paths don't have a colon for driveletter

			quotFileName += "\"";
			quotFileName += relativeFilePathToFullFilePath(currentFile);
			quotFileName += "\" ";
		}
	}

	//Only after loading all the file paths set the working directory
	::SetCurrentDirectory(NppParameters::getInstance().getNppPath().c_str());	//force working directory to path of module, preventing lock

	if ((!isMultiInst) && (!TheFirstOne))
	{
		HWND hNotepad_plus = ::FindWindow(Notepad_plus_Window::getClassName(), NULL);
		for (int i = 0 ;!hNotepad_plus && i < 5 ; ++i)
		{
			Sleep(100);
			hNotepad_plus = ::FindWindow(Notepad_plus_Window::getClassName(), NULL);
		}

        if (hNotepad_plus)
        {
			// First of all, destroy static object NppParameters
			nppParameters.destroyInstance();

			// Restore the window, bring it to front, etc
			bool isInSystemTray = ::SendMessage(hNotepad_plus, NPPM_INTERNAL_RESTOREFROMMINIMIZED, 0, 0);

			if (!isInSystemTray)
			{
				int sw = 0;

				if (::IsZoomed(hNotepad_plus))
					sw = SW_MAXIMIZE;
				else if (::IsIconic(hNotepad_plus))
					sw = SW_RESTORE;

				if (sw != 0)
					::ShowWindow(hNotepad_plus, sw);
			}
			::SetForegroundWindow(hNotepad_plus);

			if (params.size() > 0                         // if there are files to open, use the WM_COPYDATA system
				|| !cmdLineParams._pluginMessage.empty()) // or pluginMessage is present, use the WM_COPYDATA system as well
			{
				CmdLineParamsDTO dto = CmdLineParamsDTO::FromCmdLineParams(cmdLineParams);

				COPYDATASTRUCT paramData{};
				paramData.dwData = COPYDATA_PARAMS;
				paramData.lpData = &dto;
				paramData.cbData = sizeof(dto);
				::SendMessage(hNotepad_plus, WM_COPYDATA, reinterpret_cast<WPARAM>(hInstance), reinterpret_cast<LPARAM>(&paramData));

				COPYDATASTRUCT cmdLineData{};
				cmdLineData.dwData = COPYDATA_FULL_CMDLINE;
				cmdLineData.lpData = (void*)cmdLineString.c_str();
				cmdLineData.cbData = static_cast<DWORD>((cmdLineString.length() + 1) * sizeof(NppChar));
				::SendMessage(hNotepad_plus, WM_COPYDATA, reinterpret_cast<WPARAM>(hInstance), reinterpret_cast<LPARAM>(&cmdLineData));

				COPYDATASTRUCT fileNamesData{};
				fileNamesData.dwData = COPYDATA_FILENAMESW;
				fileNamesData.lpData = (void *)quotFileName.c_str();
				fileNamesData.cbData = static_cast<DWORD>((quotFileName.length() + 1) * sizeof(NppChar));
				::SendMessage(hNotepad_plus, WM_COPYDATA, reinterpret_cast<WPARAM>(hInstance), reinterpret_cast<LPARAM>(&fileNamesData));
			}
			return 0;
        }
	}

	auto upNotepadWindow = std::make_unique<Notepad_plus_Window>();
	Notepad_plus_Window & notepad_plus_plus = *upNotepadWindow.get();

	NppString updaterDir = nppParameters.getNppPath();
	updaterDir += "\\updater\\";

	NppString updaterFullPath = updaterDir + "gup.exe";

	bool isUpExist = nppGui._doesExistUpdater = doesFileExist(updaterFullPath.c_str());

	// wingup doesn't work with the obsolete security layer (API) under xp since downloads are secured with SSL on notepad-plus-plus.org
	winVer ver = nppParameters.getWinVersion();
	bool isGtXP = ver > WV_XP;

	SecurityGuard securityGuard;
	bool isSignatureOK = securityGuard.checkModule(updaterFullPath, nm_gup);

	if (TheFirstOne && isUpExist && isGtXP && isSignatureOK && doUpdateNpp && !updateAtExit && !nppParameters.isNppAutoUpdateDisabled())
	{
		launchUpdater(updaterFullPath, updaterDir);
	}

	MSG msg{};
	msg.wParam = 0;
	Win32Exception::installHandler();
	MiniDumper mdump;	//for debugging purposes.
	bool isException = false;
	try {
		notepad_plus_plus.init(hInstance, NULL, quotFileName.c_str(), &cmdLineParams);
		allowPrivilegeMessages(notepad_plus_plus, ver);
		bool going = true;
		while (going)
		{
			going = ::GetMessageW(&msg, NULL, 0, 0) != 0;
			if (going)
			{
				// if the message doesn't belong to the notepad_plus_plus's dialog
				if (!notepad_plus_plus.isDlgsMsg(&msg))
				{
					if (::TranslateAccelerator(notepad_plus_plus.getHSelf(), notepad_plus_plus.getAccTable(), &msg) == 0)
					{
						::TranslateMessage(&msg);
						::DispatchMessageW(&msg);
					}
				}
			}
		}
	}
	catch (int i)
	{
		isException = true;
		NppChar str[50] = "God Damned Exception:";
		NppChar code[10];
		sprintf(code, "%d", i);
		wcscat_s(str, code);
		::MessageBox(Notepad_plus_Window::gNppHWND, str, "Int Exception", MB_OK);
		doException(notepad_plus_plus);
	}
	catch (std::runtime_error & ex)
	{
		isException = true;
		::MessageBoxA(Notepad_plus_Window::gNppHWND, ex.what(), "Runtime Exception", MB_OK);
		doException(notepad_plus_plus);
	}
	catch (const Win32Exception & ex)
	{
		isException = true;
		NppChar message[1024];
		sprintf(message, "An exception occurred. Notepad++ cannot recover and must be shut down.\r\nThe exception details are as follows:\r\n"
			"Code:\t0x%08X\r\nType:\t%S\r\nException address: 0x%p", ex.code(), ex.what(), ex.where());
		::MessageBox(Notepad_plus_Window::gNppHWND, message, "Win32Exception", MB_OK | MB_ICONERROR);
		mdump.writeDump(ex.info());
		doException(notepad_plus_plus);
	}
	catch (std::exception & ex)
	{
		isException = true;
		::MessageBoxA(Notepad_plus_Window::gNppHWND, ex.what(), "General Exception", MB_OK);
		doException(notepad_plus_plus);
	}
	catch (...) // this shouldn't ever have to happen
	{
		isException = true;
		::MessageBoxA(Notepad_plus_Window::gNppHWND, "An exception that we did not yet found its name is just caught", "Unknown Exception", MB_OK);
		doException(notepad_plus_plus);
	}

	doUpdateNpp = nppGui._autoUpdateOpt._doAutoUpdate != NppGUI::autoupdate_disabled; // refresh, maybe user activated these opts in Preferences
	updateAtExit = nppGui._autoUpdateOpt._doAutoUpdate == NppGUI::autoupdate_on_exit; // refresh
	if (!isException && !nppParameters.isEndSessionCritical() && TheFirstOne && isUpExist && isGtXP && isSignatureOK && doUpdateNpp && updateAtExit)
	{
		if (launchUpdater(updaterFullPath, updaterDir))
		{
			// for updating the nextUpdateDate in the already saved config.xml
			nppParameters.createXmlTreeFromGUIParams();
			nppParameters.saveConfig_xml();
		}
	}

	return static_cast<int>(msg.wParam);
}
