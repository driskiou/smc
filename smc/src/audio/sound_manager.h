/***************************************************************************
 * sound_manager.h
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

#ifndef SMC_SOUND_MANAGER_H
#define SMC_SOUND_MANAGER_H

#include "core/global_basic.h"
#include "core/obj_manager.h"
// SDL
// also includes needed SDL headers
#include "SDL_mixer.h"

namespace SMC {

/* *** *** *** *** *** *** *** Sound object *** *** *** *** *** *** *** *** ***
 * *** */

class cSound {
public:
  cSound(void);
  virtual ~cSound(void);

  // Load the data
  bool Load(const std::string &filename);
  // Free the data
  void Free(void);

  // filename
  std::string m_filename;
  // data if loaded else null
  Mix_Chunk *m_chunk;
};

typedef vector<cSound *> SoundList;

/* *** *** *** *** *** *** cSound_Manager *** *** *** *** *** *** *** *** ***
 * *** *** */

/*  Keeps track of all sounds in memory
 *
 * Operators:
 * - cSound_Manager [path]
 * - cSound_Manager [ID]
 */
class cSound_Manager : public cObject_Manager<cSound> {
public:
  cSound_Manager(void);
  virtual ~cSound_Manager(void);

  // Return the Sound from Path
  virtual cSound *Get_Pointer(const std::string &path) const;

  /* Add a Sound
   * Should always have the path set
   */
  void Add(cSound *item);

  cSound *operator[](unsigned int identifier) const {
    return cObject_Manager<cSound>::Get_Pointer(identifier);
  }

  cSound *operator[](const std::string &path) const {
    return Get_Pointer(path);
  }

  // Delete all Sounds, but keep object vector entries
  void Delete_Sounds(void);

private:
  // sounds loaded since initialization
  unsigned int m_load_count;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Sound Manager
extern cSound_Manager *pSound_Manager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
