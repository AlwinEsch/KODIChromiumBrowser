/*
 *  Copyright (C) 2015-2020 Alwin Esch (Team Kodi)
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "include/cef_v8.h"
#include "include/wrapper/cef_helpers.h"

class CWebAppRenderer;

class CV8Handler : public CefV8Handler
{
public:
  CV8Handler(CWebAppRenderer* renderer);
  ~CV8Handler() override;

  bool InitKodiAPI(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context);

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) override;

  static bool OnWebKitInitialized(CWebAppRenderer* renderer);

private:
  IMPLEMENT_REFCOUNTING(CV8Handler);
  DISALLOW_COPY_AND_ASSIGN(CV8Handler);

  CWebAppRenderer* m_renderer;
};
