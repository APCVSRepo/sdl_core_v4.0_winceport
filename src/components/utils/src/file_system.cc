/*
 * Copyright (c) 2014, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "utils/logger.h"

#if defined(OS_WIN32) || defined(OS_WINCE)
#include <Windows.h>
#include <sstream>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <sys/stat.h>
#include <sys/types.h>
#else
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>

#include <dirent.h>
#endif
#include <unistd.h>
// TODO(VS): lint error: Streams are highly discouraged.
#include <fstream>
#include <cstddef>
#include <cstdio>
#include <algorithm>

#ifdef OS_WIN32
#pragma comment(lib, "shlwapi.lib")
#endif

#ifdef OS_WINCE
#include "utils/global.h"
#endif

#include "utils/file_system.h"

CREATE_LOGGERPTR_GLOBAL(logger_, "Utils")

uint64_t file_system::GetAvailableDiskSpace(const std::string& path) {
#if defined(OS_WIN32)
	uint64_t i64FreeBytesToCaller;
	uint64_t i64TotalBytes;
	uint64_t i64FreeBytes;
	BOOL fResult = GetDiskFreeSpaceEx(
		path.c_str(),
		(PULARGE_INTEGER)&i64FreeBytesToCaller,
		(PULARGE_INTEGER)&i64TotalBytes,
		(PULARGE_INTEGER)&i64FreeBytes);

	return fResult ? i64FreeBytes : 0;
#elif defined (OS_WINCE)
	uint64_t i64FreeBytesToCaller;
	uint64_t i64TotalBytes;
	uint64_t i64FreeBytes;
	
	BOOL fResult = GetDiskFreeSpaceEx(
    Global::StringToWString(path).c_str(),
		(PULARGE_INTEGER)&i64FreeBytesToCaller,
		(PULARGE_INTEGER)&i64TotalBytes,
		(PULARGE_INTEGER)&i64FreeBytes);

	return fResult ? i64FreeBytes : 0;
#else
  struct statvfs fsInfo = { 0 };
  if (statvfs(path.c_str(), &fsInfo) == 0) {
    return fsInfo.f_bsize * fsInfo.f_bfree;
  } else {
    return 0;
  }
#endif
}

uint32_t file_system::FileSize(const std::string &path) {
#if defined(OS_WIN32)
    uint32_t uSz = 0;
    if (file_system::FileExists(path)) {
        std::ifstream ifile;
        ifile.open(path.c_str());
        if (ifile.is_open()) {
            ifile.seekg(0, std::ios::end);
            uSz = ifile.tellg();
        }
        ifile.clear();
    }
    return uSz;
#elif defined(OS_WINCE)
  uint32_t uSz = 0;
  std::string absPath = path;

  if (absPath[0] != '\\' && absPath[0] != '/') {
    absPath = Global::RelativePathToAbsPath(absPath.c_str());
  }

  if (file_system::FileExists(absPath)) {
      std::ifstream ifile;
      ifile.open(absPath.c_str());
      if (ifile.is_open()) {
          ifile.seekg(0, std::ios::end);
          uSz = ifile.tellg();
      }
      ifile.clear();
  }
  return uSz;
#else
  if (file_system::FileExists(path)) {
    struct stat file_info;
    memset(reinterpret_cast<void*>(&file_info), 0, sizeof(file_info));
    stat(path.c_str(), &file_info);
    return file_info.st_size;
  }
  return 0;
#endif
}

uint32_t file_system::DirectorySize(const std::string& path) {
#if defined(OS_WIN32)
    uint32_t uSz = 0;
    WIN32_FIND_DATA fData;

    HANDLE hFind = FindFirstFile((path + "\\*.*").c_str(), &fData);

    if (hFind == INVALID_HANDLE_VALUE) return uSz;

    do {
        if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (std::string(fData.cFileName) != "." && std::string(fData.cFileName) != "..") {
                DirectorySize(path + "\\" + fData.cFileName);
            }
        }
        else {
            uSz += (fData.nFileSizeHigh * (MAXDWORD + 1)) + fData.nFileSizeLow;
        }
    } while (FindNextFile(hFind, &fData));

    FindClose(hFind);

    return uSz;
#elif defined(OS_WINCE)
    uint32_t uSz = 0;
    WIN32_FIND_DATA fData;
    std::wstring tmpStr;
    std::string absPath = path;

    if (absPath[0] != '\\' && absPath[0] != '/') {
      absPath = Global::RelativePathToAbsPath(absPath.c_str());
    }

    HANDLE hFind = FindFirstFile(Global::StringToWString(absPath + "\\*.*").c_str(), &fData);

    if (hFind == INVALID_HANDLE_VALUE) return uSz;

    do {
        if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (std::wstring(fData.cFileName) != L"." && std::wstring(fData.cFileName) != L"..") {
                DirectorySize(absPath + "\\" + Global::WStringToString(fData.cFileName));
            }
        }
        else {
            uSz += (fData.nFileSizeHigh * (MAXDWORD + 1)) + fData.nFileSizeLow;
        }
    } while (FindNextFile(hFind, &fData));

    FindClose(hFind);

    return uSz;
#else
  uint32_t size = 0;
  int32_t return_code = 0;
  DIR* directory = NULL;

#ifndef __QNXNTO__
  struct dirent dir_element_;
  struct dirent* dir_element = &dir_element_;
#else
  char* direntbuffer = new char[offsetof(struct dirent, d_name) +
                                pathconf(path.c_str(), _PC_NAME_MAX) + 1];
  struct dirent* dir_element = new (direntbuffer) dirent;
#endif
  struct dirent* result = NULL;
  struct stat file_info;
  directory = opendir(path.c_str());
  if (NULL != directory) {
    return_code = readdir_r(directory, dir_element, &result);
    for (; NULL != result && 0 == return_code;
         return_code = readdir_r(directory, dir_element, &result)) {
      if (0 == strcmp(result->d_name, "..") ||
          0 == strcmp(result->d_name, ".")) {
        continue;
      }
      std::string full_element_path = path + "/" + result->d_name;
      if (file_system::IsDirectory(full_element_path)) {
        size += DirectorySize(full_element_path);
      } else {
        stat(full_element_path.c_str(), &file_info);
        size += file_info.st_size;
      }
    }
  }
  closedir(directory);
#ifdef __QNXNTO__
  delete[] direntbuffer;
#endif
  return size;
#endif
}

#if defined(OS_WIN32) || defined(OS_WINCE)
std::string file_system::CreateDirectoryWindows(const std::string& name) {
	if (!DirectoryExists(name)) {
#if defined(OS_WIN32)
		::CreateDirectory(name.c_str(), NULL);
#elif defined(OS_WINCE)
  std::wstring absPath;

  if (name[0] != '\\' && name[0] != '/') {
    absPath = Global::RelativePathToAbsPath(Global::StringToWString(name).c_str());
  }
  else {
    absPath = Global::StringToWString(name);
  }

  CreateDirectory(absPath.c_str(), NULL);
#else
		mkdir(name.c_str(), S_IRWXU);
#endif
	}
	return name;
}
#else
std::string file_system::CreateDirectory(const std::string& name) {
	if (!DirectoryExists(name)) {
		mkdir(name.c_str(), S_IRWXU);
	}
	return name;
}
#endif

bool file_system::CreateDirectoryRecursively(const std::string& path) {
  size_t pos = 0;
  bool ret_val = true;
  std::string fp = path;

#if defined(OS_WIN32) || defined(OS_WINCE)
  // Replace all symbol '/' with '\'
  for (std::string::iterator it = fp.begin(); it != fp.end(); it++) {
      if (*it == '/') *it = '\\';
  }
#endif

#if defined(OS_WIN32)
  std::string desPath;

  if (fp.substr(fp.length() - 1, 1) != "\\") fp = fp + "\\";
  while (ret_val == true && pos <= fp.length()) {
      pos = fp.find('\\', pos + 1);
      desPath = fp.substr(0, pos);
      if (!DirectoryExists(desPath)) {
          if (0 == ::CreateDirectory(desPath.c_str(), NULL)) {
        ret_val = false;
      }
    }
  }
#elif defined(OS_WINCE)
  std::string desPath;

  if (fp[0] != '\\' && fp[0] != '/') {
    fp = Global::RelativePathToAbsPath(fp.c_str());
  }

  if (fp.substr(fp.length() - 1, 1) != "\\") fp = fp + "\\";
  while (ret_val == true && pos <= fp.length()) {
    pos = fp.find('\\', pos + 1);
    desPath = fp.substr(0, pos);
    if (!DirectoryExists(desPath)) {
      if (0 == ::CreateDirectory(Global::StringToWString(desPath).c_str(), NULL)) {
        ret_val = false;
      }
    }
  }
#else
  if (fp.substr(fp.length() - 1, 1) != "/") fp = fp + "/";
  while (ret_val == true && pos <= fp.length()) {
      pos = fp.find('/', pos + 1);
      if (!DirectoryExists(fp.substr(0, pos))) {
          if (0 != mkdir(fp.substr(0, pos).c_str(), S_IRWXU)) {
              ret_val = false;
          }
      }
  }
#endif

  return ret_val;
}

bool file_system::IsDirectory(const std::string& name) {
#if defined(OS_WIN32)
	int fileAttri = GetFileAttributes(name.c_str());

	if(fileAttri != -1) {
		if (fileAttri & FILE_ATTRIBUTE_DIRECTORY) {
			return true;
		}	
	}
	return false;
#elif defined(OS_WINCE)
  int fileAttri = GetFileAttributes(Global::RelativePathToAbsPath(Global::StringToWString(name).c_str()).c_str());

	if(fileAttri != -1) {
		if (fileAttri & FILE_ATTRIBUTE_DIRECTORY) {
			return true;
		}	
	}
	return false;
#else
  struct stat status;
  memset(&status, 0, sizeof(status));

  if (-1 == stat(name.c_str(), &status)) {
    return false;
  }

  return S_ISDIR(status.st_mode);
#endif
}

bool file_system::DirectoryExists(const std::string& name) {
#if defined(OS_WIN32)
    int fattr = GetFileAttributes(name.c_str());
    return INVALID_FILE_ATTRIBUTES != fattr;
#elif defined(OS_WINCE)
  std::wstring tmp;
  if (name[0] != '\\' && name[0] != '/') {
    tmp = Global::RelativePathToAbsPath(Global::StringToWString(name).c_str());
  }
  else {
    tmp = Global::StringToWString(name);
  }

  return INVALID_FILE_ATTRIBUTES != GetFileAttributes(tmp.c_str());
#else
  struct stat status;
  memset(&status, 0, sizeof(status));

  if (-1 == stat(name.c_str(), &status) || !S_ISDIR(status.st_mode)) {
    return false;
  }

  return true;
#endif
}

bool file_system::FileExists(const std::string& name) {
#if defined(OS_WIN32)
	HANDLE file = ::CreateFile(name.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	bool b = !(file == (HANDLE)-1);
	if (b)
		::CloseHandle((HANDLE)file);
	return b;
#elif defined(OS_WINCE)

  std::string absPath = name;

  if (absPath[0] != '\\' && absPath[0] != '/') {
    absPath = Global::RelativePathToAbsPath(absPath.c_str());
  }

  struct stat status;
  memset(&status, 0, sizeof(status));
  return (0 == stat(absPath.c_str(), &status));
#else
  struct stat status;
  memset(&status, 0, sizeof(status));

  if (-1 == stat(name.c_str(), &status)) {
    return false;
  }
  return true;
#endif
}

bool file_system::Write(const std::string& file_name,
                        const std::vector<uint8_t>& data,
                        std::ios_base::openmode mode) {
#ifdef OS_WINCE
  std::string absName = file_name;

  if (absName[0] != '\\' && absName[0] != '/') {
    absName = Global::RelativePathToAbsPath(absName.c_str());
  }

  std::ofstream file(absName.c_str(), std::ios_base::binary | mode);
  if (file.is_open()) {
    for (uint32_t i = 0; i < data.size(); ++i) {
      file << data[i];
    }
    file.close();
    return true;
  }
#else
  std::ofstream file(file_name.c_str(), std::ios_base::binary | mode);
  if (file.is_open()) {
    for (uint32_t i = 0; i < data.size(); ++i) {
      file << data[i];
    }
    file.close();
    return true;
  }
#endif

  return false;
}

std::ofstream* file_system::Open(const std::string& file_name,
                                 std::ios_base::openmode mode) {
  if (file_name.empty())  {
	  return NULL;
  }
  std::ofstream* file = new std::ofstream();
  std::string file_name_=file_name;
  if (NULL == file) {
    return NULL;
  }
#ifdef OS_WINCE
  if (file_name_[0] != '\\' && file_name_[0] != '/') {
    file_name_ = Global::RelativePathToAbsPath(file_name_.c_str());
  }
#endif
  file->open(file_name_.c_str(), std::ios_base::binary | mode);
  if (file->is_open()) {
    return file;
  }

  delete file;
  return NULL;
}

bool file_system::Write(std::ofstream* const file_stream,
                        const uint8_t* data,
                        uint32_t data_size) {
  bool result = false;
  if (file_stream) {
    for (size_t i = 0; i < data_size; ++i) {
      (*file_stream) << data[i];
    }
    result = true;
  }
  return result;
}

void file_system::Close(std::ofstream* file_stream) {
  if (file_stream) {
    file_stream->close();
  }
}

std::string file_system::CurrentWorkingDirectory() {
#if defined(OS_WIN32)
  char CurPath[MAX_PATH] = {0};
  GetCurrentDirectory(MAX_PATH, CurPath);

  return std::string(CurPath);
#elif defined(OS_WINCE)
  wchar_t szPath[MAX_PATH];
  GetModuleFileName(NULL, szPath, MAX_PATH);
  wchar_t *lpszPath = wcsrchr(szPath, '\\');
  *lpszPath = 0;

  return Global::WStringToString(szPath);
#else
  const size_t filename_max_length = 1024;
  char path[filename_max_length];
  if (0 == getcwd(path, filename_max_length)) {
    LOG4CXX_WARN(logger_, "Could not get CWD");
  }
  return std::string(path);
#endif
  
}

std::string file_system::GetAbsolutePath(const std::string& path) {
#if defined(OS_WIN32)
  std::string strRet;
  if (!DirectoryExists(path)) return "";

  char curPath[MAX_PATH] = { 0 };
  GetCurrentDirectory(MAX_PATH, curPath);

  char AbsPath[MAX_PATH] = { 0 };
  PathCombine(AbsPath, curPath, path.c_str());

  strRet = AbsPath;
  strRet.erase(strRet.find_last_not_of('/') + 1);
  strRet.erase(strRet.find_last_not_of('\\') + 1);

  return strRet;
#elif defined(OS_WINCE)
  std::string absPath = path;

  if (absPath[0] != '\\' && absPath[0] != '/') {
    absPath = Global::RelativePathToAbsPath(absPath.c_str());
  }
  
  if (DirectoryExists(absPath)) {
    return absPath;
  }
  else {
    return "";
  }
#else
  char abs_path[PATH_MAX];
  if (NULL == realpath(path.c_str(), abs_path)) {
    return std::string();
  }

  return std::string(abs_path);
#endif
}

#if defined(OS_WIN32)
bool file_system::DeleteFileWindows(const std::string& name) {
	return ::DeleteFile(name.c_str()) == TRUE ? true : false;
}
#elif defined(OS_WINCE)
bool file_system::DeleteFileWindows(const std::string& name) {
  std::string absPath = name;

  if (absPath[0] != '\\' && absPath[0] != '/') {
    absPath = Global::RelativePathToAbsPath(absPath.c_str());
  }

  return ::DeleteFile(Global::StringToWString(absPath).c_str()) == TRUE;
}
#else
bool file_system::DeleteFile(const std::string& name) {
  if (FileExists(name) && IsAccessible(name, W_OK)) {
    return !remove(name.c_str());
  }
  return false;
}
#endif

void file_system::remove_directory_content(const std::string& directory_name) {
#if defined(OS_WIN32)
  WIN32_FIND_DATA fData;

  HANDLE hFind = FindFirstFile((directory_name + "\\*.*").c_str(), &fData);

  if (hFind == INVALID_HANDLE_VALUE) return;

  do {
    if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (std::string(fData.cFileName) != "." && std::string(fData.cFileName) != "..") {
	      remove_directory_content(directory_name + "\\" + fData.cFileName);
	      RemoveDirectory((directory_name + "\\" + fData.cFileName).c_str());
      }
    }
    else {
      DeleteFile((directory_name + "\\" + fData.cFileName).c_str());
    }
  } while (FindNextFile(hFind, &fData));

  FindClose(hFind);
#elif defined(OS_WINCE)
  WIN32_FIND_DATA fData;
  std::wstring wdirectory_name;
  if (directory_name[0] != '\\' && directory_name[0] != '/') {
    wdirectory_name = Global::RelativePathToAbsPath(Global::StringToWString(directory_name).c_str());
  }
  else {
    wdirectory_name = Global::StringToWString(directory_name);
  }

  HANDLE hFind = FindFirstFile((wdirectory_name + L"\\*.*").c_str(), &fData);

  if (hFind == INVALID_HANDLE_VALUE) return;

  do {
    if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (std::wstring(fData.cFileName) != L"." && std::wstring(fData.cFileName) != L"..") {
        std::string subPath = Global::WStringToString(wdirectory_name + L"\\" + fData.cFileName);
        remove_directory_content(subPath);
	      BOOL rmBOOL = RemoveDirectory((wdirectory_name + L"\\" + fData.cFileName).c_str());
      }
    }
    else {
      BOOL rmBOOL = DeleteFile((wdirectory_name + L"\\" + fData.cFileName).c_str());
    }
  } while (FindNextFile(hFind, &fData));

  FindClose(hFind);
#else
  int32_t return_code = 0;
  DIR* directory = NULL;
#ifndef __QNXNTO__
  struct dirent dir_element_;
  struct dirent* dir_element = &dir_element_;
#else
  char* direntbuffer =
      new char[offsetof(struct dirent, d_name) +
               pathconf(directory_name.c_str(), _PC_NAME_MAX) + 1];
  struct dirent* dir_element = new (direntbuffer) dirent;
#endif
  struct dirent* result = NULL;

  directory = opendir(directory_name.c_str());

  if (NULL != directory) {
    return_code = readdir_r(directory, dir_element, &result);

    for (; NULL != result && 0 == return_code;
         return_code = readdir_r(directory, dir_element, &result)) {
      if (0 == strcmp(result->d_name, "..") ||
          0 == strcmp(result->d_name, ".")) {
        continue;
      }

      std::string full_element_path = directory_name + "/" + result->d_name;

      if (file_system::IsDirectory(full_element_path)) {
        remove_directory_content(full_element_path);
        rmdir(full_element_path.c_str());
      } else {
        remove(full_element_path.c_str());
      }
    }
  }

  closedir(directory);
#ifdef __QNXNTO__
  delete[] direntbuffer;
#endif
#endif
}

#if defined(OS_WIN32)
bool file_system::RemoveDirectoryWindows(const std::string& directory_name,
                                  bool is_recursively) {
    if (DirectoryExists(directory_name)
        && IsAccessible(directory_name, W_OK)) {
        if (is_recursively) {
            remove_directory_content(directory_name);
        }

        return ::RemoveDirectory(directory_name.c_str()) == TRUE;
    }
    return false;
}
#elif defined(OS_WINCE)
bool file_system::RemoveDirectoryWindows(const std::string& directory_name,
                                  bool is_recursively) {
  std::wstring tmp;
  if (directory_name[0] != '\\' && directory_name[0] != '/') {
    tmp = Global::RelativePathToAbsPath(Global::StringToWString(directory_name).c_str());
  }
  else {
    tmp = Global::StringToWString(directory_name);
  }

  if (DirectoryExists(Global::WStringToString(tmp))
    && IsAccessible(Global::WStringToString(tmp), W_OK)) {
    if (is_recursively) {
      remove_directory_content(Global::WStringToString(tmp));
    }

    return RemoveDirectory(tmp.c_str()) == TRUE;
  }
  return false;
}
#else
bool file_system::RemoveDirectory(const std::string& directory_name,
                                  bool is_recursively) {
    if (DirectoryExists(directory_name)
        && IsAccessible(directory_name, W_OK)) {
        if (is_recursively) {
            remove_directory_content(directory_name);
        }

    return !rmdir(directory_name.c_str());
  }
  return false;
}
#endif

bool file_system::IsAccessible(const std::string& name, int32_t how) {
#if defined(OS_WIN32) || defined(OS_WINCE)
    return true;
#else
    return !access(name.c_str(), how);
#endif
}

bool file_system::IsWritingAllowed(const std::string& name) {
  return IsAccessible(name, W_OK);
}

bool file_system::IsReadingAllowed(const std::string& name) {
  return IsAccessible(name, R_OK);
}

std::vector<std::string> file_system::ListFiles(
    const std::string& directory_name) {
  std::vector<std::string> listFiles;
#if defined(OS_WIN32)
  WIN32_FIND_DATA ffd;
  HANDLE hFind = ::FindFirstFile((directory_name + "\\*.*").c_str(), &ffd);

  if (INVALID_HANDLE_VALUE == hFind) return listFiles;

  // List all the files in the directory with some info about them.

  do {
    if (std::string(ffd.cFileName) != "." && std::string(ffd.cFileName) != "..") {
      listFiles.push_back(ffd.cFileName);
    }
  }
  while (FindNextFile(hFind, &ffd));

	FindClose(hFind);
#elif defined(OS_WINCE)
  WIN32_FIND_DATA ffd;
  std::string absName = directory_name;

  if (absName[0] != '\\' && absName[0] != '/') {
    absName = Global::RelativePathToAbsPath(absName.c_str());
  }

  HANDLE hFind = ::FindFirstFile(Global::StringToWString(absName + "\\*.*").c_str(), &ffd);

  if (INVALID_HANDLE_VALUE == hFind)
  {
	  return listFiles;
  }

  // List all the files in the directory with some info about them.

  do {
    if (std::wstring(ffd.cFileName) != L"." && std::wstring(ffd.cFileName) != L"..") {
      listFiles.push_back(Global::WStringToString(ffd.cFileName));
    }
  } while (FindNextFile(hFind, &ffd) != 0);

  FindClose(hFind);
#else
  if (!DirectoryExists(directory_name)) {
    return listFiles;
  }

  int32_t return_code = 0;
  DIR* directory = NULL;
#ifndef __QNXNTO__
  struct dirent dir_element_;
  struct dirent* dir_element = &dir_element_;
#else
  char* direntbuffer =
      new char[offsetof(struct dirent, d_name) +
               pathconf(directory_name.c_str(), _PC_NAME_MAX) + 1];
  struct dirent* dir_element = new (direntbuffer) dirent;
#endif
  struct dirent* result = NULL;

  directory = opendir(directory_name.c_str());
  if (NULL != directory) {
    return_code = readdir_r(directory, dir_element, &result);

    for (; NULL != result && 0 == return_code;
         return_code = readdir_r(directory, dir_element, &result)) {
      if (0 == strcmp(result->d_name, "..") ||
          0 == strcmp(result->d_name, ".")) {
        continue;
      }

      listFiles.push_back(std::string(result->d_name));
    }

    closedir(directory);
  }

#ifdef __QNXNTO__
  delete[] direntbuffer;
#endif

#endif
  return listFiles;
}

bool file_system::WriteBinaryFile(const std::string& name,
                                  const std::vector<uint8_t>& contents) {
  using namespace std;
#ifdef OS_WINCE
  std::string absName = name;

  if (absName[0] != '\\' && absName[0] != '/') {
    absName = Global::RelativePathToAbsPath(absName.c_str());
  }

  ofstream output(absName.c_str(), ios_base::binary | ios_base::trunc);
#else
  ofstream output(name.c_str(), ios_base::binary | ios_base::trunc);
#endif

  if (!contents.empty()) {
      output.write(reinterpret_cast<const char*>(&contents.front()),
          contents.size());
  }
  return output.good();
}

bool file_system::ReadBinaryFile(const std::string& name,
                                 std::vector<uint8_t>& result) {
#if defined(OS_WIN32)
  if (!FileExists(name) || !IsAccessible(name, R_OK)) {
    return false;
  }

  std::ifstream file(name.c_str(), std::ios_base::binary);
#elif defined(OS_WINCE)
  std::string absName = name;

  if (absName[0] != '\\' && absName[0] != '/') {
    absName = Global::RelativePathToAbsPath(absName.c_str());
  }

  if (!FileExists(absName) || !IsAccessible(absName, 0)) {
    return false;
  }

  std::ifstream file(absName.c_str(), std::ios_base::binary);
#else
  if (!FileExists(name) || !IsAccessible(name, R_OK)) {
    return false;
  }

  std::ifstream file(name.c_str(), std::ios_base::binary);
#endif
  std::ostringstream ss;
  ss << file.rdbuf();
  const std::string s = ss.str();

  result.resize(s.length());
  std::copy(s.begin(), s.end(), result.begin());
  return true;
}

bool file_system::ReadFile(const std::string& name, std::string& result) {
#if defined(OS_WIN32)
  if (!FileExists(name) || !IsAccessible(name, 0)) {
    return false;
  }

  std::ifstream file(name.c_str());
#elif defined(OS_WINCE)
  std::string absName = name;

  if (absName[0] != '\\' && absName[0] != '/') {
    absName = Global::RelativePathToAbsPath(absName.c_str());
  }
  if (!FileExists(absName) || !IsAccessible(absName, 0)) {
    return false;
  }

  std::ifstream file(absName.c_str());
#else
  if (!FileExists(name) || !IsAccessible(name, R_OK)) {
    return false;
  }

  std::ifstream file(name.c_str());
#endif
  std::ostringstream ss;
  ss << file.rdbuf();
  result = ss.str();
  return true;
}

const std::string file_system::ConvertPathForURL(const std::string& path) {
  std::string::const_iterator it_path = path.begin();
  std::string::const_iterator it_path_end = path.end();

  const std::string reserved_symbols = "!#$&'()*+,:;=?@[] ";
  size_t pos = std::string::npos;
  std::string converted_path;

  for (; it_path != it_path_end; ++it_path) {
    pos = reserved_symbols.find_first_of(*it_path);
    if (pos != std::string::npos) {
#if defined(OS_WIN32) || defined(OS_WINCE)
      const size_t size = 100;
      char percent_value[size];
	  sprintf_s(percent_value, size, "%%%x", *it_path);
#else
      const size_t size = 100;
      char percent_value[size];
      snprintf(percent_value, size, "%%%x", *it_path);
#endif
      converted_path += percent_value;
    } else {
      converted_path += *it_path;
    }
  }
  return converted_path;
}

#if defined(OS_WIN32)
bool file_system::CreateFileWindows(const std::string& path) {
  std::ofstream file;
  if (path.substr(0, 2) == ".\\") {
    file.open(path.substr(2, path.size() - 2).c_str());
  }
  else {
    file.open(path.c_str());
  }
  if (!(file.is_open())) {
    return false;
  } else {
    file.close();
    return true;
  }
}
#elif defined(OS_WINCE)
bool file_system::CreateFileWindows(const std::string& path) {
  std::ofstream file;
  std::string absPath = path;

  if (absPath[0] != '\\' && absPath[0] != '/') {
    absPath = Global::RelativePathToAbsPath(absPath.c_str());
  }

  file.open(absPath.c_str());
  if (file.is_open()) {
    file.close();
    return true;
  }

  return false;
}
#else
bool file_system::CreateFile(const std::string& path) {
  std::ofstream file(path);
  if (!(file.is_open())) {
    return false;
  } else {
    file.close();
    return true;
  }
}
#endif

uint64_t file_system::GetFileModificationTime(const std::string& path) {
#if defined(OS_WIN32) || defined(OS_WINCE)
	return 0;
#else
  struct stat info;
  stat(path.c_str(), &info);
#ifndef __QNXNTO__
  return static_cast<uint64_t>(info.st_mtim.tv_nsec);
#else
  return static_cast<uint64_t>(info.st_mtime);
#endif
#endif
}

bool file_system::CopyFile(const std::string& src, const std::string& dst) {
#if defined(OS_WIN32) || defined(OS_WINCE)
  if (!FileExists(src) || FileExists(dst) || !CreateFileWindows(dst)) {
    return false;
  }
#else
  if (!FileExists(src) || FileExists(dst) || !CreateFile(dst)) {
    return false;
  }
#endif
  std::vector<uint8_t> data;
  if (!ReadBinaryFile(src, data) || !WriteBinaryFile(dst, data)) {
#if defined(OS_WIN32) || defined(OS_WINCE)
    DeleteFileWindows(dst);
#else
    DeleteFile(dst);
#endif
    return false;
  }
  return true;
}

bool file_system::MoveFile(const std::string& src, const std::string& dst) {
#if defined(OS_WIN32) || defined(OS_WINCE)
  if (!CopyFile(src, dst)) {
    return false;
  }
  if (!DeleteFileWindows(src)) {
    DeleteFileWindows(dst);
    return false;
  }
#else
  if (std::rename(src.c_str(), dst.c_str()) == 0) {
    return true;
  } else {
    // In case of src and dst on different file systems std::rename returns
    // an error (at least on QNX).
    // Seems, streams are not recommended for use, so have
    // to find another way to do this.
    std::ifstream s_src(src, std::ios::binary);
    if (!s_src.good()) {
      return false;
    }
    std::ofstream s_dst(dst, std::ios::binary);
    if (!s_dst.good()) {
      return false;
    }
    s_dst << s_src.rdbuf();
    s_dst.close();
    s_src.close();
    DeleteFile(src);
    return true;
  }
#endif
  return false;
}
