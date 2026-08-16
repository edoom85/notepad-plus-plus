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

#include <map>
#include <string>
#include <vector>

#include "NppXml.h"

class FindReplaceDlg;
class PreferenceDlg;
class ShortcutMapper;
class UserDefineDialog;
class PluginsAdminDlg;

class MenuPosition
{
public:
	constexpr MenuPosition(int x, int y, int z, const char* id) noexcept
		: _x(x), _y(y), _z(z), _id(id)
	{}

	int _x = -1; // menu
	int _y = -1; // sub-menu
	int _z = -1; // sub-sub-menu

	static const MenuPosition& getMenuPosition(const char* id);

private:
	const char* _id = nullptr; // a unique string defined in localization XML file
};


class NativeLangSpeaker
{
public:
	void init(NppXml::Document nativeLangDocRoot, bool loadIfEnglish = false);
	void changeConfigLang(HWND hDlg) const;
	void changeLangTabContextMenu(HMENU hCM) const;
	void getAlternativeNameFromTabContextMenu(NppString& output, int cmdID, bool isAlternative, const NppString& defaultValue) const;
	static NppXml::Element searchDlgNode(NppXml::Element node, const char* dlgTagName);
	bool changeDlgLang(HWND hDlg, const char* dlgTagName, char* titleOut = nullptr, size_t titleOutMaxSize = 0, const char* titleOutTag = nullptr) const;
	void changeLangTabDropContextMenu(HMENU hCM) const;
	void changeLangTrayIconContexMenu(HMENU hCM) const;
	NppString getSubMenuEntryName(const char* nodeName) const;
	NppString getNativeLangMenuString(int itemID, const NppString& inCaseOfFailureStr = "", bool removeMarkAnd = false) const;
	NppString getShortcutNameString(int itemID) const;

	void changeMenuLang(HMENU menuHandle) const;
	static void changeStyleCtrlsLang(HWND hDlg, int* idArray, const char** translatedText);
	void changeUserDefineLang(UserDefineDialog* userDefineDlg) const;
	void changeUserDefineLangPopupDlg(HWND hDlg) const;
	void changeFindReplaceDlgLang(FindReplaceDlg& findReplaceDlg);
	void changePreferenceDlgLang(PreferenceDlg& preference) const;
	void changePluginsAdminDlgLang(PluginsAdminDlg& pluginsAdminDlg);

	bool getDoSaveOrNotStrings(NppString& title, NppString& msg) const;

	bool isRTL() const {
		return _isRTL;
	}

	bool isEditZoneRTL() const {
		return _isEditZoneRTL;
	}

	const char* getFileName() const {
		return _fileName;
	}

	const NppXml::Element& getNativeLang() const {
		return _nativeLang;
	}

	static int getLangEncoding() {
		return _nativeLangEncoding;
	}

	bool getMsgBoxLang(const char* msgBoxTagName, NppString& title, NppString& message) const;
	NppString getShortcutMapperLangStr(const char *nodeName, const NppChar* defaultStr) const;
	NppString getProjectPanelLangMenuStr(const char* nodeName, int cmdID, const NppChar* defaultStr) const;
	NppString getDlgLangMenuStr(const char* firstLevelNodeName, const char* secondLevelNodeName, int cmdID, const NppChar *defaultStr) const;
	NppString getCmdLangStr(const std::vector<const char*>& nodeNames, int cmdID, const NppChar* defaultStr) const;
	NppString getAttrNameStr(const NppChar* defaultStr, const char* nodeL1Name, const char* nodeL2Name, const char* nodeL3Name = "name") const;
	static NppString getAttrNameByIdStr(const NppChar* defaultStr, NppXml::Element targetNode, const char* nodeL1Value, const char* nodeL1Name = "id", const char* nodeL2Name = "name");
	std::string getLocalizedStrFromID(const char* strID, const std::string& defaultString) const;
	NppString getLocalizedStrFromID(const char* strID, const NppString& defaultString) const;
	void getMainMenuEntryName(NppString& dest, HMENU hMenu, const char* menuId, const NppChar* defaultDest);

	void resetShortcutMenuNameMap() {
		_shortcutMenuEntryNameMap.clear();
	}

	int messageBox(const char* msgBoxTagName, HWND hWnd, const NppChar* defaultMessage, const NppChar* defaultTitle, int msgBoxType, int intInfo = 0, const NppChar* strInfo = nullptr) const;
private:
	NppXml::Element _nativeLang{};
	static constexpr int _nativeLangEncoding = CP_UTF8; // all Notepad++ xml files should be UTF8
	bool _isRTL = false; // for Notepad++ GUI
	bool _isEditZoneRTL = false; // for Scintilla
	const char* _fileName = nullptr;
	std::map<std::string, NppString> _shortcutMenuEntryNameMap;

	static void resizeCheckboxRadioBtn(HWND hWnd);
};
