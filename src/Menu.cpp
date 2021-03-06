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

#include "shellanything/Menu.h"
#include "Unicode.h"

namespace shellanything
{
  const uint32_t Menu::INVALID_COMMAND_ID = 0;
  const int Menu::DEFAULT_NAME_MAX_LENGTH = 250;

  Menu::Menu() : Node("Menu"),
    mNameMaxLength(DEFAULT_NAME_MAX_LENGTH),
    mSeparator(false),
    mCommandId(INVALID_COMMAND_ID),
    mVisible(true),
    mEnabled(true)
  {
  }

  Menu::~Menu()
  {
    for(size_t i=0; i<mActions.size(); i++)
    {
      Action * action = mActions[i];
      delete action;
    }
    mActions.clear();
  }

  bool Menu::IsSeparator() const
  {
    return mSeparator;
  }

  void Menu::SetSeparator(bool iSeparator)
  {
    mSeparator = iSeparator;
  }

  bool Menu::IsParentMenu() const
  {
    Menu::MenuPtrList sub_menus = FilterNodes<Menu*>(this->FindChildren("Menu"));
    bool parent_menu = (sub_menus.size() != 0);
    return parent_menu;
  }

  const std::string & Menu::GetName() const
  {
    return mName;
  }

  void Menu::SetName(const std::string & iName)
  {
    mName = iName;
  }

  const int & Menu::GetNameMaxLength() const
  {
    return mNameMaxLength;
  }

  void Menu::SetNameMaxLength(const int & iNameMaxLength)
  {
    mNameMaxLength = iNameMaxLength;

    // Limit out of range values
    if (mNameMaxLength < 1)
      mNameMaxLength = 1;
    if (mNameMaxLength > DEFAULT_NAME_MAX_LENGTH)
      mNameMaxLength = DEFAULT_NAME_MAX_LENGTH;
  }

  void Menu::TruncateName(std::string & str)
  {
    // Issue #55: Menu name maximum length limit and escape string
  
    // Test if we need to truncate the input string.
    if (mNameMaxLength <= 0)
      return; // Nothing to do.
    size_t num_cp = GetLengthUtf8(str.c_str());
    if (num_cp <= mNameMaxLength)
      return; // Nothing to do.

    std::string truncated_str = SubstringUtf8(str.c_str(), 0, mNameMaxLength);

    // Add a trailing "..." indicating that we reached the maximum length for a menu
    if (num_cp > DEFAULT_NAME_MAX_LENGTH)
      truncated_str += "..."; // Indicate that string is much longer than maximum allowed by ShellAnything.

    str = truncated_str;
  }

  const std::string & Menu::GetDescription() const
  {
    return mDescription;
  }

  void Menu::SetDescription(const std::string & iDescription)
  {
    mDescription = iDescription;
  }

  const Icon & Menu::GetIcon() const
  {
    return mIcon;
  }

  void Menu::SetIcon(const Icon & icon)
  {
    mIcon = icon;
  }

  void Menu::Update(const Context & c)
  {
    //update current menu
    bool visible = mVisibility.Validate(c);
    bool enabled = mValidity.Validate(c);
    SetVisible(visible);
    SetEnabled(enabled);

    //update children
    bool all_invisible_children = true;

    //for each child
    Menu::MenuPtrList children = GetSubMenus();
    for(size_t i=0; i<children.size(); i++)
    {
      Menu * child = children[i];
      child->Update(c);

      //refresh the flag
      all_invisible_children = all_invisible_children && !child->IsVisible();
    }

    //Issue #4 - Parent menu with no children.
    //if all the direct children of this menu are invisible
    if (IsParentMenu() && visible && all_invisible_children)
    {
      //force this node as invisible.
      SetVisible(false);
    }
  }

  Menu * Menu::FindMenuByCommandId(const uint32_t & iCommandId)
  {
    if (mCommandId == iCommandId)
      return this;
 
    //for each child
    Menu::MenuPtrList children = GetSubMenus();
    for(size_t i=0; i<children.size(); i++)
    {
      Menu * child = children[i];
      Menu * match = child->FindMenuByCommandId(iCommandId);
      if (match)
        return match;
    }
 
    return NULL;
  }
 
  uint32_t Menu::AssignCommandIds(const uint32_t & iFirstCommandId)
  {
    uint32_t nextCommandId = iFirstCommandId;

    //Issue #5 - ConfigManager::AssignCommandIds() should skip invisible menus
    if (!mVisible || iFirstCommandId == INVALID_COMMAND_ID)
    {
      this->SetCommandId(INVALID_COMMAND_ID); //invalidate this menu's command id
    }
    else
    {
      //assign a command id to this menu
      this->SetCommandId(nextCommandId);
      nextCommandId++;
    }

    //for each child
    Menu::MenuPtrList children = GetSubMenus();
    for(size_t i=0; i<children.size(); i++)
    {
      Menu * child = children[i];

      if (mCommandId == INVALID_COMMAND_ID)
        child->AssignCommandIds(INVALID_COMMAND_ID); //also assign invalid ids to sub menus
      else
        nextCommandId = child->AssignCommandIds(nextCommandId); //assign the next command ids to sub menus
    }
 
    return nextCommandId;
  }
 
  const uint32_t & Menu::GetCommandId() const
  {
    return mCommandId;
  }
 
  void Menu::SetCommandId(const uint32_t & iCommandId)
  {
    mCommandId = iCommandId;
  }
 
  bool Menu::IsVisible() const
  {
    return mVisible;
  }

  bool Menu::IsEnabled() const
  {
    return mEnabled;
  }

  void Menu::SetVisible(bool visible)
  {
    mVisible = visible;
  }

  void Menu::SetEnabled(bool enabled)
  {
    mEnabled = enabled;
  }

  const Validator & Menu::GetValidity()
  {
    return mValidity;
  }

  void Menu::SetValidity(const Validator & iValidity)
  {
    mValidity = iValidity;
  }

  const Validator & Menu::GetVisibility()
  {
    return mVisibility;
  }

  void Menu::SetVisibility(const Validator & iVisibility)
  {
    mVisibility = iVisibility;
  }

  Menu::MenuPtrList Menu::GetSubMenus()
  {
    Menu::MenuPtrList sub_menus = FilterNodes<Menu*>(this->FindChildren("Menu"));
    return sub_menus;
  }

  void Menu::AddAction(Action * action)
  {
    mActions.push_back(action);
  }

  const Action::ActionPtrList & Menu::GetActions() const
  {
    return mActions;
  }

} //namespace shellanything
