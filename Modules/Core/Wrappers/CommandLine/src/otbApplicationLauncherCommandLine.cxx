/*
 * Copyright (C) 2005-2024 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "otbWrapperCommandLineLauncher.h"
#include "otbConfigurationManager.h"
#include "otb_tinyxml.h"
#include <vector>

std::string CleanWord(const std::string& word)
{
  std::string res("");
  // Suppress whitespace characters at the beginning and ending of the string
  std::string::size_type cleanStart = word.find_first_not_of(" \t");
  std::string::size_type cleanEnd   = word.find_last_not_of(" \t\f\v\n\r");
  // cleanStart == npos implies cleanEnd == npos
  if (cleanEnd != std::string::npos)
  {
    res = word.substr(cleanStart, cleanEnd - cleanStart + 1);
  }
  return res;
}

void ShowUsage(char* argv[])
{
  std::cerr << "Usage: " << argv[0] << " module_name [MODULEPATH] [arguments]" << std::endl;
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    ShowUsage(argv);
    return EXIT_FAILURE;
  }

  std::vector<std::string> vexp;

  // Construct the string expression
  for (int i = 1; i < argc; i++)
  {
    std::string strarg(argv[i]);
    std::string cleanArg = CleanWord(strarg);
    if (cleanArg.empty())
    {
      // Empty argument!
      continue;
    }
    vexp.push_back(cleanArg);
  }

  if (vexp.empty())
  {
    ShowUsage(argv);
    return EXIT_FAILURE;
  }


  otb::ConfigurationManager::InitOpenMPThreads();

  typedef otb::Wrapper::CommandLineLauncher LauncherType;
  LauncherType::Pointer                     launcher = LauncherType::New();

  bool success = launcher->Load(vexp) && launcher->ExecuteAndWriteOutput();

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
