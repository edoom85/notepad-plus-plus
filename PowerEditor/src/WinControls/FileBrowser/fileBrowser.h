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
#include <unordered_set>
#include <vector>

#include "DockingDlgInterface.h"
#include "TreeView.h"
#include "fileBrowser_rc.h"

#define FB_PANELTITLE         "Folder as Workspace"
#define FB_ADDROOT            "Add"
#define FB_REMOVEALLROOTS     "Remove All"
#define FB_REMOVEROOTFOLDER   "Remove"
#define FB_COPYPATH           "Copy path"
#define FB_COPYFILENAME       "Copy file name"
#define FB_FINDINFILES        "Find in Files..."
#define FB_EXPLORERHERE       "Explorer here"
#define FB_CMDHERE            "CMD here"
#define FB_POWERSHELLHERE     "PowerShell here"
#define FB_OPENINNPP          "Open"
#define FB_SHELLEXECUTE       "Run by system"

#define FOLDERASWORKSPACE_NODE "FolderAsWorkspace"

class FileBrowser;
class FolderInfo;

class FileInfo final
{
friend class FileBrowser;
friend class FolderInfo;

public:
	FileInfo() = delete; // constructor by default is forbidden
	explicit FileInfo(const NppString& name) noexcept : _name(name) {}
	const NppString& getName() const { return _name; }
	void setName(const NppString& name) { _name = name; }

private:
	NppString _name;
};

class FolderInfo final
{
friend class FileBrowser;
friend class FolderUpdater;

public:
	FolderInfo() = delete; // constructor by default is forbidden
	FolderInfo(const NppString& name, FolderInfo* parent) : _name(name), _parent(parent) {}
	void setRootPath(const NppString& rootPath) { _rootPath = rootPath; }
	const NppString& getRootPath() const { return _rootPath; }
	void setName(const NppString& name) { _name = name; }
	const NppString& getName() const { return _name; }
	void addFile(const NppString& fn) { _files.push_back(FileInfo(fn)); }
	void addSubFolder(FolderInfo subDirectoryStructure) { _subFolders.push_back(subDirectoryStructure); }

	bool addToStructure(NppString & fullpath, std::vector<NppString> linarPathArray);
	bool removeFromStructure(std::vector<NppString> linarPathArray);
	bool renameInStructure(std::vector<NppString> linarPathArrayFrom, std::vector<NppString> linarPathArrayTo);

private:
	std::vector<FolderInfo> _subFolders;
	std::vector<FileInfo> _files;
	NppString _name;
	FolderInfo* _parent = nullptr;
	NppString _rootPath; // set only for root folder; empty for normal folder
};

enum BrowserNodeType {
	browserNodeType_root = 0, browserNodeType_folder = 2, browserNodeType_file = 3
};

class FolderUpdater {
friend class FileBrowser;
public:
	FolderUpdater(const FolderInfo& fi, FileBrowser *pFileBrowser) : _rootFolder(fi), _pFileBrowser(pFileBrowser) {}
	~FolderUpdater() = default;

	void startWatcher();
	void stopWatcher();

private:
	FolderInfo _rootFolder;
	FileBrowser* _pFileBrowser = nullptr;
	HANDLE _watchThreadHandle = nullptr;
	HANDLE _EventHandle = nullptr;
	static DWORD WINAPI watching(void* params);

	static void processChange(DWORD dwAction, std::vector<NppString> filesToChange, FolderUpdater* thisFolderUpdater);
};

struct SortingData4lParam {
	NppString _rootPath; // Only for the root. It should be empty if it's not root
	NppString _label;    // TreeView item label
	bool _isFolder = false;   // if it's not a folder, then it's a file

	SortingData4lParam(NppString rootPath, NppString label, bool isFolder) : _rootPath(rootPath), _label(label), _isFolder(isFolder) {}
};


class FileBrowser : public DockingDlgInterface {
public:
	FileBrowser(): DockingDlgInterface(IDD_FILEBROWSER) {}
	~FileBrowser() override;

	void setParent(HWND parent2set){
		_hParent = parent2set;
	}

	void setBackgroundColor(COLORREF bgColour) override {
		TreeView_SetBkColor(_treeView.getHSelf(), bgColour);
	}

	void setForegroundColor(COLORREF fgColour) override {
		TreeView_SetTextColor(_treeView.getHSelf(), fgColour);
	}

	NppString getNodePath(HTREEITEM node) const;
	NppString getNodeName(HTREEITEM node) const;
	void addRootFolder(NppString rootFolderPath, std::unordered_set<NppString>* pExpandedPaths = nullptr);

	void applyExpandState(HTREEITEM rootHItem, std::unordered_set<NppString>* pExpandedPaths);
	std::vector<NppString> getExpandedPathsFromFaW() const;

	HTREEITEM getRootFromFullPath(const NppString & rootPath) const;
	HTREEITEM findChildNodeFromName(HTREEITEM parent, const NppString& label) const;

	HTREEITEM findInTree(const NppString& rootPath, HTREEITEM node, std::vector<NppString> linarPathArray) const;

	void deleteAllFromTree() {
		popupMenuCmd(IDM_FILEBROWSER_REMOVEALLROOTS);
	}

	bool renameInTree(const NppString& rootPath, HTREEITEM node, const std::vector<NppString>& linarPathArrayFrom, const NppString & renameTo);

	std::vector<NppString> getRoots() const;
	NppString getSelectedItemPath() const;

	bool selectItemFromPath(const NppString& itemPath) const;

protected:
	HWND _hToolbarMenu = nullptr;
	std::vector<HIMAGELIST> _iconListVector;

	TreeView _treeView;

	HIMAGELIST _hImaLst = nullptr;

	HMENU _hGlobalMenu = NULL;
	HMENU _hRootMenu = NULL;
	HMENU _hFolderMenu = NULL;
	HMENU _hFileMenu = NULL;
	std::vector<FolderUpdater *> _folderUpdaters;

	NppString _selectedNodeFullPath; // this member is used only for PostMessage call

	std::vector<SortingData4lParam*> _sortingDataArray;

	NppString _expandAllFolders = "Unfold all";
	NppString _collapseAllFolders = "Fold all";
	NppString _locateCurrentFile = "Locate current file";

	void initPopupMenus();
	void destroyMenus();

	BrowserNodeType getNodeType(HTREEITEM hItem);
	void popupMenuCmd(int cmdID);

	bool selectCurrentEditingFile() const;

	struct FilesToChange {
		NppString _commonPath; // Common path between all the files. _rootPath + _linarWithoutLastPathElement
		NppString _rootPath;
		std::vector<NppString> _linarWithoutLastPathElement;
		std::vector<NppString> _files; // file/folder names
	};

	std::vector<FilesToChange> getFilesFromParam(LPARAM lParam) const;

	bool addToTree(FilesToChange & group, HTREEITEM node);

	bool deleteFromTree(FilesToChange & group);

	std::vector<HTREEITEM> findInTree(FilesToChange & group, HTREEITEM node) const;

	std::vector<HTREEITEM> findChildNodesFromNames(HTREEITEM parent, std::vector<NppString> & labels) const;

	void removeNamesAlreadyInNode(HTREEITEM parent, std::vector<NppString> & labels) const;

	intptr_t CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;
	void notified(LPNMHDR notification);
	void showContextMenu(int x, int y);
	void openSelectFile();
	void getDirectoryStructure(const NppChar *dir, const std::vector<NppString> & patterns, FolderInfo & directoryStructure, bool isRecursive, bool isInHiddenDir); 
	HTREEITEM createFolderItemsFromDirStruct(HTREEITEM hParentItem, const FolderInfo & directoryStructure);
	static int CALLBACK categorySortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};
