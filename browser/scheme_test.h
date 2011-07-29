// Copyright (c) 2009 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#pragma once
#include "../3rd/cef_binary_r78_VS2005/include/cef.h"

// Register the scheme handler.
void InitSchemeTest();

// Run the test.
void RunSchemeTest(CefRefPtr<CefBrowser> browser);
