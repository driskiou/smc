/***************************************************************************
 * world_editor.cpp  -  class for the World Editor
 *
 * Copyright (C) 2007 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "overworld/world_editor.h"
#include "audio/audio.h"
#include "core/filesystem/filesystem.h"
#include "core/game_core.h"
#include "core/global_basic.h"
#include "core/i18n.h"
#include "gui/generic.h"
#include "overworld/overworld.h"

namespace SMC {

/* *** *** *** *** *** *** *** cEditor_World *** *** *** *** *** *** *** *** ***
 * *** */

cEditor_World ::cEditor_World(cSprite_Manager *sprite_manager,
                              cOverworld *overworld)
    : cEditor(sprite_manager) {
  m_overworld = overworld;
  m_menu_filename = DATA_DIR "/" GAME_EDITOR_DIR "/world_menu.xml";
  m_items_filename = DATA_DIR "/" GAME_EDITOR_DIR "/world_items.xml";

  m_editor_item_tag = "world";
  m_camera_speed = 20;
}

cEditor_World ::~cEditor_World(void) {
  //
}

void cEditor_World ::Init(void) {
  // already loaded
  if (m_editor_window) {
    return;
  }

  // nothing

  cEditor::Init();
}

void cEditor_World ::Enable(void) {
  // already enabled
  if (m_enabled) {
    return;
  }

  editor_world_enabled = 1;
  pOverworld_Manager->m_draw_layer = 1;

  if (Game_Mode == MODE_OVERWORLD) {
    editor_enabled = 1;
  }

  cEditor::Enable();
}

void cEditor_World ::Disable(bool native_mode /* = 0 */) {
  // already disabled
  if (!m_enabled) {
    return;
  }

  pHud_Debug->Set_Text(_("World Editor disabled"));

  editor_world_enabled = 0;
  pOverworld_Manager->m_draw_layer = 0;
  pOverworld_Manager->m_camera_mode = 0;

  if (Game_Mode == MODE_OVERWORLD) {
    native_mode = 1;
    editor_enabled = 0;
  }

  cEditor::Disable(native_mode);
}

bool cEditor_World ::Key_Down(SDL_Keycode key) {
  if (!m_enabled) {
    return 0;
  }

  // check basic editor events
  if (cEditor::Key_Down(key)) {
    return 1;
  } else {
    // not processed
    return 0;
  }

  // key got processed
  return 1;
}

void cEditor_World ::Set_Overworld(cOverworld *overworld) {
  m_overworld = overworld;
}

void cEditor_World ::Activate_Menu_Item(cEditor_Menu_Object *entry) {
  // If Function
  if (entry->bfunction) {
    if (entry->tags.compare("new") == 0) {
      Function_New();
    } else if (entry->tags.compare("load") == 0) {
      Function_Load();
    } else if (entry->tags.compare("save") == 0) {
      Function_Save();
    }
    /*else if( entry->tags.compare( "save_as" ) == 0 )
    {
            Function_Save_as();
    }*/
    else if (entry->tags.compare("reload") == 0) {
      Function_Reload();
    }
    /*else if( entry->tags.compare( "settings" ) == 0 )
    {
            Function_Settings();
    }*/
    // unknown level function
    else {
      cEditor::Activate_Menu_Item(entry);
    }
  }
  // unknown level function
  else {
    cEditor::Activate_Menu_Item(entry);
  }
}

cSprite *cEditor_World ::Get_Object(const CEGUI::String &element,
                                    CEGUI::XMLAttributes &attributes,
                                    int engine_version) {
  return Create_World_Object_From_XML(element, attributes, engine_version,
                                      m_sprite_manager, m_overworld);
}

bool cEditor_World ::Function_New(void) {
  std::string world_name = Box_Text_Input(_("Create a new World"), _("Name"));

  // aborted/invalid
  if (world_name.empty()) {
    return 0;
  }

  if (pOverworld_Manager->New(world_name)) {
    Game_Action = GA_ENTER_WORLD;
    Game_Action_Data_Start.add(
        "screen_fadeout",
        CEGUI::PropertyHelper<int>::toString(EFFECT_OUT_BLACK_TILED_RECTS));
    Game_Action_Data_Start.add("screen_fadeout_speed", "3");
    Game_Action_Data_Middle.add("enter_world", world_name.c_str());
    Game_Action_Data_End.add(
        "screen_fadein", CEGUI::PropertyHelper<int>::toString(EFFECT_IN_BLACK));
    Game_Action_Data_End.add("screen_fadein_speed", "3");

    pHud_Debug->Set_Text(_("Created ") + world_name);
    return 1;
  } else {
    pHud_Debug->Set_Text(_("World ") + world_name + _(" already exists"));
  }

  return 0;
}

void cEditor_World ::Function_Load(void) {
  std::string world_name = _("Name");

  // valid world
  while (world_name.length()) {
    world_name = Box_Text_Input(world_name, _("Load an Overworld"),
                                world_name.compare(_("Name")) == 0 ? 1 : 0);

    // break if empty
    if (world_name.empty()) {
      break;
    }

    // success
    if (pOverworld_Manager->Get(world_name)) {
      Game_Action = GA_ENTER_WORLD;
      Game_Action_Data_Start.add(
          "screen_fadeout",
          CEGUI::PropertyHelper<int>::toString(EFFECT_OUT_BLACK_TILED_RECTS));
      Game_Action_Data_Start.add("screen_fadeout_speed", "3");
      Game_Action_Data_Middle.add("enter_world", world_name.c_str());
      Game_Action_Data_End.add(
          "screen_fadein",
          CEGUI::PropertyHelper<int>::toString(EFFECT_IN_BLACK));
      Game_Action_Data_End.add("screen_fadein_speed", "3");

      pHud_Debug->Set_Text(_("Loaded ") + world_name);
      break;
    }
    // failed
    else {
      pAudio->Play_Sound("error.ogg");
    }
  }
}

void cEditor_World ::Function_Save(bool with_dialog /* = 0 */) {
  // if denied
  if (with_dialog &&
      !Box_Question(_("Save ") +
                    Trim_Filename(m_overworld->m_description->m_name, 0, 0) +
                    " ?")) {
    return;
  }

  m_overworld->Save();
}

void cEditor_World ::Function_Reload(void) {
  // if denied
  if (!Box_Question(_("Reload World ?"))) {
    return;
  }

  m_overworld->Load();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cEditor_World *pWorld_Editor = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
