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

#include "PropertyManager.h"

#include "shellanything/Context.h"

#include "rapidassist/environment_utf8.h"
#include "rapidassist/filesystem_utf8.h"

namespace shellanything
{

  PropertyManager::PropertyManager()
  {
    RegisterEnvironmentVariables();
    RegisterDefaultProperties();
  }

  PropertyManager::~PropertyManager()
  {
  }

  PropertyManager & PropertyManager::GetInstance()
  {
    static PropertyManager _instance;
    return _instance;
  }

  void PropertyManager::Clear()
  {
    properties.clear();
    RegisterEnvironmentVariables();
    RegisterDefaultProperties();
  }

  void PropertyManager::ClearProperty(const std::string & name)
  {
    PropertyMap::const_iterator propertyIt = properties.find(name);
    bool found = (propertyIt != properties.end());
    if (found)
    {
      properties.erase(propertyIt);
    }
  }

  bool PropertyManager::HasProperty(const std::string & name) const
  {
    PropertyMap::const_iterator propertyIt = properties.find(name);
    bool found = (propertyIt != properties.end());
    return found;
  }

  void PropertyManager::SetProperty(const std::string & name, const std::string & value)
  {
    //overwrite previous property
    properties[name] = value;
  }

  const std::string & PropertyManager::GetProperty(const std::string & name) const
  {
    PropertyMap::const_iterator propertyIt = properties.find(name);
    bool found = (propertyIt != properties.end());
    if (found)
    {
      const std::string & value = propertyIt->second;
      return value;
    }

    static std::string EMPTY_VALUE;
    return EMPTY_VALUE;
  }

  std::string PropertyManager::Expand(const std::string & value) const
  {
    std::string output = value;

    //for each properties
    for (PropertyMap::const_iterator propertyIt = properties.begin(); propertyIt != properties.end(); propertyIt++)
    {
      const std::string & name  = propertyIt->first;
      const std::string & value = propertyIt->second;

      //generate the search token
      std::string token;
      token.append("${");
      token.append(name);
      token.append("}");

      //process with search and replace
      ra::strings::Replace(output, token, value);
    }

    return output;
  }

  void PropertyManager::RegisterEnvironmentVariables()
  {
    //Work around for https://github.com/end2endzone/RapidAssist/issues/54
    ra::environment::GetEnvironmentVariableUtf8("foo");

    //register all environment variables
    ra::strings::StringVector vars = ra::environment::GetEnvironmentVariablesUtf8();
    for(size_t i=0; i<vars.size(); i++)
    {
      const std::string & var = vars[i];

      std::string name = "env." + var;
      std::string value = ra::environment::GetEnvironmentVariableUtf8(var.c_str());
      
      //register the variable as a valid property
      SetProperty(name, value);
    }
  }

  void PropertyManager::RegisterDefaultProperties()
  {
    //define global properties
    std::string prop_path_separator         = ra::filesystem::GetPathSeparatorStr();
    std::string prop_line_separator         = ra::environment::GetLineSeparator();

    SetProperty("path.separator"       , prop_path_separator       );
    SetProperty("line.separator"       , prop_line_separator       );
    SetProperty("newline"              , prop_line_separator       );

    // Set default property for multi selection. Issue #52.
    SetProperty(Context::MULTI_SELECTION_SEPARATOR_PROPERTY_NAME, Context::DEFAULT_MULTI_SELECTION_SEPARATOR);
  }

} //namespace shellanything
