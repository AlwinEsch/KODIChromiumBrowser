/*
 *      Copyright (C) 2015-2019 Team KODI
 *      http:/kodi.tv
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AppOther.h"
#include "AppRenderer.h"

#include "include/base/cef_logging.h"
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_app.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_library_loader.h"

// These flags must match the Chromium values.
std::string kProcessType = "type";
std::string kRendererProcess = "renderer";
std::string kZygoteProcess = "zygote";
std::string kGPUProcess = "gpu-process";

int main(int argc, char* argv[])
{
  int ret = -1;

  // Get the path where this sandbox app part is located. This is needed
  // to know from where the needed libcef becomes loaded.
  std::string path = argv[0];
  path.erase(path.length()-std::strlen("kodichromium"), path.length());
  std::string cefLib = path + LIBRARY_PREFIX "cef" LIBRARY_SUFFIX;
  if (!cef_load_library(cefLib.c_str()))
  {
    fprintf(stderr, "FATAL: Failed to load CEF library: '%s'", argv[0]);
    return ret;
  }

  CefMainArgs main_args(argc, argv);

  // Parse command-line arguments.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromArgv(argc, argv);

  // Create a ClientApp of the correct type.
  CefRefPtr<CefApp> app;
  const std::string& process_type = command_line->GetSwitchValue(kProcessType);

#ifdef DEBUG
  fprintf(stderr, "DEBUG_INFO: Sandbox Kodi Chromium started for process_type %s\n", process_type.c_str());
#endif

  if (process_type != "")
  {
    if (process_type == kRendererProcess ||
        process_type == kZygoteProcess)
    {
      // On Linux the zygote process is used to spawn other process types. Since
      // we don't know what type of process it will be give it the renderer
      // client.
      app = new CWebAppRenderer();
    }
    else
    {
      app = new CWebAppOther();
    }

    // Execute the secondary process, if any.
    ret = CefExecuteProcess(main_args, app, nullptr);
  }
  else
  {
    fprintf(stderr, "FATAL: kodichromium sandbox seems to start as browser type (process type undefined!");
  }

  if (!cef_unload_library())
    fprintf(stderr, "FATAL: Failed to unload CEF library '%s'", argv[0]);

#ifdef DEBUG
  fprintf(stderr, "DEBUG_INFO: Sandbox Kodi Chromium ended for process_type %s\n", process_type.c_str());
#endif

  return ret;
}