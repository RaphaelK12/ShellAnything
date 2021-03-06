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

#include "TestValidator.h"
#include "shellanything/Validator.h"
#include "shellanything/Context.h"
#include "PropertyManager.h"
#include "rapidassist/testing.h"

namespace shellanything { namespace test
{

  //--------------------------------------------------------------------------------------------------
  void TestValidator::SetUp()
  {
    PropertyManager & pmgr = PropertyManager::GetInstance();
    pmgr.Clear();
  }
  //--------------------------------------------------------------------------------------------------
  void TestValidator::TearDown()
  {
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testValidByDefault)
  {
    Context c;
    Validator v;

    //assert default
    ASSERT_TRUE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testMaxFiles)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32\\kernel32.dll");
      elements.push_back("C:\\Windows\\System32\\cmd.exe"     );
      elements.push_back("C:\\Windows\\System32\\notepad.exe" );
      elements.push_back("C:\\Windows\\System32\\services.msc");
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    Validator v;

    //assert default
    ASSERT_TRUE( v.Validate(c) );

    //assert higher value
    v.SetMaxFiles(c.GetNumFiles() + 1);
    ASSERT_TRUE( v.Validate(c) );

    //assert exact value
    v.SetMaxFiles(c.GetNumFiles());
    ASSERT_TRUE( v.Validate(c) );

    //assert lower value
    v.SetMaxFiles(c.GetNumFiles() - 1);
    ASSERT_FALSE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testMaxDirectories)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Program Files");
      elements.push_back("C:\\Users"        );
      elements.push_back("C:\\Windows"      );
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to directories
#endif

    Validator v;

    //assert default
    ASSERT_TRUE( v.Validate(c) );

    //assert higher value
    v.SetMaxDirectories(c.GetNumDirectories() + 1);
    ASSERT_TRUE( v.Validate(c) );

    //assert exact value
    v.SetMaxDirectories(c.GetNumDirectories());
    ASSERT_TRUE( v.Validate(c) );

    //assert lower value
    v.SetMaxDirectories(c.GetNumDirectories() - 1);
    ASSERT_FALSE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testProperties)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32\\kernel32.dll");
      elements.push_back("C:\\Windows\\System32\\cmd.exe"     );
      elements.push_back("C:\\Windows\\System32\\notepad.exe" );
      elements.push_back("C:\\Windows\\System32\\services.msc");
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    Validator v;

    //assert default
    ASSERT_TRUE( v.Validate(c) );

    PropertyManager & pmgr = PropertyManager::GetInstance();
    std::string property_name = ra::testing::GetTestQualifiedName();

    //assert failure when property is not defined
    v.SetProperties(property_name);
    ASSERT_FALSE( v.Validate(c) );

    //assert success when property is defined
    v.SetProperties(property_name);
    pmgr.SetProperty(property_name, "defined");
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when at least one property is not defined
    v.SetProperties(property_name + ";foo");
    ASSERT_FALSE( v.Validate(c) );

    //assert success when all properties are defined
    pmgr.SetProperty("foo", "bar");
    ASSERT_TRUE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testFileExtensions)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32\\kernel32.dll");
      elements.push_back("C:\\Windows\\System32\\cmd.exe"     );
      elements.push_back("C:\\Windows\\System32\\notepad.exe" );
      elements.push_back("C:\\Windows\\System32\\services.msc");
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    Validator v;

    //assert default
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when no file extension is matching
    v.SetFileExtensions("foo");
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when a single file extension is matching
    v.SetFileExtensions("dll");
    ASSERT_FALSE( v.Validate(c) );

    //assert success when all file extensions are matching
    v.SetFileExtensions("dll;exe;msc");
    ASSERT_TRUE( v.Validate(c) );
    v.SetFileExtensions("exe;dll;msc"); //random order
    ASSERT_TRUE( v.Validate(c) );

    //assert success when more file extensions are allowed
    v.SetFileExtensions("ini;txt;bat;doc;msc;dll;exe;xls;");
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when multiple files are selected and a single extension is missing
    v.SetFileExtensions("dll;exe"); //missing msc file extension
    ASSERT_FALSE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testFileExists)
  {
    Context c;
 
#ifdef _WIN32
    const std::string file_path = "C:\\Windows\\System32\\kernel32.dll";
    const std::string dir_path = "C:\\Program Files (x86)";
#else
    //TODO: complete with known path to files
#endif
 
    Validator v;
 
    //assert default
    ASSERT_TRUE( v.Validate(c) );
 
    //assert failure when a specified file/directory is not found
    v.SetFileExists("foo");
    ASSERT_FALSE( v.Validate(c) );
 
    //assert success if the specified file is found
    v.SetFileExists(file_path);
    ASSERT_TRUE( v.Validate(c) );
 
    //assert success if the specified directory is found
    v.SetFileExists(dir_path);
    ASSERT_TRUE( v.Validate(c) );
 
    //assert success if all elements exists
    v.SetFileExists(file_path + ";" + dir_path);
    ASSERT_TRUE( v.Validate(c) );
 
    //assert failure if the last element is not found
    v.SetFileExists(file_path + ";" + dir_path + ";foo");
    ASSERT_FALSE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testClass)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32\\kernel32.dll");
      elements.push_back("C:\\Windows\\System32\\cmd.exe"     );
      elements.push_back("C:\\Windows\\System32\\notepad.exe" );
      elements.push_back("C:\\Windows\\System32\\services.msc");
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    Validator v;

    //assert default
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when no file extension is matching
    v.SetClass(".foo");
    ASSERT_FALSE( v.Validate(c) );

    //assert success when a single file extension is matching
    v.SetClass(".dll");
    ASSERT_FALSE( v.Validate(c) );

    //assert success when all file extensions are matching
    v.SetClass(".dll;.exe;.msc");
    ASSERT_TRUE( v.Validate(c) );
    v.SetClass(".exe;.dll;.msc"); //random order
    ASSERT_TRUE( v.Validate(c) );

    //assert success when more file extensions are allowed
    v.SetClass(".ini;.txt;.bat;.doc;.msc;.dll;.exe;.xls;");
    ASSERT_TRUE( v.Validate(c) );

    //assert success when using 'file'
    v.SetClass("file");
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when using 'folder'
    v.SetClass("folder");
    ASSERT_FALSE( v.Validate(c) );

    //assert success when using 'drive'
    v.SetClass("drive");
    ASSERT_TRUE( v.Validate(c) );

    //assert success when using 'drive:fixed'
    v.SetClass("drive:fixed");
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when using 'drive:network'
    v.SetClass("drive:network");
    ASSERT_FALSE( v.Validate(c) );

    //assert 'at least one' class must match
    v.SetClass("folder;drive:network;drive:fixed"); // folder and drive:network fails but drive:fixed matches
    ASSERT_TRUE( v.Validate(c) );

    //Set only folders
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32");
      elements.push_back("C:\\Windows\\Fonts"     );
      elements.push_back("C:\\Windows\\SysWOW64" );
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    //assert failure when using 'file'
    v.SetClass("file");
    ASSERT_FALSE( v.Validate(c) );

    //assert success when using 'folder'
    v.SetClass("folder");
    ASSERT_TRUE( v.Validate(c) );

    //assert success when using 'drive'
    v.SetClass("drive");
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when using 'drive:network'
    v.SetClass("drive:network");
    ASSERT_FALSE( v.Validate(c) );

#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("\\\\localhost\\public\\foo.dat" );
      elements.push_back("\\\\localhost\\public\\bar.dat" );
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    //assert failure when using 'file'
    v.SetClass("file");
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when using 'drive'
    v.SetClass("drive");
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when using 'drive:fixed'
    v.SetClass("drive:fixed");
    ASSERT_FALSE( v.Validate(c) );

    //assert success when using 'drive:network'
    v.SetClass("drive:network");
    ASSERT_TRUE( v.Validate(c) );

  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testPattern)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32\\kernel32.dll");
      elements.push_back("C:\\Windows\\System32\\cmd.exe"     );
      elements.push_back("C:\\Windows\\System32\\notepad.exe" );
      elements.push_back("C:\\Windows\\System32\\services.msc");
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    Validator v;

    //assert default
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when no pattern is matching
    v.SetPattern("foo");
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when patterns is matching only a single file
    v.SetPattern("*cmd.exe");
    ASSERT_FALSE( v.Validate(c) );

    //assert success when patterns are matching all files
    v.SetPattern("*.dll;*.exe;*.msc");
    ASSERT_TRUE( v.Validate(c) );
    v.SetPattern("*.exe;*.dll;*.msc"); //random order
    ASSERT_TRUE( v.Validate(c) );

    //assert success when more than required patterns are provided
    v.SetPattern("*e*;*.dll;*.exe;*.msc;*a*;");
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when multiple files are selected and a single pattern is missing
    v.SetPattern("*.dll;*.exe"); //missing msc file extension
    ASSERT_FALSE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testIsInversed)
  {
    Validator v;
 
    // Invalid attribute name
    ASSERT_FALSE( v.IsInversed(NULL) );
    ASSERT_FALSE( v.IsInversed("") );

    v.SetInserve("foo");

    // Wrong attribute name
    ASSERT_FALSE( v.IsInversed("bar") );

    // Good attribute name;
    ASSERT_TRUE( v.IsInversed("foo") );

    // Longer attribute name
    ASSERT_FALSE( v.IsInversed("foobar") );

    // Shorter name
    ASSERT_FALSE( v.IsInversed("barfoo") );



    v.SetInserve("foobar");

    // Searching subsets of the inserve string
    ASSERT_FALSE( v.IsInversed("foo") );
    ASSERT_FALSE( v.IsInversed("bar") );
    ASSERT_FALSE( v.IsInversed("ooba") );



    v.SetInserve("foo;bar;baz");

    // Search first, middle, last attribute names
    ASSERT_TRUE( v.IsInversed("foo") );
    ASSERT_TRUE( v.IsInversed("bar") );
    ASSERT_TRUE( v.IsInversed("baz") );

    // Search is case sensitive
    ASSERT_FALSE( v.IsInversed("fOo") );
    ASSERT_FALSE( v.IsInversed("bAr") );
    ASSERT_FALSE( v.IsInversed("bAz") );


    // Assert the search go beyond its first match.
    // There is a substring match at bart and bars but they should then
    // be rejected. The last 'bar' should be where the match occurs.
    v.SetInserve("bart;bars;bar");
    ASSERT_TRUE( v.IsInversed("bar") );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testMaxFilesInversed)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32\\kernel32.dll");
      elements.push_back("C:\\Windows\\System32\\cmd.exe"     );
      elements.push_back("C:\\Windows\\System32\\notepad.exe" );
      elements.push_back("C:\\Windows\\System32\\services.msc");
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    Validator v;
    v.SetInserve("maxfiles"); // now the maxfiles attribute means "more than x"

    //assert default
    ASSERT_FALSE( v.Validate(c) ); // matches <visibility inverse="maxfiles" /> inversing maxfiles without specifying a maxfiles attribute...

    //assert higher value
    v.SetMaxFiles(c.GetNumFiles() + 1);
    ASSERT_FALSE( v.Validate(c) );

    //assert exact value
    v.SetMaxFiles(c.GetNumFiles());
    ASSERT_FALSE( v.Validate(c) );

    //assert lower value
    v.SetMaxFiles(c.GetNumFiles() - 1);
    ASSERT_TRUE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testMaxDirectoriesInversed)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Program Files");
      elements.push_back("C:\\Users"        );
      elements.push_back("C:\\Windows"      );
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to directories
#endif

    Validator v;
    v.SetInserve("maxfolders"); // now the maxfolders attribute means "more than x"

    //assert default
    ASSERT_FALSE( v.Validate(c) ); // matches <visibility inverse="maxfolders" /> inversing maxfolders without specifying a maxfolders attribute...

    //assert higher value
    v.SetMaxDirectories(c.GetNumDirectories() + 1);
    ASSERT_FALSE( v.Validate(c) );

    //assert exact value
    v.SetMaxDirectories(c.GetNumDirectories());
    ASSERT_FALSE( v.Validate(c) );

    //assert lower value
    v.SetMaxDirectories(c.GetNumDirectories() - 1);
    ASSERT_TRUE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testPropertiesInversed)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32\\kernel32.dll");
      elements.push_back("C:\\Windows\\System32\\cmd.exe"     );
      elements.push_back("C:\\Windows\\System32\\notepad.exe" );
      elements.push_back("C:\\Windows\\System32\\services.msc");
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    Validator v;
    v.SetInserve("properties"); // now the properties attribute means "valid when the property is not defined or empty"

    //assert default
    ASSERT_TRUE( v.Validate(c) );

    PropertyManager & pmgr = PropertyManager::GetInstance();
    std::string property_name = ra::testing::GetTestQualifiedName();

    //assert success when property is not defined
    pmgr.Clear();
    v.SetProperties(property_name);
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when property is defined
    pmgr.Clear();
    pmgr.SetProperty(property_name, "defined");
    v.SetProperties(property_name);
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when at least one property is defined
    pmgr.Clear();
    pmgr.SetProperty(property_name, "defined"); // property_name is defined
    v.SetProperties(property_name + ";foo"); // foo is not defined
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when all properties are defined
    pmgr.Clear();
    pmgr.SetProperty(property_name, "defined");
    pmgr.SetProperty("foo", "bar");
    v.SetProperties(property_name + ";foo"); // all defined
    ASSERT_FALSE( v.Validate(c) );

    //assert success when all properties are not defined
    pmgr.Clear();
    v.SetProperties(property_name + ";foo"); // all not defined
    ASSERT_TRUE( v.Validate(c) );

    // If multiple properties are specified, all properties must be empty or not defined for the validation to be successful.
    pmgr.Clear();
    v.SetProperties("foo;bar;baz");
    ASSERT_TRUE( v.Validate(c) );

    pmgr.SetProperty(property_name, "defined");
    v.SetProperties("foo;bar;baz;" + property_name);
    ASSERT_FALSE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testFileExtensionsInversed)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32\\kernel32.dll");
      elements.push_back("C:\\Windows\\System32\\cmd.exe"     );
      elements.push_back("C:\\Windows\\System32\\notepad.exe" );
      elements.push_back("C:\\Windows\\System32\\services.msc");
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    Validator v;
    v.SetInserve("fileextensions");

    //assert default
    ASSERT_TRUE( v.Validate(c) );

    //assert success when no file extension is matching
    v.SetFileExtensions("foo");
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when a single file extension is matching
    v.SetFileExtensions("dll");
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when all file extensions are defined
    v.SetFileExtensions("dll;exe;msc");
    ASSERT_FALSE( v.Validate(c) );
    v.SetFileExtensions("exe;dll;msc"); //random order
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when more file extensions are matching
    v.SetFileExtensions("ini;txt;bat;doc;msc;dll;exe;xls;");
    ASSERT_FALSE( v.Validate(c) );

    // If multiple file extensions are specified, no extension must match for the validation to be successful.
    v.SetFileExtensions("aaa;bbb;ccc;");
    ASSERT_TRUE( v.Validate(c) );
    v.SetFileExtensions("aaa;bbb;exe;ccc;");
    ASSERT_FALSE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testFileExistsInversed)
  {
    Context c;
 
#ifdef _WIN32
    const std::string file_path = "C:\\Windows\\System32\\kernel32.dll";
    const std::string dir_path = "C:\\Program Files (x86)";
#else
    //TODO: complete with known path to files
#endif
 
    Validator v;
    v.SetInserve("exists");
 
    //assert default
    ASSERT_TRUE( v.Validate(c) );
 
    //assert success when a specified file/directory is not found
    v.SetFileExists("foo");
    ASSERT_TRUE( v.Validate(c) );
 
    //assert failure if the specified file is found
    v.SetFileExists(file_path);
    ASSERT_FALSE( v.Validate(c) );
 
    //assert failure if the specified directory is found
    v.SetFileExists(dir_path);
    ASSERT_FALSE( v.Validate(c) );
 
    //assert failure if all elements exists
    v.SetFileExists(file_path + ";" + dir_path);
    ASSERT_FALSE( v.Validate(c) );
 
    //assert failure if at least one of all elements is found
    v.SetFileExists("bar;" + dir_path + ";foo");
    ASSERT_FALSE( v.Validate(c) );
    v.SetFileExists("bar;" + file_path + ";foo");
    ASSERT_FALSE( v.Validate(c) );
 
    // If multiple files are specified, all files must not exists on the system for the validation to be successful.
    v.SetFileExists("foo;bar;baz");
    ASSERT_TRUE( v.Validate(c) );
    v.SetFileExists("foo;bar;C:\\Windows\\System32\\kernel32.dll;baz");
    ASSERT_FALSE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testPatternInversed)
  {
    Context c;
#ifdef _WIN32
    {
      Context::ElementList elements;
      elements.push_back("C:\\Windows\\System32\\kernel32.dll");
      elements.push_back("C:\\Windows\\System32\\cmd.exe"     );
      elements.push_back("C:\\Windows\\System32\\notepad.exe" );
      elements.push_back("C:\\Windows\\System32\\services.msc");
      c.SetElements(elements);
    }
#else
    //TODO: complete with known path to files
#endif

    Validator v;
    v.SetInserve("pattern");

    //assert default
    ASSERT_TRUE( v.Validate(c) );

    //assert success when no pattern is matching
    v.SetPattern("foo");
    ASSERT_TRUE( v.Validate(c) );

    //assert failure when a pattern is matching a single file
    v.SetPattern("*.dll");
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when pattern is matching all files
    v.SetPattern("*.dll;*.exe;*.msc");
    ASSERT_FALSE( v.Validate(c) );
    v.SetPattern("*.exe;*.dll;*.msc"); //random order
    ASSERT_FALSE( v.Validate(c) );

    //assert failure when more than required patterns are provided
    v.SetPattern("*e*;*.dll;*.exe;*.msc;*a*;");
    ASSERT_FALSE( v.Validate(c) );

    // If multiple patterns are specified, no pattern must match for the validation to be successful.
    v.SetPattern("*.foo;*.bar;*.baz;");
    ASSERT_TRUE( v.Validate(c) );
    v.SetPattern("*.foo;*.exe;*.bar;*.baz;");
    ASSERT_FALSE( v.Validate(c) );
  }
  //--------------------------------------------------------------------------------------------------
  TEST_F(TestValidator, testInversedAll)
  {
    Validator v;

    v.SetInserve("all");
    ASSERT_TRUE( v.IsInversed("maxfiles") );
    ASSERT_TRUE( v.IsInversed("maxfolders") );
    ASSERT_TRUE( v.IsInversed("fileextensions") );
    ASSERT_TRUE( v.IsInversed("exists") );
    ASSERT_TRUE( v.IsInversed("properties") );

    v.SetInserve("foo;all");
    ASSERT_TRUE( v.IsInversed("maxfiles") );
    ASSERT_TRUE( v.IsInversed("maxfolders") );
    ASSERT_TRUE( v.IsInversed("fileextensions") );
    ASSERT_TRUE( v.IsInversed("exists") );
    ASSERT_TRUE( v.IsInversed("properties") );

    v.SetInserve("all;foo");
    ASSERT_TRUE( v.IsInversed("maxfiles") );
    ASSERT_TRUE( v.IsInversed("maxfolders") );
    ASSERT_TRUE( v.IsInversed("fileextensions") );
    ASSERT_TRUE( v.IsInversed("exists") );
    ASSERT_TRUE( v.IsInversed("properties") );

    v.SetInserve("foo;all;bar");
    ASSERT_TRUE( v.IsInversed("maxfiles") );
    ASSERT_TRUE( v.IsInversed("maxfolders") );
    ASSERT_TRUE( v.IsInversed("fileextensions") );
    ASSERT_TRUE( v.IsInversed("exists") );
    ASSERT_TRUE( v.IsInversed("properties") );

  }
  //--------------------------------------------------------------------------------------------------

} //namespace test
} //namespace shellanything
