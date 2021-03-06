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

#ifndef SA_MENU_H
#define SA_MENU_H

#include "shellanything/Node.h"
#include "shellanything/Icon.h"
#include "shellanything/Validator.h"
#include "shellanything/Action.h"
#include <string>
#include <vector>
#include <stdint.h>

namespace shellanything
{

  /// <summary>
  /// The Menu class defines a displayed menu option.
  /// </summary>
  class Menu : public Node
  {
  public:
    /// <summary>
    /// A list of Menu pointers.
    /// </summary>
    typedef std::vector<Menu*> MenuPtrList;

    /// <summary>
    /// An invalid command id.
    /// </summary>
    static const uint32_t INVALID_COMMAND_ID;

    /// <summary>
    /// The default maximum length for the 'name' parameter.
    /// Default initialization value for the 'GetNameMaxLength()' method.
    /// </summary>
    static const int DEFAULT_NAME_MAX_LENGTH;

    Menu();
    virtual ~Menu();

  private:
    // Disable copy constructor and copy operator
    Menu(const Menu&);
    Menu& operator=(const Menu&);
  public:

    /// <summary>
    /// Returns true of the menu is a separator.
    /// </summary>
    /// <returns>Returns true of the menu is a separator. Returns false otherwise.</returns>
    bool IsSeparator() const;

    /// <summary>
    /// Sets the menu separator property.
    /// </summary>
    /// <param name="iSeparator">True to define the menu as a separator. False otherwise.</param>
    void SetSeparator(bool iSeparator);

    /// <summary>
    /// Returns true if the menu is a parent menu (if this menu have submenus).
    /// </summary>
    /// <returns>Returns true if the menu is a parent menu (if this menu have submenus). Returns false otherwise.</returns>
    bool IsParentMenu() const;

    /// <summary>
    /// Getter for the 'name' parameter.
    /// </summary>
    const std::string & GetName() const;

    /// <summary>
    /// Setter for the 'name' parameter.
    /// </summary>
    void SetName(const std::string & iName);

    /// <summary>
    /// Getter for the 'max_length' parameter.
    /// </summary>
    const int & GetNameMaxLength() const;

    /// <summary>
    /// Setter for the 'max_length' parameter.
    /// </summary>
    void SetNameMaxLength(const int & iNameMaxLength);

    /// <summary>
    /// Truncate a string to the maximum length allowed by this menu.
    /// Note, the given string must be already expanded.
    /// If the length of the given string is longer than DEFAULT_NAME_MAX_LENGTH, a trailing "..." will
    /// be added at the end of a string to indicate that the maximum supported length is reached.
    /// </summary>
    /// <remarks>
    /// There is no need to validate the maximum length of a given string if it is not already expanded
    /// because property expansion can shorten or lengthen the given string.
    /// </remarks>
    /// <param name="str">The expanded string value which length must be validated.</param>
    void TruncateName(std::string & str);

    /// <summary>
    /// Getter for the 'description' parameter.
    /// </summary>
    const std::string & GetDescription() const;

    /// <summary>
    /// Setter for the 'description' parameter.
    /// </summary>
    void SetDescription(const std::string & iDescription);

    /// <summary>
    /// Get this menu icon instance.
    /// </summary>
    const Icon & GetIcon() const;

    /// <summary>
    /// Set this menu icon instance.
    /// </summary>
    /// <param name="iIcon">An icon definition</param>
    void SetIcon(const Icon & iIcon);

    /// <summary>
    /// Updates the menu and submenus 'visible' and 'enabled' properties based on the given Context.
    /// </summary>
    /// <param name="c">The context used for updating the menu.</param>
    void Update(const Context & c);

    /// <summary>
    /// Searches this menu and submenus for a menu whose command id is iCommandId.
    /// </summary>
    /// <param name="iCommandId">The search command id value.</param>
    /// <returns>Returns a Menu pointer if a match is found. Returns NULL otherwise.</returns>
    Menu * FindMenuByCommandId(const uint32_t & iCommandId);

    /// <summary>
    /// Assign unique command id to this menus and submenus.
    /// </summary>
    /// <param name="iFirstCommandId">The first command id available.</param>
    /// <returns>Returns the next available command id. Returns iFirstCommandId if no command id was assigned.</returns>
    uint32_t AssignCommandIds(const uint32_t & iFirstCommandId);

    /// <summary>
    /// Getter for the 'command-id' parameter.
    /// </summary>
    const uint32_t & GetCommandId() const;

    /// <summary>
    /// Setter for the 'command-id' parameter.
    /// </summary>
    void SetCommandId(const uint32_t & iCommandId);

    /// <summary>
    /// Getter for the 'visible' parameter.
    /// </summary>
    bool IsVisible() const;

    /// <summary>
    /// Setter for the 'visible' parameter.
    /// </summary>
    void SetVisible(bool visible);

    /// <summary>
    /// Getter for the 'enabled' parameter.
    /// </summary>
    bool IsEnabled() const;

    /// <summary>
    /// Setter for the 'enabled' parameter.
    /// </summary>
    void SetEnabled(bool enabled);

    /// <summary>
    /// Get the Validator for the 'validity' parameter. 
    /// </summary>
    const Validator & GetValidity();

    /// <summary>
    /// Set the Validator for the 'validity' parameter.
    /// </summary>
    /// <param name="iValidity">Set the new Validator for the 'validity' parameter.</param>
    void SetValidity(const Validator & iValidity);

    /// <summary>
    /// Get the Validator for the 'visibility' parameter.
    /// </summary>
    const Validator & GetVisibility();

    /// <summary>
    /// Set the Validator for the 'visibility' parameter.
    /// </summary>
    /// <param name="iVisibility">Set the new Validator for the 'visibility' parameter.</param>
    void SetVisibility(const Validator & iVisibility);

    /// <summary>
    /// Add a new Action to the menu. The menu instance takes ownership of the action.
    /// </summary>
    /// <param name="action">The given action to add to the menu</param>
    void AddAction(Action * action);
    
    /// <summary>
    /// Get the list of action of the menu.
    /// </summary>
    const Action::ActionPtrList & GetActions() const;

    /// <summary>
    /// Get the list of submenu of the menu.
    /// </summary>
    MenuPtrList GetSubMenus();

  private:
    Icon mIcon;
    Validator mValidity;
    Validator mVisibility;
    bool mVisible;
    bool mEnabled;
    bool mSeparator;
    uint32_t mCommandId;
    std::string mName;
    int mNameMaxLength;
    std::string mDescription;
    Action::ActionPtrList mActions;
  };

} //namespace shellanything

#endif //SA_MENU_H
