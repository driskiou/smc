/***************************************************************************
 * level_settings.cpp  - level editor settings class
 *
 * Copyright (C) 2006 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "level/level_settings.h"
#include "audio/audio.h"
#include "core/filesystem/filesystem.h"
#include "core/framerate.h"
#include "core/game_core.h"
#include "core/global_basic.h"
#include "core/i18n.h"
#include "gui/generic.h"
#include "input/mouse.h"
#include "level/level.h"
#include "video/font.h"
#include "video/renderer.h"
// CEGUI
#include <CEGUI/WindowManager.h>
#include <CEGUI/widgets/Combobox.h>
#include <CEGUI/widgets/Editbox.h>
#include <CEGUI/widgets/Listbox.h>
#include <CEGUI/widgets/ListboxTextItem.h>
#include <CEGUI/widgets/MultiLineEditbox.h>
#include <CEGUI/widgets/PushButton.h>
#include <CEGUI/widgets/Slider.h>
#include <CEGUI/widgets/Spinner.h>
#include <CEGUI/widgets/TabControl.h>

namespace SMC
{

    /* *** *** *** *** *** cLevel_Settings *** *** *** *** *** *** *** *** *** ***
     * *** *** */

    cLevel_Settings ::cLevel_Settings(cSprite_Manager *sprite_manager,
                                      cLevel *level)
    {
        m_active = 0;

        m_level = level;
        m_camera = new cCamera(sprite_manager);
        m_gui_window = NULL;
        m_tabcontrol = NULL;
    }

    cLevel_Settings ::~cLevel_Settings(void)
    {
        Unload();

        delete m_camera;
    }

    void cLevel_Settings ::Init(void)
    {
        if (m_gui_window)
        {
            Unload();
        }

        // get window manager
        CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

        // GUI
        m_gui_window = wmgr.loadLayoutFromFile("level_settings/main.layout");
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->addChild(m_gui_window);

        // Tab Control
        m_tabcontrol = static_cast<CEGUI::TabControl *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("tabcontrol_main"));
        m_tabcontrol->addTab(
            wmgr.loadLayoutFromFile("level_settings/tab_main.layout"));
        m_tabcontrol->addTab(
            wmgr.loadLayoutFromFile("level_settings/tab_background.layout"));
        m_tabcontrol->addTab(
            wmgr.loadLayoutFromFile("level_settings/tab_global_effect.layout"));

        // Main
        // level filename
        CEGUI::Editbox *editbox_level_filename = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_level_filename"));
        editbox_level_filename->setText(
            Trim_Filename(m_level->m_level_filename, 0, 0).c_str());
        // music filename
        CEGUI::Editbox *editbox_music_filename = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_music_filename"));
        editbox_music_filename->setText(m_level->Get_Music_Filename(1).c_str());
        // author
        CEGUI::Editbox *editbox_author =
            static_cast<CEGUI::Editbox *>(CEGUI::System::getSingleton()
                                              .getDefaultGUIContext()
                                              .getRootWindow()
                                              ->getChildRecursive("editbox_author"));
        editbox_author->setText(
            reinterpret_cast<const CEGUI::utf8 *>(m_level->m_author.c_str()));
        // version
        CEGUI::Editbox *editbox_version =
            static_cast<CEGUI::Editbox *>(CEGUI::System::getSingleton()
                                              .getDefaultGUIContext()
                                              .getRootWindow()
                                              ->getChildRecursive("editbox_version"));
        editbox_version->setText(m_level->m_version.c_str());
        // difficulty
        m_spinner_difficulty = static_cast<CEGUI::Spinner *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("spinner_difficulty"));
        m_spinner_difficulty->setCurrentValue(
            static_cast<float>(m_level->m_difficulty));
        m_spinner_difficulty->subscribeEvent(
            CEGUI::Spinner::EventValueChanged,
            CEGUI::Event::Subscriber(&cLevel_Settings::Spinner_Difficulty_Changed,
                                     this));

        m_slider_difficulty = static_cast<CEGUI::Slider *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("slider_difficulty"));
        m_slider_difficulty->setCurrentValue(
            static_cast<float>(m_level->m_difficulty));
        m_slider_difficulty->subscribeEvent(
            CEGUI::Slider::EventValueChanged,
            CEGUI::Event::Subscriber(&cLevel_Settings::Slider_Difficulty_Changed,
                                     this));

        m_text_difficulty_name = CEGUI::System::getSingleton()
                                     .getDefaultGUIContext()
                                     .getRootWindow()
                                     ->getChildRecursive("text_difficulty_name");
        m_text_difficulty_name->setText(Get_Difficulty_Name(m_level->m_difficulty));

        // land type
        CEGUI::Combobox *combo_land_type = static_cast<CEGUI::Combobox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("combo_land_type"));
        // add all types
        for (unsigned int i = 0; i < LLT_LAST; i++)
        {
            combo_land_type->addItem(new CEGUI::ListboxTextItem(
                Get_Level_Land_Type_Name(static_cast<LevelLandType>(i)), i));
        }
        combo_land_type->setText(
            Get_Level_Land_Type_Name(m_level->m_land_type).c_str());
        // description
        CEGUI::MultiLineEditbox *multieditbox_description =
            static_cast<CEGUI::MultiLineEditbox *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("multieditbox_description"));
        multieditbox_description->setText(
            reinterpret_cast<const CEGUI::utf8 *>(m_level->m_description.c_str()));

        // camera limits
        CEGUI::Spinner *spinner = static_cast<CEGUI::Spinner *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("spinner_camera_limit_w"));
        spinner->setCurrentValue(m_level->m_camera_limits.m_w);
        spinner = static_cast<CEGUI::Spinner *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("spinner_camera_limit_h"));
        spinner->setCurrentValue(m_level->m_camera_limits.m_h);
        // fixed camera horizontal velocity
        spinner = static_cast<CEGUI::Spinner *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("spinner_camera_hor_vel"));
        spinner->setCurrentValue(m_level->m_fixed_camera_hor_vel);
        // last save time
        CEGUI::Editbox *editbox_save_time = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_save_time"));
        editbox_save_time->setText(
            Time_to_String(m_level->m_last_saved, "%Y-%m-%d  %H:%M:%S"));

        // add background image button
        CEGUI::PushButton *button_add_background_image =
            static_cast<CEGUI::PushButton *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("button_add_background_image"));
        button_add_background_image->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::Event::Subscriber(&cLevel_Settings::Add_Background_Image, this));
        // delete background image button
        CEGUI::PushButton *button_delete_background_image =
            static_cast<CEGUI::PushButton *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("button_delete_background_image"));
        button_delete_background_image->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::Event::Subscriber(&cLevel_Settings::Delete_Background_Image,
                                     this));
        // apply button
        CEGUI::PushButton *button_apply =
            static_cast<CEGUI::PushButton *>(CEGUI::System::getSingleton()
                                                 .getDefaultGUIContext()
                                                 .getRootWindow()
                                                 ->getChildRecursive("button_apply"));
        button_apply->subscribeEvent(
            CEGUI::PushButton::EventClicked,
            CEGUI::Event::Subscriber(&cLevel_Settings::Button_Apply, this));

        // Background
        // listbox
        CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("listbox_backgrounds"));
        listbox->setSortingEnabled(1);
        listbox->subscribeEvent(
            CEGUI::Listbox::EventSelectionChanged,
            CEGUI::Event::Subscriber(&cLevel_Settings::Set_Background_Image, this));
        // type
        CEGUI::Combobox *combobox = static_cast<CEGUI::Combobox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("combo_bg_image_type"));
        combobox->addItem(
            new CEGUI::ListboxTextItem(cBackground::Get_Type_Name(BG_NONE)));
        combobox->addItem(
            new CEGUI::ListboxTextItem(cBackground::Get_Type_Name(BG_IMG_TOP)));
        combobox->addItem(
            new CEGUI::ListboxTextItem(cBackground::Get_Type_Name(BG_IMG_BOTTOM)));
        combobox->addItem(
            new CEGUI::ListboxTextItem(cBackground::Get_Type_Name(BG_IMG_ALL)));
        combobox->subscribeEvent(
            CEGUI::Combobox::EventListSelectionAccepted,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Image, this));
        // filename
        CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_image_name"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Image, this));
        // speed
        spinner = static_cast<CEGUI::Spinner *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("spinner_bg_image_speed_x"));
        spinner->subscribeEvent(
            CEGUI::Spinner::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Image, this));
        spinner = static_cast<CEGUI::Spinner *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("spinner_bg_image_speed_y"));
        spinner->subscribeEvent(
            CEGUI::Spinner::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Image, this));
        // position
        editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_image_posx"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Image, this));
        editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_image_posy"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Image, this));
        editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_image_posz"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Image, this));
        // constant velocity
        spinner = static_cast<CEGUI::Spinner *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("spinner_bg_image_const_vel_x"));
        spinner->subscribeEvent(
            CEGUI::Spinner::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Image, this));
        spinner = static_cast<CEGUI::Spinner *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("spinner_bg_image_const_vel_y"));
        spinner->subscribeEvent(
            CEGUI::Spinner::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Image, this));
        // Gradient colors
        m_bg_color_1 =
            Color(m_level->m_background_manager->Get_Pointer(0)->m_color_1.red,
                  m_level->m_background_manager->Get_Pointer(0)->m_color_1.green,
                  m_level->m_background_manager->Get_Pointer(0)->m_color_1.blue, 255);
        m_bg_color_2 =
            Color(m_level->m_background_manager->Get_Pointer(0)->m_color_2.red,
                  m_level->m_background_manager->Get_Pointer(0)->m_color_2.green,
                  m_level->m_background_manager->Get_Pointer(0)->m_color_2.blue, 255);

        // color 1
        editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_start_red"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Colors, this));
        editbox->setText(int_to_string(m_bg_color_1.red).c_str());
        editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_start_green"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Colors, this));
        editbox->setText(int_to_string(m_bg_color_1.green).c_str());
        editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_start_blue"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Colors, this));
        editbox->setText(int_to_string(m_bg_color_1.blue).c_str());
        // color 2
        editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_end_red"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Colors, this));
        editbox->setText(int_to_string(m_bg_color_2.red).c_str());
        editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_end_green"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Colors, this));
        editbox->setText(int_to_string(m_bg_color_2.green).c_str());
        editbox = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_end_blue"));
        editbox->subscribeEvent(
            CEGUI::Editbox::EventKeyUp,
            CEGUI::Event::Subscriber(&cLevel_Settings::Update_BG_Colors, this));
        editbox->setText(int_to_string(m_bg_color_2.blue).c_str());

        Update_BG_Colors(CEGUI::EventArgs());
        Clear_Layer_Field();
        Load_BG_Image_List();
    }

    void cLevel_Settings ::Exit(void)
    {
        // back to level
        Game_Action = GA_ENTER_LEVEL;
        Game_Action_Data_Start.add(
            "screen_fadeout", CEGUI::PropertyHelper<int>::toString(EFFECT_OUT_BLACK));
        Game_Action_Data_Start.add("screen_fadeout_speed", "3");
        Game_Action_Data_End.add(
            "screen_fadein", CEGUI::PropertyHelper<int>::toString(EFFECT_IN_BLACK));
        Game_Action_Data_End.add("screen_fadein_speed", "3");
    }

    void cLevel_Settings ::Enter(void)
    {
        // set active camera
        pActive_Camera = m_camera;

        editor_enabled = 0;

        if (pMouseCursor->m_active_object)
        {
            pMouseCursor->Clear_Active_Object();
        }

        // Initialize level data
        Init();
        // set active
        m_active = 1;

        // update camera
        m_camera->Update_Position();
    }

    void cLevel_Settings ::Leave(void)
    {
        // get window manager
        CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

        // # Main Tab
        // filename
        std::string level_filename = CEGUI::System::getSingleton()
                                         .getDefaultGUIContext()
                                         .getRootWindow()
                                         ->getChildRecursive("editbox_level_filename")
                                         ->getText()
                                         .c_str();
        if (level_filename.length() > 1 &&
            Trim_Filename(m_level->m_level_filename, 0, 0).compare(level_filename) !=
                0)
        {
            m_level->Set_Filename(level_filename);
            if (Box_Question(_("Save ") + Trim_Filename(level_filename, 0, 0) + " ?"))
            {
                m_level->Save();
            }
        }
        // music
        std::string new_music = CEGUI::System::getSingleton()
                                    .getDefaultGUIContext()
                                    .getRootWindow()
                                    ->getChildRecursive("editbox_music_filename")
                                    ->getText()
                                    .c_str();
        // if the music is new
        if (pAudio->Is_Music_Playing() &&
            new_music.compare(m_level->Get_Music_Filename(1)) != 0)
        {
            m_level->Set_Music(new_music);
            pAudio->Fadeout_Music(1000);
        }
        // author
        m_level->Set_Author(CEGUI::System::getSingleton()
                                .getDefaultGUIContext()
                                .getRootWindow()
                                ->getChildRecursive("editbox_author")
                                ->getText()
                                .c_str());
        // version
        m_level->Set_Version(CEGUI::System::getSingleton()
                                 .getDefaultGUIContext()
                                 .getRootWindow()
                                 ->getChildRecursive("editbox_version")
                                 ->getText()
                                 .c_str());
        // difficulty
        m_level->Set_Difficulty(static_cast<CEGUI::Spinner *>(
                                    CEGUI::System::getSingleton()
                                        .getDefaultGUIContext()
                                        .getRootWindow()
                                        ->getChildRecursive("spinner_difficulty"))
                                    ->getCurrentValue());
        // land type
        m_level->Set_Land_Type(Get_Level_Land_Type_Id(
            static_cast<CEGUI::Combobox *>(CEGUI::System::getSingleton()
                                               .getDefaultGUIContext()
                                               .getRootWindow()
                                               ->getChildRecursive("combo_land_type"))
                ->getText()
                .c_str()));
        // description
        m_level->Set_Description(
            static_cast<CEGUI::MultiLineEditbox *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("multieditbox_description"))
                ->getText()
                .c_str());

        // Camera Limits
        pLevel_Manager->m_camera->Set_Limit_W(
            (static_cast<CEGUI::Spinner *>(
                 CEGUI::System::getSingleton()
                     .getDefaultGUIContext()
                     .getRootWindow()
                     ->getChildRecursive("spinner_camera_limit_w")))
                ->getCurrentValue());
        pLevel_Manager->m_camera->Set_Limit_H(
            (static_cast<CEGUI::Spinner *>(
                 CEGUI::System::getSingleton()
                     .getDefaultGUIContext()
                     .getRootWindow()
                     ->getChildRecursive("spinner_camera_limit_h")))
                ->getCurrentValue());
        m_level->m_camera_limits.m_w = pLevel_Manager->m_camera->m_limit_rect.m_w;
        m_level->m_camera_limits.m_h = pLevel_Manager->m_camera->m_limit_rect.m_h;
        // fixed camera horizontal velocity
        pLevel_Manager->m_camera->m_fixed_hor_vel =
            (static_cast<CEGUI::Spinner *>(
                 CEGUI::System::getSingleton()
                     .getDefaultGUIContext()
                     .getRootWindow()
                     ->getChildRecursive("spinner_camera_hor_vel")))
                ->getCurrentValue();
        m_level->m_fixed_camera_hor_vel = pLevel_Manager->m_camera->m_fixed_hor_vel;

        // # Background Tab
        // Gradient
        m_level->m_background_manager->Get_Pointer(0)->Set_Color_1(m_bg_color_1);
        m_level->m_background_manager->Get_Pointer(0)->Set_Color_2(m_bg_color_2);

        Unload();
    }

    void cLevel_Settings ::Unload(void)
    {
        if (!m_gui_window)
        {
            return;
        }

        // destroy CEGUI window
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->removeChild(m_gui_window);
        CEGUI::WindowManager::getSingleton().destroyWindow(m_gui_window);
        m_gui_window = NULL;
        m_tabcontrol = NULL;

        m_active = 0;
    }

    void cLevel_Settings ::Update(void)
    {
        // uhm...

        // update performance timer
        pFramerate->m_perf_timer[PERF_UPDATE_LEVEL_SETTINGS]->Update();
    }

    void cLevel_Settings ::Draw(void)
    {
        pVideo->Clear_Screen();
        pVideo->Draw_Rect(NULL, 0.00001f, &black);

        // update performance timer
        pFramerate->m_perf_timer[PERF_DRAW_LEVEL_SETTINGS]->Update();
    }

    bool cLevel_Settings ::Key_Down(SDL_Keycode key)
    {
        if (!m_active)
        {
            return 0;
        }

        if (key == SDLK_ESCAPE)
        {
            Exit();
        }
        else
        {
            // not processed
            return 0;
        }

        // key got processed
        return 1;
    }

    void cLevel_Settings ::Set_Level(cLevel *level) { m_level = level; }

    void cLevel_Settings ::Set_Sprite_Manager(cSprite_Manager *sprite_manager)
    {
        m_camera->Set_Sprite_Manager(sprite_manager);
    }

    bool cLevel_Settings ::Add_Background_Image(const CEGUI::EventArgs &event)
    {
        cBackground *background = new cBackground(m_level->m_sprite_manager);
        background->Set_Type(BG_IMG_BOTTOM);
        background->Set_Image(LEVEL_DEFAULT_BACKGROUND);

        m_level->m_background_manager->Add(background);

        Load_BG_Image_List();

        return 1;
    }

    bool cLevel_Settings ::Delete_Background_Image(const CEGUI::EventArgs &event)
    {
        CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("listbox_backgrounds"));
        CEGUI::ListboxItem *item = listbox->getFirstSelectedItem();

        if (!item)
        {
            return 1;
        }

        // get background
        cBackground *background = static_cast<cBackground *>(item->getUserData());
        // delete it
        m_level->m_background_manager->Delete(background);

        // update list
        Load_BG_Image_List();
        // clear background image gui
        Update_BG_Image(CEGUI::EventArgs());

        return 1;
    }

    bool cLevel_Settings ::Set_Background_Image(const CEGUI::EventArgs &event)
    {
        // get window manager
        CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

        CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("listbox_backgrounds"));
        CEGUI::ListboxItem *item = listbox->getFirstSelectedItem();

        // selected
        if (item)
        {
            // get background
            cBackground *background = static_cast<cBackground *>(item->getUserData());
            std::string background_filename = background->m_image_1_filename;

            // type
            CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("combo_bg_image_type"));
            editbox->setText(reinterpret_cast<const CEGUI::utf8 *>(
                background->Get_Type_Name().c_str()));
            // filename
            editbox = static_cast<CEGUI::Editbox *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("editbox_bg_image_name"));
            editbox->setText(background_filename.c_str());
            // position
            editbox = static_cast<CEGUI::Editbox *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("editbox_bg_image_posx"));
            editbox->setText(float_to_string(background->m_start_pos_x, 6, 0).c_str());
            editbox = static_cast<CEGUI::Editbox *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("editbox_bg_image_posy"));
            editbox->setText(float_to_string(background->m_start_pos_y, 6, 0).c_str());
            editbox = static_cast<CEGUI::Editbox *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("editbox_bg_image_posz"));
            editbox->setText(float_to_string(background->m_pos_z, 6, 0).c_str());
            // speed
            CEGUI::Spinner *spinner = static_cast<CEGUI::Spinner *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("spinner_bg_image_speed_x"));
            spinner->setCurrentValue(background->m_speed_x);
            spinner = static_cast<CEGUI::Spinner *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("spinner_bg_image_speed_y"));
            spinner->setCurrentValue(background->m_speed_y);
            // constant velocity
            spinner = static_cast<CEGUI::Spinner *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("spinner_bg_image_const_vel_x"));
            spinner->setCurrentValue(background->m_const_vel_x);
            spinner = static_cast<CEGUI::Spinner *>(
                CEGUI::System::getSingleton()
                    .getDefaultGUIContext()
                    .getRootWindow()
                    ->getChildRecursive("spinner_bg_image_const_vel_y"));
            spinner->setCurrentValue(background->m_const_vel_y);
        }
        // deselected
        else
        {
            Clear_Layer_Field();
        }

        return 1;
    }

    bool cLevel_Settings ::Button_Apply(const CEGUI::EventArgs &event)
    {
        Exit();
        return 1;
    }

    bool cLevel_Settings ::Update_BG_Colors(const CEGUI::EventArgs &event)
    {
        // get window manager
        CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

        CEGUI::Editbox *color_start_red = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_start_red"));
        CEGUI::Editbox *color_start_green = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_start_green"));
        CEGUI::Editbox *color_start_blue = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_start_blue"));
        CEGUI::Editbox *color_end_red = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_end_red"));
        CEGUI::Editbox *color_end_green = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_end_green"));
        CEGUI::Editbox *color_end_blue = static_cast<CEGUI::Editbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("editbox_bg_color_end_blue"));

        m_bg_color_1.red = string_to_int(color_start_red->getText().c_str());
        m_bg_color_1.green = string_to_int(color_start_green->getText().c_str());
        m_bg_color_1.blue = string_to_int(color_start_blue->getText().c_str());
        m_bg_color_2.red = string_to_int(color_end_red->getText().c_str());
        m_bg_color_2.green = string_to_int(color_end_green->getText().c_str());
        m_bg_color_2.blue = string_to_int(color_end_blue->getText().c_str());

        // color start
        color_start_red->setProperty(
            "NormalTextColour",
            CEGUI::PropertyHelper<CEGUI::Colour>::toString(
                CEGUI::Colour(1, 1 - (static_cast<float>(m_bg_color_1.red) / 255),
                              1 - (static_cast<float>(m_bg_color_1.red) / 255), 1)));
        color_start_green->setProperty(
            "NormalTextColour",
            CEGUI::PropertyHelper<CEGUI::Colour>::toString(CEGUI::Colour(
                1 - (static_cast<float>(m_bg_color_1.green) / 255), 1,
                1 - (static_cast<float>(m_bg_color_1.green) / 255), 1)));
        color_start_blue->setProperty(
            "NormalTextColour",
            CEGUI::PropertyHelper<CEGUI::Colour>::toString(CEGUI::Colour(
                1 - (static_cast<float>(m_bg_color_1.blue) / 255),
                1 - (static_cast<float>(m_bg_color_1.blue) / 255), 1, 1)));
        // color end
        color_end_red->setProperty(
            "NormalTextColour",
            CEGUI::PropertyHelper<CEGUI::Colour>::toString(
                CEGUI::Colour(1, 1 - (static_cast<float>(m_bg_color_2.red) / 255),
                              1 - (static_cast<float>(m_bg_color_2.red) / 255), 1)));
        color_end_green->setProperty(
            "NormalTextColour",
            CEGUI::PropertyHelper<CEGUI::Colour>::toString(CEGUI::Colour(
                1 - (static_cast<float>(m_bg_color_2.green) / 255), 1,
                1 - (static_cast<float>(m_bg_color_2.green) / 255), 1)));
        color_end_blue->setProperty(
            "NormalTextColour",
            CEGUI::PropertyHelper<CEGUI::Colour>::toString(CEGUI::Colour(
                1 - (static_cast<float>(m_bg_color_2.blue) / 255),
                1 - (static_cast<float>(m_bg_color_2.blue) / 255), 1, 1)));

        CEGUI::Window *statictext = CEGUI::System::getSingleton()
                                        .getDefaultGUIContext()
                                        .getRootWindow()
                                        ->getChildRecursive("text_color_start");
        statictext->setProperty("TextColours",
                                CEGUI::PropertyHelper<CEGUI::Colour>::toString(
                                    m_bg_color_1.Get_cegui_Color()));
        statictext = CEGUI::System::getSingleton()
                         .getDefaultGUIContext()
                         .getRootWindow()
                         ->getChildRecursive("text_color_end");
        statictext->setProperty("TextColours",
                                CEGUI::PropertyHelper<CEGUI::Colour>::toString(
                                    m_bg_color_2.Get_cegui_Color()));

        return 1;
    }

    void cLevel_Settings ::Load_BG_Image_List(void)
    {
        CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("listbox_backgrounds"));
        listbox->resetList();

        for (vector<cBackground *>::iterator itr =
                 m_level->m_background_manager->objects.begin();
             itr != m_level->m_background_manager->objects.end(); ++itr)
        {
            cBackground *background = (*itr);

            // skip gradients
            if (background->m_type == BG_GR_HOR || background->m_type == BG_GR_VER)
            {
                continue;
            }

            CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem(
                float_to_string(background->m_pos_z).c_str(), 0, background);
            item->setSelectionColours(CEGUI::Colour(0.33f, 0.33f, 0.33f));
            item->setSelectionBrushImage("TaharezLook/ListboxSelectionBrush");
            listbox->addItem(static_cast<CEGUI::ListboxItem *>(item));
        }

        CEGUI::PushButton *button_add = static_cast<CEGUI::PushButton *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("button_add_background_image"));

        // 9 layers + default background is the maximum
        if (m_level->m_background_manager->size() >= 10)
        {
            button_add->disable();
        }
        else
        {
            button_add->enable();
        }
    }

    bool cLevel_Settings ::Update_BG_Image(const CEGUI::EventArgs &event)
    {
        // get window manager
        CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

        CEGUI::Listbox *listbox = static_cast<CEGUI::Listbox *>(
            CEGUI::System::getSingleton()
                .getDefaultGUIContext()
                .getRootWindow()
                ->getChildRecursive("listbox_backgrounds"));
        CEGUI::ListboxItem *item = listbox->getFirstSelectedItem();

        // clear
        if (!item)
        {
            Clear_Layer_Field();
            return 1;
        }

        std::string bg_type = CEGUI::System::getSingleton()
                                  .getDefaultGUIContext()
                                  .getRootWindow()
                                  ->getChildRecursive("combo_bg_image_type")
                                  ->getText()
                                  .c_str();
        std::string bg_filename = CEGUI::System::getSingleton()
                                      .getDefaultGUIContext()
                                      .getRootWindow()
                                      ->getChildRecursive("editbox_bg_image_name")
                                      ->getText()
                                      .c_str();
        float posx =
            string_to_float((CEGUI::System::getSingleton()
                                 .getDefaultGUIContext()
                                 .getRootWindow()
                                 ->getChildRecursive("editbox_bg_image_posx"))
                                ->getText()
                                .c_str());
        float posy =
            string_to_float((CEGUI::System::getSingleton()
                                 .getDefaultGUIContext()
                                 .getRootWindow()
                                 ->getChildRecursive("editbox_bg_image_posy"))
                                ->getText()
                                .c_str());
        float posz =
            string_to_float((CEGUI::System::getSingleton()
                                 .getDefaultGUIContext()
                                 .getRootWindow()
                                 ->getChildRecursive("editbox_bg_image_posz"))
                                ->getText()
                                .c_str());
        float speed_x = (static_cast<CEGUI::Spinner *>(
                             CEGUI::System::getSingleton()
                                 .getDefaultGUIContext()
                                 .getRootWindow()
                                 ->getChildRecursive("spinner_bg_image_speed_x")))
                            ->getCurrentValue();
        float speed_y = (static_cast<CEGUI::Spinner *>(
                             CEGUI::System::getSingleton()
                                 .getDefaultGUIContext()
                                 .getRootWindow()
                                 ->getChildRecursive("spinner_bg_image_speed_y")))
                            ->getCurrentValue();
        float const_vel_x =
            (static_cast<CEGUI::Spinner *>(
                 CEGUI::System::getSingleton()
                     .getDefaultGUIContext()
                     .getRootWindow()
                     ->getChildRecursive("spinner_bg_image_const_vel_x")))
                ->getCurrentValue();
        float const_vel_y =
            (static_cast<CEGUI::Spinner *>(
                 CEGUI::System::getSingleton()
                     .getDefaultGUIContext()
                     .getRootWindow()
                     ->getChildRecursive("spinner_bg_image_const_vel_y")))
                ->getCurrentValue();

        // get background
        cBackground *background = static_cast<cBackground *>(item->getUserData());
        // set type
        background->Set_Type(bg_type);
        // set position
        background->Set_Start_Pos(posx, posy);
        // set position z
        background->Set_Pos_Z(posz);
        // set scroll speed
        background->Set_Scroll_Speed(speed_x, speed_y);
        // set constant velocity
        background->Set_Const_Velocity_X(const_vel_x);
        background->Set_Const_Velocity_Y(const_vel_y);

        // full filename for validation
        bg_filename.insert(0, DATA_DIR "/" GAME_PIXMAPS_DIR "/");

        // invalid file
        if (!File_Exists(bg_filename))
        {
            // clear image
            bg_filename.clear();
        }

        // set image
        background->Set_Image(bg_filename);
        // set new item name
        item->setText(float_to_string(posz).c_str());
        // fixme : should update sorting because of the new name
        listbox->handleUpdatedItemData();

        return 1;
    }

    void cLevel_Settings ::Clear_Layer_Field(void)
    {
        // get window manager
        CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->getChildRecursive("combo_bg_image_type")
            ->setText("Disabled");
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->getChildRecursive("editbox_bg_image_name")
            ->setText("");
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->getChildRecursive("editbox_bg_image_posx")
            ->setText("");
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->getChildRecursive("editbox_bg_image_posy")
            ->setText("");
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->getChildRecursive("editbox_bg_image_posz")
            ->setText("");
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->getChildRecursive("spinner_bg_image_speed_x")
            ->setText("");
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->getChildRecursive("spinner_bg_image_speed_y")
            ->setText("");
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->getChildRecursive("spinner_bg_image_const_vel_x")
            ->setText("");
        CEGUI::System::getSingleton()
            .getDefaultGUIContext()
            .getRootWindow()
            ->getChildRecursive("spinner_bg_image_const_vel_y")
            ->setText("");
    }

    bool cLevel_Settings ::Spinner_Difficulty_Changed(
        const CEGUI::EventArgs &event)
    {
        const CEGUI::WindowEventArgs &windowEventArgs =
            static_cast<const CEGUI::WindowEventArgs &>(event);
        CEGUI::Spinner *spinner =
            static_cast<CEGUI::Spinner *>(windowEventArgs.window);
        float val = spinner->getCurrentValue();

        m_slider_difficulty->setCurrentValue(val);
        m_text_difficulty_name->setText(Get_Difficulty_Name(val));

        return 1;
    }

    bool cLevel_Settings ::Slider_Difficulty_Changed(
        const CEGUI::EventArgs &event)
    {
        const CEGUI::WindowEventArgs &windowEventArgs =
            static_cast<const CEGUI::WindowEventArgs &>(event);
        float val =
            static_cast<CEGUI::Slider *>(windowEventArgs.window)->getCurrentValue();

        m_spinner_difficulty->setCurrentValue(val);
        m_text_difficulty_name->setText(Get_Difficulty_Name(val));

        return 1;
    }

    /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
