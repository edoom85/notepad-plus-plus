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

#include <string>

#include "StaticDialog.h"

#define CURRENTWORD_MAXLENGTH 2048

const NppChar fullCurrentPath[] = "FULL_CURRENT_PATH";
const NppChar currentDirectory[] = "CURRENT_DIRECTORY";
const NppChar onlyFileName[] = "FILE_NAME";
const NppChar fileNamePart[] = "NAME_PART";
const NppChar fileExtPart[] = "EXT_PART";
const NppChar currentWord[] = "CURRENT_WORD";
const NppChar nppDir[] = "NPP_DIRECTORY";
const NppChar nppFullFilePath[] = "NPP_FULL_FILE_PATH";
const NppChar currentLine[] = "CURRENT_LINE";
const NppChar currentColumn[] = "CURRENT_COLUMN";
const NppChar currentLineStr[] = "CURRENT_LINESTR";

int whichVar(NppChar *str);
void expandNppEnvironmentStrs(const NppChar *strSrc, NppChar *stringDest, size_t strDestLen, HWND hWnd);

class Command {
public :
	Command() = default;
	explicit Command(const NppChar* cmd) : _cmdLine(cmd) {}
	explicit Command(const NppString& cmd) : _cmdLine(cmd) {}
	HINSTANCE run(HWND hWnd);
	HINSTANCE run(HWND hWnd, const NppChar* cwd);

protected :
	NppString _cmdLine;
private :
	void extractArgs(NppChar *cmd2Exec, size_t cmd2ExecLen, NppChar *args, size_t argsLen, const NppChar *cmdEntier);
};

class RunDlg : public Command, public StaticDialog
{
public :
	RunDlg() = default;

	void doDialog(bool isRTL = false);
	void destroy() override {}

protected :
	void insertVariable(unsigned char id);
	intptr_t CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;

private :
	void addTextToCombo(const NppChar *txt2Add) const;
	void removeTextFromCombo(const NppChar *txt2Remove) const;
};
