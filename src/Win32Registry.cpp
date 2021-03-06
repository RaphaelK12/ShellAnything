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

#include "Win32Registry.h"

//#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#undef GetEnvironmentVariable
#undef DeleteFile
#undef CreateDirectory
#undef CopyFile
#undef CreateFile

#include "rapidassist/strings.h"
#include "rapidassist/filesystem.h"
#include "rapidassist/environment.h"

#include <assert.h>
#include <algorithm>

namespace Win32Registry
{
  bool IsIconEquals(const REGISTRY_ICON & a, const REGISTRY_ICON & b)
  {
    if (a.path == b.path && a.index == b.index)
    {
      return true;
    }
    return false;
  }

  DWORD ConvertType(const REGISTRY_TYPE & iValue)
  {
    DWORD type = REG_SZ;

    switch(iValue)
    {
    case REGISTRY_TYPE_STRING:
      type = REG_SZ;
      break;
    case REGISTRY_TYPE_BINARY:
      type = REG_BINARY;
      break;
    case REGISTRY_TYPE_UINT32:
      type = REG_DWORD;
      break;
    case REGISTRY_TYPE_UINT64:
      type = REG_QWORD;
      break;
    default:
      assert( false ); //should not append
    };

    return type;
  }

  REGISTRY_TYPE ConvertType(DWORD iValue)
  {
    REGISTRY_TYPE type = REGISTRY_TYPE_STRING;
    switch(iValue)
    {
    case REG_SZ:
    case REG_EXPAND_SZ:
    case REG_MULTI_SZ:
      type = REGISTRY_TYPE_STRING;
      break;
    case REG_BINARY:
      type = REGISTRY_TYPE_BINARY;
      break;
    case REG_DWORD:
      type = REGISTRY_TYPE_UINT32;
      break;
    case REG_QWORD:
      type = REGISTRY_TYPE_UINT64;
      break;
    default:
      assert( false ); //should not append
    };
    return type;
  }

  struct HKEY_T
  {
    HKEY key;
    const char* name;
  };
  static HKEY_T supportedKeys[] = {
    {HKEY_CLASSES_ROOT, "HKEY_CLASSES_ROOT"},
    {HKEY_CURRENT_USER, "HKEY_CURRENT_USER"},
    {HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE"},
    {HKEY_USERS, "HKEY_USERS"},
    {HKEY_CURRENT_CONFIG, "HKEY_CURRENT_CONFIG"},
  };
  static HKEY_T* FindKeyInPath(const char* iPath)
  {
    //detecting the key within iPath
    int numSupportedKeys = sizeof(supportedKeys)/sizeof(HKEY_T);
    HKEY_T* rootKey = NULL;
    for(int i=0; i<numSupportedKeys && rootKey == NULL; i++)
    {
      HKEY_T* current = &supportedKeys[i];
      if (_strnicmp(iPath, current->name, strlen(current->name)) == 0)
      {
        rootKey = current;
      }
    }

    return rootKey;
  }
  static const char* GetShortKeyPath(const char* iKeyPath)
  {
    HKEY_T* rootKey = FindKeyInPath(iKeyPath);
    if (rootKey)
    {
      size_t keyPathIndex = strlen(rootKey->name)+1;
      return &iKeyPath[keyPathIndex];
    }
    return NULL;
  }
  static bool SetRegistryValue(const char* iKeyPath, const char* iValueName, int iType, const void *iValue, const uint32_t & iSize)
  {
    bool result = false;

    HKEY_T* rootKey = FindKeyInPath(iKeyPath);
    if (rootKey)
    {
      const char* keyShortPath = GetShortKeyPath(iKeyPath);

      HKEY keyHandle;
      if ( !RegCreateKey(rootKey->key, keyShortPath, &keyHandle))
      {
        if ( !RegSetValueEx(keyHandle, iValueName, 0, iType, (LPBYTE)iValue, iSize))
        {
          result = true;
        }
        RegCloseKey(keyHandle);
      }
    }
    return result;
  }


  bool GetValue(const char * iKeyPath,
                const char * iValueName,
                REGISTRY_TYPE & oType,
                MemoryBuffer & oValue)
  {
    bool success = false;
    HKEY_T* rootKey = FindKeyInPath(iKeyPath);
    
    if (rootKey)
    {
      HKEY keyHandle = NULL;
      const char* keyShortPath = GetShortKeyPath(iKeyPath);

      if (RegOpenKeyEx(rootKey->key, keyShortPath, 0, KEY_QUERY_VALUE|KEY_WOW64_64KEY, &keyHandle) == ERROR_SUCCESS)
      {
        //Read value's size and type
        DWORD valueType = 0;
        DWORD valueSize = 0; //the size of the returned buffer in bytes. This size includes any terminating null character.
        RegQueryValueEx( keyHandle, iValueName, NULL, &valueType, NULL, &valueSize);

        DWORD length = valueSize - 1;

        //allocate space for value
        if (valueSize > 0 && oValue.assign(length, 0).size())
        {
          //Read the actual data of the value
          valueType = 0;
          RegQueryValueEx( keyHandle, iValueName, NULL, &valueType, (LPBYTE)oValue.c_str(), &valueSize);
          
          oType = ConvertType(valueType);
          success = (length == oValue.size());
        }

        RegCloseKey(keyHandle);
        return success;
      }
    }

    return success;
  }

  bool GetDefaultKeyValue( const char * iKeyPath, REGISTRY_TYPE & oType, MemoryBuffer & oValue)
  {
    return GetValue(iKeyPath, "", oType, oValue);
  }

  bool HasKey(const char* iKeyPath)
  {
    bool result = false;

    HKEY_T* rootKey = FindKeyInPath(iKeyPath);
    if (rootKey)
    {
      const char* keyShortPath = GetShortKeyPath(iKeyPath);

      HKEY keyHandle;
      if ( !RegOpenKey(rootKey->key, keyShortPath, &keyHandle))
      {
        result = true;
        RegCloseKey(keyHandle);
      }
    }
    return result;
  }

  bool CreateKey(const char* iKeyPath)
  {
    bool result = false;

    HKEY_T* rootKey = FindKeyInPath(iKeyPath);
    if (rootKey)
    {
      const char* keyShortPath = GetShortKeyPath(iKeyPath);

      HKEY keyHandle = NULL;
      //printf("RegCreateKeyA(%s, %s, 0x%08x)\n", rootKey->name, keyShortPath, keyHandle);
      if ( !RegCreateKey(rootKey->key, keyShortPath, &keyHandle))
      {
        result = true;
      }
      RegCloseKey(keyHandle);
    }

    return result;
  }

  bool CreateKey(const char* iKeyPath, const char* iDefaultValue)
  {
    bool key_result = CreateKey(iKeyPath);
    if (!key_result)
      return false;

    //set default value
    if (iDefaultValue)
    {
      if (!Win32Registry::SetValue(iKeyPath, "", iDefaultValue))
        return false;
    }

    return true;
  }

  bool DeleteKey(const char* iKeyPath)
  {
    if (!HasKey(iKeyPath))
      return true; //return a success if the key cannot be found

    bool result = false;

    HKEY_T* rootKey = FindKeyInPath(iKeyPath);
    if (rootKey)
    {
      const char* keyShortPath = GetShortKeyPath(iKeyPath);

      LSTATUS status = RegDeleteTreeA(rootKey->key, keyShortPath);
      if (status == ERROR_SUCCESS || status == ERROR_PATH_NOT_FOUND)
      {
        result = true;
      }
    }

    return result;
  }

  bool SetValue(const char* iKeyPath, const char* iValueName, const uint8_t* iBuffer, const uint32_t & iBufferSize)
  {
    return SetRegistryValue(iKeyPath, iValueName, REG_BINARY, iBuffer, iBufferSize);
  }

  bool SetValue(const char* iKeyPath, const char* iValueName, const uint32_t & iValue)
  {
    return SetRegistryValue(iKeyPath, iValueName, REG_DWORD, &iValue, sizeof(iValue));
  }

  bool SetValue(const char* iKeyPath, const char* iValueName, const char *iValue)
  {
    uint32_t size = (uint32_t)strlen(iValue) + 1;
    return SetRegistryValue(iKeyPath, iValueName, REG_SZ, iValue, size);
  }

  bool DeleteValue(const char* iKeyPath, const char* iValueName)
  {
    if (!HasKey(iKeyPath))
      return true; //return a success if the key cannot be found

    bool result = false;

    HKEY_T* rootKey = FindKeyInPath(iKeyPath);
    if (rootKey)
    {
      const char* keyShortPath = GetShortKeyPath(iKeyPath);

      HKEY keyHandle = NULL;
      if ( !RegOpenKeyEx(rootKey->key, keyShortPath, 0, KEY_SET_VALUE|KEY_WOW64_64KEY, &keyHandle))
      {
        LSTATUS status = RegDeleteValueA(keyHandle, iValueName);
        if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND)
        {
          result = true;
        }
      }
      RegCloseKey(keyHandle);
    }

    return result;
  }

  bool GetAssociatedProgram(const char* iFileExtention, std::string & oCmdLine)
  {
	  const DWORD SIZE = 2048;

    HKEY hKey = NULL;
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, iFileExtention, 0, KEY_READ|KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
    {
      //Key does not exist
      oCmdLine = "";
      return false;
    }

    //Get key's unnamed value
    char defaultKeyValueName[SIZE];
    DWORD defaultKeyValueSize = SIZE;
    RegQueryValueEx(hKey, "", 0, 0, (LPBYTE)defaultKeyValueName, &defaultKeyValueSize);
    RegCloseKey(hKey);

    //Open Command key of File Association key's Open subkey
    std::string sPath = defaultKeyValueName;
    sPath.append("\\shell\\open\\command");

    HKEY hProgKey = NULL;
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, sPath.c_str(), 0, KEY_READ|KEY_WOW64_64KEY, &hProgKey) == ERROR_SUCCESS)
    {
      DWORD progNameSize = 0;
      //Determine data type and buffer size of key
      RegQueryValueEx(hProgKey, "", 0, 0, NULL, &progNameSize);

      //Retrieve file association
      char programName[SIZE];
      progNameSize = SIZE;
      DWORD lType = 0;
      RegQueryValueEx(hProgKey, "", 0, &lType, (LPBYTE)programName, &progNameSize);

      RegCloseKey(hProgKey);
    
      oCmdLine = programName;

      return true;
    }

    oCmdLine = "";
    return false;
	}

  bool SetAssociatedProgram(const char* iExtention, const char* iFileType, const char* iCmdLine)
  {
    //save current setting;
    //REGISTRY_HANDLER_ROOT current = m_CurrentRoot;
    //CRegistryHandler c;

    //setRootFolder(CLASSES_ROOT);

    //char sExt[100];
    //char sExtAutoFile[100];
    //sprintf(sExt,".%s",ext);
    //sprintf(sExtAutoFile,"%s_auto_file",ext);

    bool success = true;

    //Building the extention (removing the '.' character in iExtention in case)
    std::string extention = iExtention;
    if (iExtention[0] == '.')
    {
      extention = &iExtention[1];
    }

    //Build the extention file type (most of the time the format used is extentionfile. ie: txtfile)
    std::string extentionFileType = extention.c_str();
    extentionFileType.append("file");

    //Create the key for the extention
    std::string extentionKey = "HKEY_CLASSES_ROOT\\.";
    extentionKey.append(extention.c_str());
    success = CreateKey(extentionKey.c_str());
    if (!success)
    {
      //printf("1 Unable to create key: %s\n", extentionKey.c_str());
      return false;
    }

    //printf("step #1\n");

    //Create the key for the file type
    std::string typeKey = "HKEY_CLASSES_ROOT\\";
    typeKey.append(extentionFileType.c_str());
    success = CreateKey(typeKey.c_str());
    if (!success)
    {
      //printf("2 Unable to create key: %s\n", typeKey.c_str());
      return false;
    }

    //printf("step #2\n");

    //Create the extention key's default value. (extention file type)
    success = SetValue(extentionKey.c_str(), "", extentionFileType.c_str());
    if (!success)
    {
      //printf("3 Unable to set value. key=%s, name=%s\n", extentionKey.c_str(), extentionFileType.c_str());
      return false;
    }

    //printf("step #3\n");

    //Create the filetype key's default value (friendly name) 
    SetValue(extentionFileType.c_str(), "", iFileType);

    //printf("step #4\n");

    //Create the shell, open and command sub keys
    const char* subKeys[] = {"shell", "open", "command"};
    int numSubKeys = sizeof(subKeys)/sizeof(const char*);
    std::string subKey = typeKey.c_str();
    for(int i=0; i<numSubKeys; i++)
    {
      const char* currentSubKey = subKeys[i];

      subKey.append("\\");
      subKey.append(currentSubKey);

      //create this key
      //printf("debug: trying to create key: %s\n", subKey.c_str());
      success = CreateKey(subKey.c_str());
      if (!success)
      {
        //printf("4 Unable to create key: %s\n", subKey.c_str());
        return false;
      }
    }

    //printf("step #5\n");

    //Define the default value for the command's key
    success = SetValue(subKey.c_str(), "", iCmdLine);
    if (!success)
    {
      //printf("5 Unable to set value. key=%s, name=%s\n", subKey.c_str(), iCmdLine);
      return false;
    }

    //printf("step #6\n");

    //char sShell[100];
    //sprintf(sShell,"%s_auto_file\\shell",ext);
    //createFolder(sShell);
    //setKeyValue(sShell,"","open");

    //char sOpen[100];
    //sprintf(sOpen,"%s_auto_file\\shell\\open",ext);
    //createFolder(sOpen);

    //char sCommand[100];
    //sprintf(sCommand,"%s_auto_file\\shell\\open\\command",ext);
    //setKeyValue(sCommand,"",execString);

    ////Restore
    //m_CurrentRoot = current;

    return true;
  }

  bool RegisterCommandForFile(const char * iName, const char * iCommand)
  {
    bool success = false;
    
    std::string key;

    key = ra::strings::Format("HKEY_CLASSES_ROOT\\*\\shell\\%s", iName); 
    success = CreateKey(key.c_str());
    if (!success)
      return false;

    key = ra::strings::Format("HKEY_CLASSES_ROOT\\*\\shell\\%s\\command", iName); 
    success = CreateKey(key.c_str());
    if (!success)
      return false;

    success = SetValue(key.c_str(), "", iCommand);
    if (!success)
      return false;

    return true;
  }

  bool RegisterCommandForFolder(const char * iName, const char * iCommand)
  {
    bool success = false;
    
    std::string key;

    key = ra::strings::Format("HKEY_CLASSES_ROOT\\Folder\\shell\\%s", iName); 
    success = CreateKey(key.c_str());
    if (!success)
      return false;

    key = ra::strings::Format("HKEY_CLASSES_ROOT\\Folder\\shell\\%s\\command", iName); 
    success = CreateKey(key.c_str());
    if (!success)
      return false;

    success = SetValue(key.c_str(), "", iCommand);
    if (!success)
      return false;

    return true;
  }

  static REGISTRY_ICON GetFileTypeDefaultIcon(const char * iBaseKey)
  {
    //Extract default icon. ie: C:\Windows\Installer\{AC76BA86-7AD7-1036-7B44-A90000000001}\PDFFile_8.ico,0
    std::string default_icon;
    {
      std::string key = ra::strings::Format("%s\\DefaultIcon", iBaseKey); 
      MemoryBuffer buffer;
      REGISTRY_TYPE type = REGISTRY_TYPE_STRING;
      const char * debugValue = key.c_str();
      bool success = GetDefaultKeyValue(key.c_str(), type, buffer);
      if (!success)
        return NULL_ICON;
      if (type != REGISTRY_TYPE_STRING)
        return NULL_ICON;
      default_icon = buffer.c_str();
    }
    
    //check format of default icon. File only or file with index
    std::string file_path;
    int index = 0;
    {
      ra::strings::StringVector parts;
      ra::strings::Split( parts, default_icon, ",");

      if (parts.size() == 1)
      {
        //assumed a file only
        file_path = parts[0];
      }
      else if (parts.size() == 2)
      {
        //assumed a file and index
        file_path = parts[0];
        std::string indexStr = parts[1];
        const char * debugValue = indexStr.c_str();
        ra::strings::Parse(indexStr, index);
      }
      else
        return NULL_ICON; //unknown format
    }
    
    //Does file_path contains double quotes ?
    if (file_path.find("\"") != std::string::npos)
    {
      ra::strings::Replace(file_path, "\"", "");
    }

    //Does file exists ?
    {
      const char * debug_value = file_path.c_str();
      if (file_path.size() > 0 && ra::filesystem::FileExists(file_path.c_str()))
      {
        REGISTRY_ICON icon;
        icon.path = file_path;
        icon.index = index;
        return icon;
      }
    }

    //File does not exists. Maybe path contains "expandables" values
    file_path = ra::environment::Expand(file_path.c_str());
    {
      const char * debug_value = file_path.c_str();
      if (file_path.size() > 0 && ra::filesystem::FileExists(file_path.c_str()))
      {
        REGISTRY_ICON icon;
        icon.path = file_path;
        icon.index = index;
        return icon;
      }
    }

    //File does not exists. Maybe path is only a filename and must be found using %PATH%.
    std::string path_env = ra::environment::GetEnvironmentVariable("PATH");
    ra::strings::StringVector paths;
    ra::strings::Split(paths, path_env, ";");
    for(size_t i=0; i<paths.size(); i++)
    {
      //build temp file path
      std::string test_path = paths[i];
      test_path = ra::environment::Expand(test_path.c_str());
      ra::filesystem::NormalizePath(test_path);
      test_path += ra::filesystem::GetPathSeparatorStr();
      test_path += file_path;

      //test to find the file
      {
        const char * debug_value = test_path.c_str();
        if (test_path.size() > 0 && ra::filesystem::FileExists(test_path.c_str()))
        {
          REGISTRY_ICON icon;
          icon.path = test_path;
          icon.index = index;
          return icon;
        }
      }
    }

    //Unable to extract icon
    return NULL_ICON;
  }

  bool IsValid(const REGISTRY_ICON & icon)
  {
    if (icon.path.empty())
      return false;
    if (icon.index == INVALID_ICON_INDEX) //See issue #17
      return false;
    if (IsIconEquals(icon,NULL_ICON))
      return false;
    return true;
  }

  REGISTRY_ICON GetFileTypeIcon(const char * iFileExtention)
  {
    if (iFileExtention == NULL)
      return NULL_ICON;

    std::string extention = iFileExtention;
    if (extention[0] != '.')
    {
      extention.insert(0, 1, '.');
    }
    extention = ra::strings::Lowercase(extention); //file extensions are in lowercase in the registry.

    //Process known file extensions that can't be solved by searching the registry.
    //image reference: https://diymediahome.org/windows-icons-reference-list-with-details-locations-images/
    if (extention == "exe")
    {
      REGISTRY_ICON icon;
      icon.path = ra::filesystem::FindFileFromPaths("shell32.dll");
      icon.index = 2;
      return icon;
    }
    else if ( extention == "dll" ||
              extention == "ocx" ||
              extention == "sys" )
    {
      REGISTRY_ICON icon;
      icon.path = ra::filesystem::FindFileFromPaths("shell32.dll");
      icon.index = 72;
      return icon;
    }

    //Extract document short name. ie: AcroExch.Document
    std::string document_short_name;
    {
      std::string key = ra::strings::Format("HKEY_CLASSES_ROOT\\%s", extention.c_str()); 
      MemoryBuffer buffer;
      REGISTRY_TYPE type = REGISTRY_TYPE_STRING;
      bool success = GetDefaultKeyValue(key.c_str(), type, buffer);
      if (!success)
        return NULL_ICON;
      if (type != REGISTRY_TYPE_STRING)
        return NULL_ICON;
      if (buffer.empty())
        return NULL_ICON;
      document_short_name = buffer.c_str();
    }

    //Check DefautIcon with document_short_name. ie: HKEY_CLASSES_ROOT\FirefoxHTML\DefaultIcon
    {
      std::string basekey = ra::strings::Format("HKEY_CLASSES_ROOT\\%s", document_short_name.c_str()); 
      REGISTRY_ICON icon = GetFileTypeDefaultIcon(basekey.c_str());
      if (IsValid(icon))
        return icon;
    }

    //Extract document long name. ie: AcroExch.Document.7
    std::string document_current_version_name;
    {
      std::string key = ra::strings::Format("HKEY_CLASSES_ROOT\\%s\\CurVer", document_short_name.c_str()); 
      MemoryBuffer buffer;
      REGISTRY_TYPE type = REGISTRY_TYPE_STRING;
      bool success = GetDefaultKeyValue(key.c_str(), type, buffer);
      if (success)
        document_current_version_name = buffer.c_str();
    }

    //Does file type have a current version ?
    if (!document_current_version_name.empty())
    {
      //Check DefautIcon with current version name. ie: HKEY_CLASSES_ROOT\AcroExch.Document.7\DefaultIcon
      std::string basekey = ra::strings::Format("HKEY_CLASSES_ROOT\\%s", document_current_version_name.c_str()); 
      REGISTRY_ICON icon = GetFileTypeDefaultIcon(basekey.c_str());
      if (IsValid(icon))
        return icon;
    }
    
    //Does the file type have an icon handler? ie: 
    std::string document_icon_handler_guid;
    {
      std::string key = ra::strings::Format("HKEY_CLASSES_ROOT\\%s\\ShellEx\\IconHandler", document_short_name.c_str()); 
      MemoryBuffer buffer;
      REGISTRY_TYPE type = REGISTRY_TYPE_STRING;
      bool success = GetDefaultKeyValue(key.c_str(), type, buffer);
      if (success)
        document_icon_handler_guid = buffer.c_str();
    }

    if (!document_icon_handler_guid.empty())
    {
      //Keep looking with IconHandler
                                                
      //check default icon of icon handler. ie: HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{42042206-2D85-11D3-8CFF-005004838597}\Old Icon\htmlfile
      {
        std::string basekey = ra::strings::Format("HKEY_CLASSES_ROOT\\SOFTWARE\\Classes\\CLSID\\%s\\Old Icon\\%s", document_icon_handler_guid.c_str(), document_short_name.c_str()); 
        REGISTRY_ICON icon = GetFileTypeDefaultIcon(basekey.c_str());
        if (IsValid(icon))
          return icon;
      }

      //check default icon of icon handler. ie: KEY_CLASSES_ROOT\Wow6432Node\CLSID\{42042206-2D85-11D3-8CFF-005004838597}\Old Icon\htmlfile\DefaultIcon
      {
        std::string basekey = ra::strings::Format("HKEY_CLASSES_ROOT\\Wow6432Node\\CLSID\\%s\\Old Icon\\%s", document_icon_handler_guid.c_str(), document_short_name.c_str()); 
        REGISTRY_ICON icon = GetFileTypeDefaultIcon(basekey.c_str());
        if (IsValid(icon))
          return icon;
      }
    }

    //Try the associated program
    std::string program;
    if (GetAssociatedProgram(extention.c_str(), program))
    {
      if (program.size() > 0)
      {
        //check if file exists
        const char * debugValue = program.c_str();
        if (ra::filesystem::FileExists(program.c_str()))
        {
          REGISTRY_ICON icon;
          icon.path = program;
          icon.index = 0;
          return icon;
        }

        //File does not exists. Maybe arguments are included in the path. Try cleaning it
        ra::strings::Replace(program, "\"", "");
        size_t pos = 0;
        if (program.find(".exe", pos))
        {
          program = program.substr(pos + 4);

          //matbe there is expandable in the path too
          program = ra::environment::Expand(program.c_str());

          //try again
          const char * debugValue = program.c_str();
          if (ra::filesystem::FileExists(program.c_str()))
          {
            REGISTRY_ICON icon;
            icon.path = program;
            icon.index = 0;
            return icon;
          }
        }
      }
    }

    //Unable to extract icon
    return NULL_ICON;
  }

  std::string ToString(const RGS_ENTRY & entry)
  {
    std::string output;

    if (entry.isKey)
      output.append("  KEY ");
    else
      output.append("VALUE ");

    if (entry.isNoRemove)
      output.append("NoRemove ");
    else
      output.append("         ");

    if (entry.isForceRemove)
      output.append("ForceRemove ");
    else
      output.append("            ");

    static const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "path='%s', value='%s'",
        entry.path.c_str(),
        entry.value.c_str() );
    output.append(buffer);

    return output;
  }

  bool ParseRgsFlag(std::string & line, const std::string & flag)
  {
    if (line.find(flag) != std::string::npos)
    {
      //flag found.
      //cleanup
      ra::strings::Replace(line, flag, "");
      
      return true;
    }

    //flag not found
    return false;
  }

  bool ExtractNameValuePair(const std::string & line, std::string & name, std::string & value)
  {
    const std::string pattern = " = s ";
    size_t pattern_pos = line.find(pattern);
    if (pattern_pos != std::string::npos)
    {
      size_t name_length = pattern_pos;
      size_t value_length = line.size() - pattern_pos + pattern.size();
      name = line.substr(0, name_length);
      value = line.substr(pattern_pos + pattern.size(), value_length);
      return true;
    }
    return false;
  }

  const std::string & GetLastParentKey(const ra::strings::StringVector & keys)
  {
    static const std::string EMPTY_KEY;
    if (keys.size() == 0)
      return EMPTY_KEY;
    
    const std::string & last = keys[keys.size()-1];
    return last;
  }

  bool IsSubDirectory(const std::string & base_path, const std::string & test_path)
  {
    if (test_path.size() < base_path.size())
      return false;
    if (test_path.substr(0, base_path.size()) == base_path)
      return true;
    return false;
  }

  void ValidateRgsIntegrity(RGS_ENTRY_LIST & entries)
  {
    for(size_t i=0; i<entries.size(); i++)
    {
      RGS_ENTRY & entry = entries[i];
      if (entry.isForceRemove)
      {
        //make sure all subdirectories are also set to ForceRemove
        //for each other entries
        const std::string & base_path = entry.path;
        for(size_t j=i+1; j<entries.size(); j++)
        {
          RGS_ENTRY & sub_entry = entries[j];

          const std::string & sub_entry_path = sub_entry.path;
          if (IsSubDirectory(base_path, sub_entry_path))
          {
            //this is a direct child
            sub_entry.isForceRemove = true;
          }
        }
      }
    }
  }

  bool ParseRgsRegistry(const std::string & rgs, const std::string & module_path, RGS_ENTRY_LIST & entries)
  {
    entries.clear();

    ra::strings::StringVector parent_keys;
    std::string previous_key_name;

    //split rgs code in lines
    std::string tmp = rgs;
    ra::strings::Replace(tmp, ra::environment::GetLineSeparator(), "\n");
    ra::strings::Replace(tmp, "\r\n", "\n");
    ra::strings::StringVector lines = ra::strings::Split(tmp, "\n");

    //process each lines
    for(size_t i=0; i<lines.size(); i++)
    {
      std::string line = lines[i];

      //trim
      line = ra::strings::TrimLeft(line, '\t');

      //extract flags
      bool isNoRemove     = ParseRgsFlag(line, "NoRemove ");
      bool isForceRemove  = ParseRgsFlag(line, "ForceRemove ");
      bool isValue        = ParseRgsFlag(line, "val ");
      bool isKey          = !isValue;

      if (isValue)
      {
        std::string name;
        std::string value;
        if (!ExtractNameValuePair(line, name, value))
          return false; //failed parsing rgs code

        //trim
        value = ra::strings::Trim(value, '\'');

        std::string parent_key = GetLastParentKey(parent_keys);

        //build a new entry
        RGS_ENTRY entry;
        entry.isKey = isKey;
        entry.isNoRemove = isNoRemove;
        entry.isForceRemove = isForceRemove;
        entry.path = parent_key + "\\" + name;
        entry.value = value;

        entries.push_back(entry);
      }

      //process root key acronyms for Win32Registry functions
      else if (line == "HKCR")
        previous_key_name = "HKEY_CLASSES_ROOT";
      else if (line == "HKCU")
        previous_key_name = "HKEY_CURRENT_USER";
      else if (line == "HKLM")
        previous_key_name = "HKEY_LOCAL_MACHINE";
      else if (line == "HKCC")
        previous_key_name = "HKEY_CURRENT_CONFIG";
      else if (line == "{")
      {
        //the previous key is a parent key
        parent_keys.push_back(previous_key_name);
      }
      else if (line == "}")
      {
        //go up 1 parent key
        if (parent_keys.size() > 0)
          parent_keys.pop_back();
      }
      else if (line.empty())
      {
        //skip
      }
      else if (isKey)
      {
        //is there a default value for the key?
        std::string name;
        std::string default_value;
        if (!ExtractNameValuePair(line, name, default_value))
        {
          //there is not a default value
          name = line; //the key is the line itself
        }

        //trim
        name          = ra::strings::Trim(name,           '\'');
        default_value = ra::strings::Trim(default_value,  '\'');

        //build key full path
        std::string parent_key = GetLastParentKey(parent_keys);
        std::string key_path;
        if (!parent_key.empty())
          key_path.append(parent_key + "\\");
        key_path.append(name);

        //process with search and replace
        {
          const std::string module_pattern = "%MODULE%";
          if (default_value.find(module_pattern) != std::string::npos)
          {
            ra::strings::Replace(default_value, module_pattern, module_path);
          }
        }

        //remember the key path if we run into a { character on next line
        previous_key_name = key_path;

        //build a new entry
        RGS_ENTRY entry;
        entry.isKey = isKey;
        entry.isNoRemove = isNoRemove;
        entry.isForceRemove = isForceRemove;
        entry.path = key_path;
        entry.value = default_value;

        entries.push_back(entry);
      }
      else
      {
        //unknown data
        return false;
      }
    }

    ValidateRgsIntegrity(entries);
    return true;
  }

  struct less_than_key
  {
    inline bool operator() (const RGS_ENTRY& struct1, const RGS_ENTRY& struct2)
    {
      if (struct1.path == struct2.path)
        return struct1.value < struct2.value;
      return (struct1.path < struct2.path);
    }
  };

  bool CreateRegistry(const RGS_ENTRY_LIST & tmp)
  {
    RGS_ENTRY_LIST entries = tmp;
    ValidateRgsIntegrity(entries);

    //sort in acsending order to make sure that a parent entry is created before its children
    std::sort(entries.begin(), entries.end(), less_than_key());

    for(size_t i=0; i<entries.size(); i++)
    {
      const Win32Registry::RGS_ENTRY & entry = entries[i];
      if (entry.isKey)
      {
        //std::string debug_message;
        //debug_message += "key=" + entry.path + "\n";
        //debug_message += "default=" + entry.value + "\n";
        //MessageBox(NULL, debug_message.c_str(), "CREATE KEY", MB_OK);

        //create a new key
        if (!CreateKey(entry.path.c_str()))
          return false;

        //set a default value if applicable
        if (!entry.value.empty())
        {
          if (!SetValue(entry.path.c_str(), "", entry.value.c_str()))
            return false;
        }
      }
      else
      {
        //create a new string value
        std::string parent_path;
        std::string filename;
        ra::filesystem::SplitPath(entry.path, parent_path, filename);

        //std::string debug_message;
        //debug_message += "parent=" + parent_path + "\n";
        //debug_message += "filename=" + filename + "\n";
        //debug_message += "value=" + entry.value + "\n";
        //MessageBox(NULL, debug_message.c_str(), "CREATE VALUE", MB_OK);

        if (!SetValue(parent_path.c_str(), filename.c_str(), entry.value.c_str()))
          return false;
      }
    }

    return true;
  }

  bool DeleteRegistry(const RGS_ENTRY_LIST & tmp)
  {
    RGS_ENTRY_LIST entries = tmp;
    ValidateRgsIntegrity(entries);

    //sort in descending order to make sure that a child entry is deleted before his parent
    std::sort(entries.rbegin(), entries.rend(), less_than_key());

    for(size_t i=0; i<entries.size(); i++)
    {
      const Win32Registry::RGS_ENTRY & entry = entries[i];
      if (entry.isNoRemove)
        continue;

      if (entry.isForceRemove)
      {
        if (entry.isKey)
        {
          //MessageBox(NULL, entry.path.c_str(), "DELETE KEY", MB_OK);

          //delete the key
          if (!DeleteKey(entry.path.c_str()))
            return false;
        }
        else
        {
          //delete the string value
          std::string parent_path;
          std::string filename;
          ra::filesystem::SplitPath(entry.path, parent_path, filename);

          //std::string debug_message;
          //debug_message += "parent=" + parent_path + "\n";
          //debug_message += "filename=" + filename + "\n";
          //MessageBox(NULL, debug_message.c_str(), "DELETE VALUE", MB_OK);

          if (!DeleteValue(parent_path.c_str(), filename.c_str()))
            return false;
        }
      }
    }

    return true;
  }

  REGISTRY_ICON GetUnknownFileTypeIcon()
  {
    //C:\Windows\System32\imageres.dll
    std::string imageres_path = ra::filesystem::FindFileFromPaths("imageres.dll");

    REGISTRY_ICON icon;
    icon.path = imageres_path;
    icon.index = 2;
    return icon;
  }

} //namespace Win32Registry
