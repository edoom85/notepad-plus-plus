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

#include <memory>
#include <string>
#include <vector>

#include "DockingDlgInterface.h"
#include "NppXml.h"
#include "ProjectPanel_rc.h"
#include "StaticDialog.h"
#include "TreeView.h"

#define PM_PROJECTPANELTITLE       "Project Panel"
#define PM_WORKSPACEROOTNAME       "Workspace"
#define PM_NEWFOLDERNAME           "Folder Name"
#define PM_NEWPROJECTNAME          "Project Name"

#define PM_NEWWORKSPACE            "New Workspace"
#define PM_OPENWORKSPACE           "Open Workspace"
#define PM_RELOADWORKSPACE         "Reload Workspace"
#define PM_SAVEWORKSPACE           "Save"
#define PM_SAVEASWORKSPACE         "Save As..."
#define PM_SAVEACOPYASWORKSPACE    "Save a Copy As..."
#define PM_NEWPROJECTWORKSPACE     "Add New Project"
#define PM_FINDINFILESWORKSPACE    "Find in Projects..."

#define PM_EDITRENAME              "Rename"
#define PM_EDITNEWFOLDER           "Add Folder"
#define PM_EDITADDFILES            "Add Files..."
#define PM_EDITADDFILESRECUSIVELY  "Add Files from Directory..."
#define PM_EDITREMOVE              "Remove\tDEL"
#define PM_EDITMODIFYFILE          "Modify File Path"

#define PM_WORKSPACEMENUENTRY      "Workspace"
#define PM_EDITMENUENTRY           "Edit"

#define PM_MOVEUPENTRY             "Move Up\tCtrl+Up"
#define PM_MOVEDOWNENTRY           "Move Down\tCtrl+Down"

enum NodeType {
	nodeType_root = 0, nodeType_project = 1, nodeType_folder = 2, nodeType_file = 3
};

class CustomFileDialog;

class ProjectPanel : public DockingDlgInterface {
public:
	ProjectPanel() : DockingDlgInterface(IDD_PROJECTPANEL) {}
	~ProjectPanel() override = default;

	void init(HINSTANCE hInst, HWND hPere, int panelID) {
		DockingDlgInterface::init(hInst, hPere);
		_panelID = panelID;
	}

	void setParent(HWND parent2set) {
		_hParent = parent2set;
	}

	void setPanelTitle(const NppString& title) {
		_panelTitle = title;
	}
	const NppChar* getPanelTitle() const {
		return _panelTitle.c_str();
	}

	void newWorkSpace();
	bool saveWorkspaceRequest();
	bool openWorkSpace(const NppChar* projectFileName, bool force = false);
	bool saveWorkSpace();
	bool saveWorkSpaceAs(bool saveCopyAs);
	void setWorkSpaceFilePath(const NppChar* projectFileName) {
		_workSpaceFilePath = projectFileName;
	}
	const NppChar* getWorkSpaceFilePath() const {
		return _workSpaceFilePath.c_str();
	}
	bool isDirty() const {
		return _isDirty;
	}
	bool checkIfNeedSave();

	void setBackgroundColor(COLORREF bgColour) override {
		TreeView_SetBkColor(_treeView.getHSelf(), bgColour);
	}
	void setForegroundColor(COLORREF fgColour) override {
		TreeView_SetTextColor(_treeView.getHSelf(), fgColour);
	}
	bool enumWorkSpaceFiles(HTREEITEM tvFrom, const std::vector<NppString>& patterns, std::vector<NppString>& fileNames);

protected:
	TreeView _treeView;
	HIMAGELIST _hImaLst = nullptr;
	HWND _hToolbarMenu = nullptr;
	HMENU _hWorkSpaceMenu = nullptr;
	HMENU _hProjectMenu = nullptr;
	HMENU _hFolderMenu = nullptr;
	HMENU _hFileMenu = nullptr;
	NppString _panelTitle;
	NppString _workSpaceFilePath;
	NppString _selDirOfFilesFromDirDlg;
	bool _isDirty = false;
	int _panelID = 0;

	void initMenus();
	void destroyMenus() const;
	void addFiles(HTREEITEM hTreeItem);
	void addFilesFromDirectory(HTREEITEM hTreeItem);
	void recursiveAddFilesFrom(const NppChar* folderPath, HTREEITEM hTreeItem);
	HTREEITEM addFolder(HTREEITEM hTreeItem, const NppChar* folderName);

	bool writeWorkSpace(const NppChar* projectFileName = nullptr, bool doUpdateGUI = true);
	NppString getRelativePath(const NppString& filePath, const NppChar* workSpaceFileName);
	void buildProjectXml(NppXml::Element& root, HTREEITEM hItem, const NppChar* fn2write);
	NodeType getNodeType(HTREEITEM hItem);
	void setWorkSpaceDirty(bool isDirty);
	void popupMenuCmd(int cmdID);
	POINT getMenuDisplayPoint(int iButton) const;
	intptr_t CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;
	bool buildTreeFrom(const NppXml::Element& projectRoot, HTREEITEM hParentItem);
	void notified(LPNMHDR notification);
	void showContextMenu(int x, int y);
	void showContextMenuFromMenuKey(HTREEITEM selectedItem, int x, int y);
	HMENU getMenuHandler(HTREEITEM selectedItem);
	NppString getAbsoluteFilePath(const NppChar* relativePath);
	void openSelectFile();
	void setFileExtFilter(CustomFileDialog& fDlg);
	std::vector<std::unique_ptr<NppString>> _fullPathStrs;

private:
	using DockingDlgInterface::init;
};

class FileRelocalizerDlg : public StaticDialog
{
public:
	FileRelocalizerDlg() = default;

	int doDialog(const NppChar* fn, bool isRTL = false);

	void destroy() override {}

	const NppString& getFullFilePath() const {
		return _fullFilePath;
	}

protected:
	intptr_t CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	NppString _fullFilePath;
};
