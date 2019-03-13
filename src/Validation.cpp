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

#include <string>
#include <limits>
#include "shellanything/Validation.h"
#include "shellanything/PropertyManager.h"
#include "rapidassist/strings.h"
#include "rapidassist/filesystem.h"

namespace shellanything
{
  bool hasValue(const ra::strings::StringVector & values, const std::string & token)
  {
    for(size_t i=0; i<values.size(); i++)
    {
      const std::string & v = values[i];
      if (token == v)
        return true;
    }
    return false;
  }

  void uppercase(ra::strings::StringVector & values)
  {
    for(size_t i=0; i<values.size(); i++)
    {
      std::string tmp = ra::strings::uppercase(values[i]);
      values[i] = tmp;
    }
  }





  Validation::Validation() :
    mMaxFiles(std::numeric_limits<int>::max()),
    mMaxDirectories(std::numeric_limits<int>::max())
  {
  }

  Validation::Validation(const Validation & iValidation)
  {
    (*this) = iValidation;
  }
  
  Validation::~Validation()
  {
  }

  const int & Validation::getMaxFiles() const
  {
    return mMaxFiles;
  }

  void Validation::setMaxFiles(const int & iMaxFiles)
  {
    mMaxFiles = iMaxFiles;
  }

  const int & Validation::getMaxDirectories() const
  {
    return mMaxDirectories;
  }

  void Validation::setMaxDirectories(const int & iMaxDirectories)
  {
    mMaxDirectories = iMaxDirectories;
  }

  const std::string & Validation::getProperties() const
  {
    return mProperties;
  }

  void Validation::setProperties(const std::string & iProperties)
  {
    mProperties = iProperties;
  }

  const std::string & Validation::getFileExtensions() const
  {
    return mFileExtensions;
  }

  void Validation::setFileExtensions(const std::string & iFileExtensions)
  {
    mFileExtensions = iFileExtensions;
  }

  bool Validation::isValid(const Context & iContext) const
  {
    if (iContext.filesCount > mMaxFiles)
      return false; //too many files selected

    if (iContext.directoryCount > mMaxDirectories)
      return false; //too many directories selected

    //validate properties
    PropertyManager & pmgr = PropertyManager::getInstance();
    if (!mProperties.empty())
    {
      //split
      ra::strings::StringVector properties = ra::strings::split(mProperties, ";");

      //each property specified must exists
      for(size_t i=0; i<properties.size(); i++)
      {
        const std::string & p = properties[i];
        if (!pmgr.hasProperty(p))
          return false; //missing property
      }
    }

    //validate file extentions
    if (!mFileExtensions.empty())
    {
      //split
      ra::strings::StringVector accepted_file_extensions = ra::strings::split(mFileExtensions, ";");
      uppercase(accepted_file_extensions);

      //for each file selected
      for(size_t i=0; i<iContext.elements.size(); i++) 
      {
        const std::string & element = iContext.elements[i];
        std::string current_file_extension = ra::strings::uppercase(ra::filesystem::getFileExtention(element));

        //each file extension must be part of accepted_file_extensions
        bool found = hasValue(accepted_file_extensions, current_file_extension);
        if (!found)
          return false; //current file extension is not accepted
      }
    }

    return true;
  }

  const Validation & Validation::operator =(const Validation & iValidation)
  {
    this->mMaxFiles       = iValidation.mMaxFiles      ;
    this->mMaxDirectories = iValidation.mMaxDirectories;
    this->mProperties     = iValidation.mProperties    ;
    this->mFileExtensions = iValidation.mFileExtensions;

    return (*this);
  }

} //namespace shellanything
