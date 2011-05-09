#/**********************************************************\ 
#
# Auto-Generated Plugin Configuration file
# for Plugin Gigaso
#
#\**********************************************************/

set(PLUGIN_NAME "PluginGigaso")
set(PLUGIN_PREFIX "PGI")
set(COMPANY_NAME "gigaso")

# ActiveX constants:
set(FBTYPELIB_NAME PluginGigasoLib)
set(FBTYPELIB_DESC "PluginGigaso 1.0 Type Library")
set(IFBControl_DESC "PluginGigaso Control Interface")
set(FBControl_DESC "PluginGigaso Control Class")
set(IFBComJavascriptObject_DESC "PluginGigaso IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "PluginGigaso ComJavascriptObject Class")
set(IFBComEventSource_DESC "PluginGigaso IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID 26bdf79f-19f2-5ca6-8bf2-a6d0aea68fc4)
set(IFBControl_GUID e452ae29-d985-5f6d-a825-1eb01a7f6abe)
set(FBControl_GUID 37921e23-e7bd-5211-adb9-59eb82bfcdc9)
set(IFBComJavascriptObject_GUID 3927dfce-b7c3-5f63-b757-af3e2913aa2a)
set(FBComJavascriptObject_GUID 58cd8008-78e1-554e-9ff7-fb467576b3d3)
set(IFBComEventSource_GUID 8daae601-62e9-5870-88bd-644f47cc9118)

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "gigaso.PluginGigaso")
set(MOZILLA_PLUGINID "gigaso.com/PluginGigaso")

# strings
set(FBSTRING_CompanyName "gigaso.com")
set(FBSTRING_FileDescription "Gigaso Desktop Searcher")
set(FBSTRING_PLUGIN_VERSION "1.0.0")
set(FBSTRING_LegalCopyright "Copyright 2011 gigaso.com")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}")
set(FBSTRING_ProductName "Plugin Gigaso")
set(FBSTRING_FileExtents "")
set(FBSTRING_PluginName "Plugin Gigaso")
set(FBSTRING_MIMEType "application/x-plugingigaso")

# Uncomment this next line if you're not planning on your plugin doing
# any drawing:

# set (FB_GUI_DISABLED 1)

# Mac plugin settings. If your plugin does not draw, set these all to 0
set(FBMAC_USE_QUICKDRAW 0)
set(FBMAC_USE_CARBON 1)
set(FBMAC_USE_COCOA 1)
set(FBMAC_USE_COREGRAPHICS 1)
set(FBMAC_USE_COREANIMATION 0)

# If you want to register per-machine on Windows, uncomment this line
#set (FB_ATLREG_MACHINEWIDE 1)
