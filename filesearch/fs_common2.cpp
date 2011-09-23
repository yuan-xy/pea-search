
#include "env.h"
#include "global.h"
#include "fs_common.h"
#include "search.h"
#include "util.h"
#include "desktop.h"

#include <stdio.h>
#include <algorithm>
#include <functional>
#include <vector>
#include <google/sparse_hash_map>

typedef google::sparse_hash_map<KEY, pFileEntry> sparsehash;
sparsehash DirMaps[DIRVE_COUNT_OFFLINE];

typedef std::vector<pFileEntry> FileList, *pFileList;
FileList tmpList[DIRVE_COUNT_OFFLINE];


extern "C" {

void SubDirIterate(pFileEntry dir, pFileVisitor visitor, void *data){
	if(!IsDir(dir)) return;
	pFileList children = (pFileList)dir->children;
	if(children==NULL) return;
	for(FileList::const_iterator it = children->begin(); it!= children->end(); ++it) {
		visitor(pFileEntry(*it),data);
	}
}

void SubDirIterate_p3(pFileEntry dir, pFileVisitor_p3 visitor, WCHAR *full_name, int name_len, int i){
	if(!IsDir(dir)) return;
	pFileList children = (pFileList)dir->children;
	if(children==NULL) return;
	for(FileList::const_iterator it = children->begin(); it!= children->end(); ++it) {
		visitor(pFileEntry(*it),full_name,name_len,i);
	}
}

pFileEntry SubDirIterateB(pFileEntry dir, pFileVisitorB visitor, void *data){
	if(!IsDir(dir)) return NULL;
	pFileList children = (pFileList)dir->children;
	if(children==NULL) return NULL;
	for(FileList::const_iterator it = children->begin(); it!= children->end(); ++it) {
		pFileEntry fe = pFileEntry(*it);
		if(visitor(fe,data)){
			return fe;
		}
	}
	return NULL;
}

void FilesIterate(pFileEntry file,pFileVisitor visitor, void *data){
	visitor(file,data);
	if(!IsDir(file)) return;
	pFileList children = (pFileList)file->children;
	if(children==NULL) return;
	for(FileList::const_iterator it = children->begin(); it!= children->end(); ++it) {
		FilesIterate(pFileEntry(*it),visitor,data);
	}
}

BOOL is_recycle(pFileEntry pf,int i){
	return (strnicmp((const char *)pf->FileName,"RECYCLER",8)==0 && IsNtfs(i)) ||
	(strnicmp((const char *)pf->FileName,"Recycled",8)==0 && !IsNtfs(i));
}

void AllFilesIterate(pFileVisitor visitor, void *data, BOOL offline){
	int i=0;
	if(offline){
		for(i=DIRVE_COUNT;i<DIRVE_COUNT_OFFLINE;i++){
			if(g_rootVols[i]!=NULL){
				FilesIterate(g_rootVols[i],visitor,data);
			}
		}
	}else{
		for(;i<26;i++){
			if(g_loaded[i] && g_rootVols[i]!=NULL){
				pFileList children = (pFileList)g_rootVols[i]->children;
				if(children==NULL) return;
				for(FileList::const_iterator it = children->begin(); it!= children->end(); ++it) {
					pFileEntry pf = pFileEntry(*it);
					if(!is_recycle(pf,i)){
						FilesIterate(pf,visitor,data);
					}
				}
			}
		}
		if(get_desktop()!=NULL) FilesIterate(get_desktop(),visitor,data);
	}
}

void addChildren(pFileEntry parent, pFileEntry file){
	pFileList children;
	file->up.parent = parent;
	children = (FileList *)parent->children;
	if(children==NULL || children->size()==0){
		FileList *list = new FileList;
		list->push_back(file);
		parent->children = (pFileList) list;
	}else{
		//if(children->size() > 10) printf("%d,%ls\n",children->size(), parent->FileName);
		children->push_back(file);
	}
}

__forceinline void add2Map(pFileEntry file,int i){
	tmpList[i].push_back(file);
	if(IsDir(file)) DirMaps[i][file->FileReferenceNumber] = file;
}

void resetMap(int i){
	tmpList[i].clear();
	FileList(0).swap(tmpList[i]);
	DirMaps[i].clear();
	sparsehash tmp;
	tmp.swap(DirMaps[i]);
}

__forceinline pFileEntry findDir(KEY frn,int i){
	return DirMaps[i][frn];
}

void build_dir(int i) {
	for (FileList::const_iterator it = tmpList[i].begin(); it
			!= tmpList[i].end(); ++it) {
		pFileEntry fe = pFileEntry(*it);
		attachParent(fe, i);
	}
	tmpList[i].clear();
	FileList(0).swap(tmpList[i]);
}

class pFileEntry_eq : public std::unary_function<pFileEntry, bool> {
	pFileEntry frn;
public:
	explicit pFileEntry_eq(const pFileEntry& k) { frn = k;}
	bool operator() (const pFileEntry& p) const {
		if(frn->FileReferenceNumber != 0)
			return p->FileReferenceNumber == frn->FileReferenceNumber; //ntfs
		else
			return strncmp((const char *)frn->FileName, (const char *)p->FileName, frn->us.v.FileNameLength)==0;  //fat
	}
};

static void delete_file_from_parent_vector(pFileEntry file,pFileEntry parent){
	try{
		pFileList children = (pFileList)parent->children;
		FileList::iterator it = find_if(children->begin(),children->end(),pFileEntry_eq(file));
		if(it!= children->end()) children->erase(it);
	}catch(...){
		CPP_ERROR;
	}
}

void renameFile(pFileEntry file, wchar_t *new_name, int name_byte_len){
	if(file==NULL){
		return;
	}else{
		int str_len = name_byte_len/sizeof(wchar_t);
		int len = WCHAR_TO_UTF8_LEN(new_name,str_len);
		NEW0_FILE(ret,len);
		memcpy(ret,file,sizeof(FileEntry));
		ret->us.v.FileNameLength = len;
		ret->us.v.StrLen = str_len;
		WCHAR_TO_UTF8(new_name,str_len,ret->FileName,len);
		if(file->up.parent!=NULL){
			addChildren(file->up.parent,ret);
		}
		pFileList children = (pFileList)file->children;
		if(children!=NULL){
			for(FileList::const_iterator it = children->begin(); it!= children->end(); ++it) {
				pFileEntry(*it)->up.parent = ret;
			}
		}
		delete_file_from_parent_vector(file,file->up.parent);
		free_safe(file);
	}
}

void moveFile(pFileEntry file, pFileEntry pnew){
	if(file==NULL || pnew==NULL) return;
	delete_file_from_parent_vector(file,file->up.parent);
	file->up.parent = pnew;
	addChildren(pnew,file);
}

static void deleteDir(pFileEntry file){
	if(file==NULL) return;
	if(IsDir(file)){
		pFileList children = (pFileList)file->children;
		if(children!=NULL){
			for(FileList::iterator it = children->begin(); it!= children->end(); ++it) {
				pFileEntry p = pFileEntry(*it);
				deleteDir(p);
			}
			children->clear();
			FileList(0).swap(*children);
		}
		file->children = NULL;
	}
	free_safe(file);
	ALL_FILE_COUNT--;
}

void deleteFile(pFileEntry file){
	if(file==NULL) return;
	if(file->up.parent!=NULL){
		delete_file_from_parent_vector(file,file->up.parent);
	}
	if(IsDir(file)){
		deleteDir(file);
	}else{
		free_safe(file);
		ALL_FILE_COUNT--;
	}
}

}// extern "C"
