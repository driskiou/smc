/***************************************************************************
 * flyon.cpp  -  flying plant
 *
 * Copyright (C) 2003 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "enemies/flyon.h"
#include "core/filesystem/filesystem.h"
#include "core/game_core.h"
#include "core/i18n.h"
#include "core/math/utilities.h"
#include "input/mouse.h"
#include "level/level_player.h"
#include "user/savegame.h"
#include "video/animation.h"
#include "video/gl_surface.h"
// CEGUI
#include "CEGUI/widgets/Combobox.h"
#include "CEGUI/widgets/ListboxTextItem.h"
#include <CEGUI/WindowManager.h>
#include <CEGUI/XMLAttributes.h>
#include <CEGUI/widgets/Editbox.h>

namespace SMC {

/* *** *** *** *** *** *** cFlyon *** *** *** *** *** *** *** *** *** *** *** */

cFlyon ::cFlyon(cSprite_Manager *sprite_manager) : cEnemy(sprite_manager) {
  cFlyon::Init();
}

cFlyon ::cFlyon(CEGUI::XMLAttributes &attributes,
                cSprite_Manager *sprite_manager)
    : cEnemy(sprite_manager) {
  cFlyon::Init();
  cFlyon::Load_From_XML(attributes);
}

cFlyon ::~cFlyon(void) {
  //
}

void cFlyon ::Init(void) {
  m_type = TYPE_FLYON;
  m_pos_z = 0.06f;
  Set_Rotation_Affects_Rect(1);
  m_editor_pos_z = 0.089f;
  m_camera_range = 1000;
  m_can_be_on_ground = 0;
  m_can_be_ground = 0;

  m_state = STA_STAY;
  Set_Direction(DIR_UP);
  Set_Image_Dir("enemy/flyon/orange/");
  Set_Max_Distance(200);
  Set_Speed(5.8f);

  m_kill_sound = "enemy/flyon/die.ogg";
  m_kill_points = 100;

  m_wait_time = Get_Random_Float(0.0f, 70.0f);
  m_move_back = 0;
}

cFlyon *cFlyon ::Copy(void) const {
  cFlyon *jpiranha = new cFlyon(m_sprite_manager);
  jpiranha->Set_Pos(m_start_pos_x, m_start_pos_y);
  jpiranha->Set_Direction(m_start_direction);
  jpiranha->Set_Image_Dir(m_img_dir);
  jpiranha->Set_Max_Distance(m_max_distance);
  jpiranha->Set_Speed(m_speed);
  return jpiranha;
}

void cFlyon ::Load_From_XML(CEGUI::XMLAttributes &attributes) {
  // position
  Set_Pos(static_cast<float>(attributes.getValueAsInteger("posx")),
          static_cast<float>(attributes.getValueAsInteger("posy")), 1);
  // direction
  Set_Direction(Get_Direction_Id(
      attributes
          .getValueAsString("direction", Get_Direction_Name(m_start_direction))
          .c_str()));
  // image directory
  Set_Image_Dir(attributes.getValueAsString("image_dir", m_img_dir).c_str());
  // max distance
  Set_Max_Distance(attributes.getValueAsFloat("max_distance", m_max_distance));
  // speed
  Set_Speed(attributes.getValueAsFloat("speed", m_speed));
}

void cFlyon ::Save_To_XML(CEGUI::XMLSerializer &stream) {
  // begin
  stream.openTag(m_type_name);

  // name
  Write_Property(stream, "type", "flyon");
  // position
  Write_Property(stream, "posx", static_cast<int>(m_start_pos_x));
  Write_Property(stream, "posy", static_cast<int>(m_start_pos_y));
  // direction
  Write_Property(stream, "direction", Get_Direction_Name(m_start_direction));
  // image directory
  Write_Property(stream, "image_dir", m_img_dir);
  // max distance
  Write_Property(stream, "max_distance", static_cast<int>(m_max_distance));
  // speed
  Write_Property(stream, "speed", m_speed);

  // end
  stream.closeTag();
}

void cFlyon ::Load_From_Savegame(cSave_Level_Object *save_object) {
  cEnemy::Load_From_Savegame(save_object);

  // move_back
  if (save_object->exists("move_back")) {
    m_move_back = string_to_int(save_object->Get_Value("move_back")) > 0;
  }
}

cSave_Level_Object *cFlyon ::Save_To_Savegame(void) {
  cSave_Level_Object *save_object = cEnemy::Save_To_Savegame();

  // move_back ( only save if needed )
  if (m_move_back) {
    save_object->m_properties.push_back(
        cSave_Level_Object_Property("move_back", int_to_string(m_move_back)));
  }

  return save_object;
}

void cFlyon ::Set_Image_Dir(std::string dir) {
  if (dir.empty()) {
    return;
  }

  // remove pixmaps dir
  if (dir.find(DATA_DIR "/" GAME_PIXMAPS_DIR "/") == 0) {
    dir.erase(0, strlen(DATA_DIR "/" GAME_PIXMAPS_DIR "/"));
  }

  // add trailing slash if missing
  if (*(dir.end() - 1) != '/') {
    dir.insert(dir.length(), "/");
  }

  // if not image directory
  if (!File_Exists(DATA_DIR "/" GAME_PIXMAPS_DIR "/" + dir + "closed_1.png") &&
      !File_Exists(DATA_DIR "/" GAME_PIXMAPS_DIR "/" + dir +
                   "closed_1.settings")) {
    printf("Warning : Flyon image dir does not exist %s\n", dir.c_str());
    return;
  }

  m_img_dir = dir;

  // clear images
  Clear_Images();
  // set images
  Add_Image(pVideo->Get_Surface(m_img_dir + "closed_1.png"));
  Add_Image(pVideo->Get_Surface(m_img_dir + "closed_2.png"));
  Add_Image(pVideo->Get_Surface(m_img_dir + "open_1.png"));
  Add_Image(pVideo->Get_Surface(m_img_dir + "open_2.png"));
  // set start image
  Set_Image_Num(0, 1);

  Set_Animation(1);
  Set_Animation_Image_Range(0, 3);
  Set_Time_All(130, 1);
  Reset_Animation();

  Create_Name();
}

void cFlyon ::Set_Direction(const ObjectDirection dir) {
  // already set
  if (dir == m_direction) {
    return;
  }

  if (dir != DIR_UP && dir != DIR_DOWN && dir != DIR_LEFT && dir != DIR_RIGHT) {
    printf("Error : Unknown Flyon direction %d\n", m_direction);
    return;
  }

  cEnemy::Set_Direction(dir, 1);

  // clear
  Set_Rotation(0.0f, 0.0f, 0.0f, 1);

  if (m_start_direction == DIR_UP) {
    // default
  } else if (m_start_direction == DIR_LEFT) {
    Set_Rotation_Z(270.0f, 1);
  } else if (m_start_direction == DIR_RIGHT) {
    Set_Rotation_Z(90.0f, 1);
  } else if (m_start_direction == DIR_DOWN) {
    Set_Rotation_Z(180.0f, 1);
  }

  Set_Velocity(0.0f, 0.0f);
  Update_Dest_Vel();
  Create_Name();
}

void cFlyon ::Set_Max_Distance(float nmax_distance) {
  m_max_distance = nmax_distance;

  if (m_max_distance < 0.0f) {
    m_max_distance = 0.0f;
  }
}

void cFlyon ::Set_Speed(float val) {
  if (m_speed < 0.1f) {
    m_speed = 0.1f;
  }

  m_speed = val;

  Update_Dest_Vel();
}

void cFlyon ::DownGrade(bool force /* = 0 */) {
  Set_Dead(1);
  m_massive_type = MASS_PASSIVE;
  m_counter = 0.0f;
  m_velx = 0.0f;
  m_vely = 0.0f;

  if (!force) {
    // animation
    cParticle_Emitter *anim = new cParticle_Emitter(m_sprite_manager);
    Generate_Hit_Animation(anim);

    anim->Set_Speed(5.0f, 0.6f);
    anim->Set_Scale(0.8f);
    anim->Emit();
    pActive_Animation_Manager->Add(anim);
  } else {
    Set_Rotation_Z(180.0f);
  }
}

void cFlyon ::Update_Dying(void) {
  m_counter += pFramerate->m_speed_factor;

  // default death
  if (!Is_Float_Equal(m_rot_z, 180.0f)) {
    Set_Active(0);
  }
  // falling death
  else {
    // a little bit upwards first
    if (m_counter < 5.0f) {
      Move(0.0f, -5.0f);
    }
    // if not below the ground : fall
    else if (m_col_rect.m_y < pActive_Camera->m_limit_rect.m_y) {
      Move(0.0f, 20.0f);
    }
    // if below disable
    else {
      m_rot_z = 0.0f;
      Set_Active(0);
    }
  }
}

void cFlyon ::Set_Moving_State(Moving_state new_state) {
  if (new_state == m_state) {
    return;
  }

  if (new_state == STA_STAY) {
    m_velx = 0.0f;
    m_vely = 0.0f;

    m_move_back = 0;

    Set_Image_Num(0);
    Reset_Animation();
    Set_Animation(0);
  } else if (new_state == STA_FLY) {
    m_velx = m_dest_velx;
    m_vely = m_dest_vely;
    m_move_back = 0;
    Set_Animation(1);
  }

  m_state = new_state;
}

void cFlyon ::Update(void) {
  cEnemy::Update();

  if (!m_valid_update || !Is_In_Range()) {
    return;
  }

  Update_Animation();

  // standing ( waiting )
  if (m_state == STA_STAY) {
    // if waiting time
    if (m_wait_time > 0.0f) {
      m_wait_time -= pFramerate->m_speed_factor;

      if (m_wait_time < 0.0f) {
        m_wait_time = 0.0f;
      }
    }
    // no more waiting try to jump out
    else {
      GL_rect rect1 = m_col_rect;

      if (m_direction == DIR_UP) {
        rect1.m_y -= 40.0f;
        rect1.m_h += 40.0f;
      } else if (m_direction == DIR_DOWN) {
        rect1.m_y += 40.0f;
        rect1.m_h -= 40.0f;
      } else if (m_direction == DIR_LEFT) {
        rect1.m_x -= 35.0f;
        rect1.m_w += 35.0f;
      } else if (m_direction == DIR_RIGHT) {
        rect1.m_x += 35.0f;
        rect1.m_w += 35.0f;
      }

      // if player is in front: wait again
      if (pLevel_Player->m_maryo_type != MARYO_GHOST &&
          pLevel_Player->m_col_rect.Intersects(rect1)) {
        m_wait_time = speedfactor_fps * 2;
      }
      // if not: jump out
      else {
        Set_Moving_State(STA_FLY);
      }
    }
  }
  // flying ( moving into the destination direction )
  else {
    // distance to final position
    float dist_to_final_pos = Get_End_Distance();
    // multiplier for the minimal velocity
    float vel_mod_min =
        (dist_to_final_pos + (m_max_distance * 0.1f)) / m_max_distance;

    // if behind max distance
    if (vel_mod_min <= 0.1f) {
      vel_mod_min = 0.1f;
    }

    /* slow down
     * with the velocity mod which is calculated from the distance to the final
     * position
     */
    switch (m_direction) {
    case DIR_LEFT: {
      // move forward
      if (!m_move_back) {
        m_velx = m_dest_velx * vel_mod_min;
      }
      // move back
      else {
        m_velx = -m_dest_velx * vel_mod_min;
      }
      break;
    }
    case DIR_RIGHT: {
      // move forward
      if (!m_move_back) {
        m_velx = m_dest_velx * vel_mod_min;
      }
      // move back
      else {
        m_velx = -m_dest_velx * vel_mod_min;
      }
      break;
    }
    case DIR_UP: {
      // move forward
      if (!m_move_back) {
        m_vely = m_dest_vely * vel_mod_min;
      }
      // move back
      else {
        m_vely = -m_dest_vely * vel_mod_min;
      }
      break;
    }
    case DIR_DOWN: {
      // move forward
      if (!m_move_back) {
        m_vely = m_dest_vely * vel_mod_min;
      }
      // move back
      else {
        m_vely = -m_dest_vely * vel_mod_min;
      }
      break;
    }
    default: {
      break;
    }
    }

    // moving forward
    if (!m_move_back) {
      // reached final position move back
      if (dist_to_final_pos < 0.0f) {
        m_velx = -m_dest_velx * 0.01f;
        m_vely = -m_dest_vely * 0.01f;

        m_move_back = 1;
      }
    }
    // moving back
    else {
      // reached original position
      if (dist_to_final_pos > m_max_distance) {
        Set_Pos(m_start_pos_x, m_start_pos_y);
        m_wait_time = speedfactor_fps * 2;

        Set_Moving_State(STA_STAY);
      }
    }
  }
}

void cFlyon ::Draw(cSurface_Request *request /* = NULL */) {
  if (!m_valid_draw) {
    return;
  }

  // draw distance rect
  if (editor_level_enabled) {
    if (m_start_direction == DIR_RIGHT) {
      pVideo->Draw_Rect(m_start_pos_x - pActive_Camera->m_x,
                        m_start_pos_y + (m_rect.m_h * 0.5f) - 5.0f -
                            pActive_Camera->m_y,
                        m_max_distance + m_rect.m_w, 10.0f,
                        m_editor_pos_z - 0.000001f, &whitealpha128);
    } else if (m_start_direction == DIR_LEFT) {
      pVideo->Draw_Rect(m_start_pos_x - pActive_Camera->m_x + m_rect.m_w,
                        m_start_pos_y + (m_rect.m_h * 0.5f) - 5.0f -
                            pActive_Camera->m_y,
                        -(m_rect.m_w + m_max_distance), 10.0f,
                        m_editor_pos_z - 0.000001f, &whitealpha128);
    } else if (m_start_direction == DIR_DOWN) {
      pVideo->Draw_Rect(m_start_pos_x + (m_rect.m_w * 0.5f) - 5.0f -
                            pActive_Camera->m_x,
                        m_start_pos_y - pActive_Camera->m_y, 10.0f,
                        m_max_distance + m_rect.m_h, m_editor_pos_z - 0.000001f,
                        &whitealpha128);
    } else if (m_start_direction == DIR_UP) {
      pVideo->Draw_Rect(m_start_pos_x + (m_rect.m_w * 0.5f) - 5.0f -
                            pActive_Camera->m_x,
                        m_start_pos_y - pActive_Camera->m_y + m_rect.m_h, 10.0f,
                        -(m_rect.m_h + m_max_distance),
                        m_editor_pos_z - 0.000001f, &whitealpha128);
    }
  }

  cEnemy::Draw(request);
}

float cFlyon ::Get_End_Distance(void) const {
  switch (m_direction) {
  case DIR_LEFT: {
    return m_max_distance - (m_start_pos_x - m_pos_x);
  }
  case DIR_RIGHT: {
    return m_max_distance + (m_start_pos_x - m_pos_x);
  }
  case DIR_UP: {
    return m_max_distance - (m_start_pos_y - m_pos_y);
  }
  case DIR_DOWN: {
    return m_max_distance + (m_start_pos_y - m_pos_y);
  }
  default: {
    break;
  }
  }

  return 0;
}

void cFlyon ::Update_Dest_Vel(void) {
  if (m_direction == DIR_UP) {
    m_dest_velx = 0.0f;
    m_dest_vely = -m_speed;
  } else if (m_direction == DIR_DOWN) {
    m_dest_velx = 0.0f;
    m_dest_vely = m_speed;
  } else if (m_direction == DIR_LEFT) {
    m_dest_velx = -m_speed;
    m_dest_vely = 0.0f;
  } else if (m_direction == DIR_RIGHT) {
    m_dest_velx = m_speed;
    m_dest_vely = 0.0f;
  } else {
    m_dest_velx = 0.0f;
    m_dest_vely = 0.0f;
  }
}

bool cFlyon ::Is_Update_Valid(void) {
  if (m_dead || m_freeze_counter) {
    return 0;
  }

  return 1;
}

bool cFlyon ::Is_Draw_Valid(void) {
  bool valid = cEnemy::Is_Draw_Valid();

  // if editor enabled
  if (editor_enabled) {
    // if active mouse object
    if (pMouseCursor->m_active_object == this) {
      return 1;
    }
  }

  return valid;
}

Col_Valid_Type cFlyon ::Validate_Collision(cSprite *obj) {
  if (obj->m_massive_type == MASS_MASSIVE) {
    if (obj->m_type == TYPE_PLAYER) {
      return COL_VTYPE_INTERNAL;
    }
    if (obj->m_type == TYPE_BALL) {
      return COL_VTYPE_BLOCKING;
    }

    return COL_VTYPE_NOT_VALID;
  }

  return COL_VTYPE_NOT_VALID;
}

void cFlyon ::Handle_Collision_Player(cObjectCollision *collision) {
  // unknown direction
  if (collision->m_direction == DIR_UNDEFINED) {
    return;
  }

  if (pLevel_Player->m_maryo_type != MARYO_SMALL &&
      !pLevel_Player->m_invincible && collision->m_direction == m_direction) {
    // todo : create again
    // pAudio->PlaySound( "player/maryo_au.ogg", RID_MARYO_AU );
    pLevel_Player->Action_Jump(1);
  }

  pLevel_Player->DownGrade_Player();
}

void cFlyon ::Editor_Activate(void) {
  // get window manager
  CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

  // direction
  CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(
      wmgr.createWindow("TaharezLook/Combobox", "editor_flyon_direction"));
  Editor_Add(UTF8_("Direction"), UTF8_("Direction it moves into."), combobox,
             100, 110);

  combobox->addItem(new CEGUI::ListboxTextItem("up"));
  combobox->addItem(new CEGUI::ListboxTextItem("down"));
  combobox->addItem(new CEGUI::ListboxTextItem("left"));
  combobox->addItem(new CEGUI::ListboxTextItem("right"));

  combobox->setText(Get_Direction_Name(m_start_direction));
  combobox->subscribeEvent(
      CEGUI::Combobox::EventListSelectionAccepted,
      CEGUI::Event::Subscriber(&cFlyon::Editor_Direction_Select, this));

  // image dir
  CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(
      wmgr.createWindow("TaharezLook/Editbox", "editor_flyon_image_dir"));
  Editor_Add(UTF8_("Image directory"), UTF8_("Directory containing the images"),
             editbox, 200);

  editbox->setText(m_img_dir.c_str());
  editbox->subscribeEvent(
      CEGUI::Editbox::EventTextChanged,
      CEGUI::Event::Subscriber(&cFlyon::Editor_Image_Dir_Text_Changed, this));

  // max distance
  editbox = static_cast<CEGUI::Editbox *>(
      wmgr.createWindow("TaharezLook/Editbox", "editor_flyon_max_distance"));
  Editor_Add(UTF8_("Distance"), _("Movable Distance into its direction"),
             editbox, 90);

  editbox->setValidationString("^[+]?\\d*$");
  editbox->setText(int_to_string(static_cast<int>(m_max_distance)));
  editbox->subscribeEvent(CEGUI::Editbox::EventTextChanged,
                          CEGUI::Event::Subscriber(
                              &cFlyon::Editor_Max_Distance_Text_Changed, this));

  // speed
  editbox = static_cast<CEGUI::Editbox *>(
      wmgr.createWindow("TaharezLook/Editbox", "editor_flyon_speed"));
  Editor_Add(UTF8_("Speed"), UTF8_("Initial speed when jumping out"), editbox,
             120);

  editbox->setValidationString("[+]?[0-9]*\\.?[0-9]*");
  editbox->setText(float_to_string(m_speed, 6, 0));
  editbox->subscribeEvent(
      CEGUI::Editbox::EventTextChanged,
      CEGUI::Event::Subscriber(&cFlyon::Editor_Speed_Text_Changed, this));

  // init
  Editor_Init();
}

bool cFlyon ::Editor_Direction_Select(const CEGUI::EventArgs &event) {
  const CEGUI::WindowEventArgs &windowEventArgs =
      static_cast<const CEGUI::WindowEventArgs &>(event);
  CEGUI::ListboxItem *item =
      static_cast<CEGUI::Combobox *>(windowEventArgs.window)->getSelectedItem();

  Set_Direction(Get_Direction_Id(item->getText().c_str()));

  return 1;
}

bool cFlyon ::Editor_Image_Dir_Text_Changed(const CEGUI::EventArgs &event) {
  const CEGUI::WindowEventArgs &windowEventArgs =
      static_cast<const CEGUI::WindowEventArgs &>(event);
  std::string str_text =
      static_cast<CEGUI::Editbox *>(windowEventArgs.window)->getText().c_str();

  Set_Image_Dir(str_text);

  return 1;
}

bool cFlyon ::Editor_Max_Distance_Text_Changed(const CEGUI::EventArgs &event) {
  const CEGUI::WindowEventArgs &windowEventArgs =
      static_cast<const CEGUI::WindowEventArgs &>(event);
  std::string str_text =
      static_cast<CEGUI::Editbox *>(windowEventArgs.window)->getText().c_str();

  Set_Max_Distance(static_cast<float>(string_to_int(str_text)));

  return 1;
}

bool cFlyon ::Editor_Speed_Text_Changed(const CEGUI::EventArgs &event) {
  const CEGUI::WindowEventArgs &windowEventArgs =
      static_cast<const CEGUI::WindowEventArgs &>(event);
  std::string str_text =
      static_cast<CEGUI::Editbox *>(windowEventArgs.window)->getText().c_str();

  Set_Speed(string_to_float(str_text));

  return 1;
}

void cFlyon ::Create_Name(void) {
  m_name = "Flyon ";
  m_name += _(Get_Direction_Name(m_start_direction).c_str());

  if (m_start_image && !m_start_image->m_name.empty()) {
    m_name += " " + m_start_image->m_name;
  }
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
