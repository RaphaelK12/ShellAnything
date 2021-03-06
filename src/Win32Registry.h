/**********************************************************************************
 * MIT License
 * 
 * Copyright (c) 2018 Antoine Beauchamp
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *********************************************************************************/

#ifndef WIN32_REGISTRY_H
#define WIN32_REGISTRY_H

#include <stdint.h>
#include <vector>
#include <string>

namespace Win32Registry
{
  enum REGISTRY_TYPE
  {
    REGISTRY_TYPE_STRING,
    REGISTRY_TYPE_BINARY,
    REGISTRY_TYPE_UINT32,
    REGISTRY_TYPE_UINT64
  };
  
  struct REGISTRY_ICON
  {
    std::string path;
    int index;
  };
  
  struct RGS_ENTRY
  {
    bool isKey; //true if the entry is a key, false if the entry is a value
    bool isNoRemove;
    bool isForceRemove;
    std::string path;
    std::string value;
  };

  typedef std::vector<RGS_ENTRY> RGS_ENTRY_LIST;
  typedef std::string MemoryBuffer;

  static const int & INVALID_ICON_INDEX = -1;
  static const REGISTRY_ICON NULL_ICON = { std::string(), INVALID_ICON_INDEX };
  
  std::string ToString(const RGS_ENTRY & entry);
  bool ParseRgsRegistry(const std::string & rgs, const std::string & module_path, RGS_ENTRY_LIST & entries);
  bool CreateRegistry(const RGS_ENTRY_LIST & entries);
  bool DeleteRegistry(const RGS_ENTRY_LIST & entries);

  bool GetValue( const char * iKeyPath,
                 const char * iValueName,
                 REGISTRY_TYPE & oType,
                 MemoryBuffer & oValue);
  bool GetDefaultKeyValue( const char * iKeyPath, REGISTRY_TYPE & oType, MemoryBuffer & oValue);
  
  bool HasKey(const char* iKeyPath);
  bool CreateKey(const char* iKeyPath);
  bool CreateKey(const char* iKeyPath, const char* iDefaultValue);
  bool DeleteKey(const char* iKeyPath);
  bool SetValue(const char* iKeyPath, const char* iValueName, const uint8_t* iBuffer, const uint32_t & iBufferSize);
  bool SetValue(const char* iKeyPath, const char* iValueName, const uint32_t & iValue);
  bool SetValue(const char* iKeyPath, const char* iValueName, const char *iValue);
  bool DeleteValue(const char* iKeyPath, const char* iValueName);

  bool GetAssociatedProgram(const char* iFileExtention, std::string & oCmdLine);
  bool SetAssociatedProgram(const char* iExtention, const char* iFileType, const char* iCmdLine);

  bool RegisterCommandForFile(const char * iName, const char * iCommand);
  bool RegisterCommandForFolder(const char * iName, const char * iCommand);

  bool IsValid(const REGISTRY_ICON & icon);
  REGISTRY_ICON GetFileTypeIcon(const char * iFileExtention);
  REGISTRY_ICON GetUnknownFileTypeIcon();

} //namespace Win32Registry

#endif //WIN32_REGISTRY_H
