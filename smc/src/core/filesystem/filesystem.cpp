/***************************************************************************
 * filesystem.cpp  -  File System
 *
 * Copyright (C) 2005 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <filesystem>
#include <QDir>
#include "core/filesystem/filesystem.h"
#include "core/game_core.h"

namespace fs = std::filesystem;

namespace SMC
{

  /* *** *** *** *** *** *** cResource_Manager *** *** *** *** *** *** *** *** ***
   * *** *** */

  std::string Trim_Filename(std::string filename, bool keep_dir /* = 1 */,
                            bool keep_end /* = 1 */)
  {
    if (!keep_dir && filename.find("/") != std::string::npos)
    {
      filename.erase(0, filename.rfind("/") + 1);
    }

    if (!keep_end && filename.rfind(".") != std::string::npos)
    {
      filename.erase(filename.rfind("."));
    }

    return filename;
  }

  bool File_Exists(const std::string &filename)
  {
    auto status = fs::status(fs::path(filename));
    return fs::is_symlink(status) || fs::is_regular_file(status);
  }

  bool Dir_Exists(const std::string &dir)
  {
    auto status = fs::status(fs::path(dir));
    return fs::is_directory(status) || fs::is_symlink(status);
  }

  bool Delete_File(const std::string &filename)
  {
    return fs::remove(fs::path(filename));
  }

  bool Delete_Dir(const std::string &dir)
  {
    return fs::remove(fs::path(dir));
  }

  bool Delete_Dir_And_Content(const std::string &dir)
  {
    return fs::remove_all(fs::path(dir)) > 0;
  }

  bool Rename_File(const std::string &old_filename,
                   const std::string &new_filename)
  {
    try
    {
      fs::rename(fs::path(old_filename), fs::path(new_filename));
    }
    catch (const fs::filesystem_error &error)
    {
      return false;
    }

    return true;
  }

  bool Create_Directory(const std::string &dir)
  {
    if (dir.empty())
    {
      return false;
    }

    return fs::create_directory(fs::path(dir));
  }

  bool Create_Directories(const std::string &dir)
  {
    return fs::create_directories(fs::path(dir));
  }

  size_t Get_File_Size(const std::string &filename)
  {

    if (File_Exists(filename))
    {
      return fs::file_size(fs::path(filename));
    }

    return 0;
  }

  void Convert_Path_Separators(std::string &str)
  {
    for (std::string::iterator itr = str.begin(); itr != str.end(); ++itr)
    {
      // convert it
      if (*itr == '\\' || *itr == '!')
      {
        *itr = '/';
      }
    }
  }

  vector<std::string> Get_Directory_Files(
      const std::string &dir, const std::string &file_type /* = "" */,
      bool with_directories /* = 0 */, bool search_in_sub_directories /* = 1 */)
  {
    vector<std::string> valid_files;

    fs::path full_path(dir);

    fs::directory_iterator end_iter;

    // load all available objects
    for (fs::directory_iterator dir_itr(full_path); dir_itr != end_iter;
         ++dir_itr)
    {
      try
      {
        const std::string filename_str = dir_itr->path().filename().string();

        // if directory
        if (fs::is_directory(*dir_itr))
        {
          // ignore hidden directories
          if (filename_str.find(".") == 0)
          {
            continue;
          }

          if (with_directories)
          {
            valid_files.push_back(dir + "/" + filename_str);
          }

          // load all items from the sub-directory
          if (search_in_sub_directories)
          {
            vector<std::string> new_valid_files = Get_Directory_Files(
                dir + "/" + filename_str, file_type, with_directories);
            valid_files.insert(valid_files.end(), new_valid_files.begin(),
                               new_valid_files.end());
          }
        }
        // valid file
        else if (file_type.empty() ||
                 filename_str.rfind(file_type) != std::string::npos)
        {
          valid_files.push_back(dir + "/" + filename_str);
        }
      }
      catch (const std::exception &ex)
      {
        printf("%s %s\n", dir_itr->path().string().c_str(), ex.what());
      }
    }

    return valid_files;
  }

  std::string Get_Temp_Directory(void)
  {
    return fs::temp_directory_path().generic_string();
  }

  std::string Get_User_Directory(void)
  {
    return (fs::path(QDir::homePath().toStdString()) / ".smc/").string();
  }

  /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
