#############################################################################
# Makefile for building: InterLock
# Generated by qmake (3.1) (Qt 5.9.2)
# Project:  InterLock.pro
# Template: app
# Command: F:\QT\Qt5.9.2\5.9.2\msvc2015_64\bin\qmake.exe -o Makefile InterLock.pro -spec win32-msvc "CONFIG+=debug" "CONFIG+=qml_debug"
#############################################################################

MAKEFILE      = Makefile

first: debug
install: debug-install
uninstall: debug-uninstall
QMAKE         = F:\QT\Qt5.9.2\5.9.2\msvc2015_64\bin\qmake.exe
DEL_FILE      = del
CHK_DIR_EXISTS= if not exist
MKDIR         = mkdir
COPY          = copy /y
COPY_FILE     = copy /y
COPY_DIR      = xcopy /s /q /y /i
INSTALL_FILE  = copy /y
INSTALL_PROGRAM = copy /y
INSTALL_DIR   = xcopy /s /q /y /i
QINSTALL      = F:\QT\Qt5.9.2\5.9.2\msvc2015_64\bin\qmake.exe -install qinstall
QINSTALL_PROGRAM = F:\QT\Qt5.9.2\5.9.2\msvc2015_64\bin\qmake.exe -install qinstall -exe
DEL_FILE      = del
SYMLINK       = $(QMAKE) -install ln -f -s
DEL_DIR       = rmdir
MOVE          = move
SUBTARGETS    =  \
		debug \
		release


debug: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Debug
debug-make_first: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Debug 
debug-all: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Debug all
debug-clean: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Debug clean
debug-distclean: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Debug distclean
debug-install: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Debug install
debug-uninstall: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Debug uninstall
release: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Release
release-make_first: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Release 
release-all: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Release all
release-clean: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Release clean
release-distclean: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Release distclean
release-install: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Release install
release-uninstall: FORCE
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Release uninstall

Makefile: InterLock.pro F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\win32-msvc\qmake.conf F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\spec_pre.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\common\angle.conf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\common\msvc-desktop.conf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\qconfig.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3danimation.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3danimation_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dcore.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dcore_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dextras.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dextras_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dinput.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dinput_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dlogic.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dlogic_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquick.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquick_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickanimation.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickanimation_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickextras.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickextras_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickinput.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickinput_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickrender.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickrender_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickscene2d.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickscene2d_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3drender.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3drender_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_accessibility_support_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axbase.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axbase_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axcontainer.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axcontainer_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axserver.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axserver_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_bluetooth.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_bluetooth_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_bootstrap_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_concurrent.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_concurrent_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_core.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_core_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_dbus.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_dbus_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_designer.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_designer_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_designercomponents_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_devicediscovery_support_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_egl_support_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_eventdispatcher_support_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_fb_support_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_fontdatabase_support_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_gamepad.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_gamepad_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_gui.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_gui_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_help.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_help_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_location.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_location_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_multimedia.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_multimedia_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_multimediawidgets.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_multimediawidgets_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_network.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_network_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_nfc.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_nfc_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_opengl.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_opengl_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_openglextensions.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_openglextensions_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_packetprotocol_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_platformcompositor_support_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_positioning.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_positioning_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_printsupport.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_printsupport_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qml.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qml_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qmldebug_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qmldevtools_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qmltest.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qmltest_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qtmultimediaquicktools_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quick.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quick_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickcontrols2.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickcontrols2_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickparticles_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quicktemplates2_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickwidgets.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickwidgets_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_scxml.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_scxml_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_sensors.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_sensors_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_serialbus.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_serialbus_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_serialport.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_serialport_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_sql.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_sql_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_svg.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_svg_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_testlib.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_testlib_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_texttospeech.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_texttospeech_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_theme_support_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_uiplugin.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_uitools.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_uitools_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_webchannel.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_webchannel_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_websockets.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_websockets_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_webview.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_webview_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_widgets.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_widgets_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_winextras.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_winextras_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_xml.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_xml_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_xmlpatterns.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_xmlpatterns_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_zlib_private.pri \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qt_functions.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qt_config.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\win32-msvc\qmake.conf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\spec_post.prf \
		.qmake.stash \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\exclusive_builds.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\common\msvc-version.conf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\toolchain.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\default_pre.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\win32\default_pre.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\resolve_config.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\exclusive_builds_post.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\default_post.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qml_debug.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\precompile_header.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\warn_on.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qt.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\resources.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\moc.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\win32\opengl.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\uic.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qmake_use.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\file_copies.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\win32\windows.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\testcase_targets.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\exceptions.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\yacc.prf \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\lex.prf \
		InterLock.pro \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\qtmaind.prl \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Widgets.prl \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Gui.prl \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Network.prl \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Sql.prl \
		F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Core.prl
	$(QMAKE) -o Makefile InterLock.pro -spec win32-msvc "CONFIG+=debug" "CONFIG+=qml_debug"
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\spec_pre.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\common\angle.conf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\common\msvc-desktop.conf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\qconfig.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3danimation.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3danimation_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dcore.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dcore_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dextras.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dextras_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dinput.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dinput_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dlogic.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dlogic_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquick.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquick_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickanimation.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickanimation_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickextras.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickextras_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickinput.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickinput_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickrender.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickrender_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickscene2d.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3dquickscene2d_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3drender.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_3drender_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_accessibility_support_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axbase.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axbase_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axcontainer.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axcontainer_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axserver.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_axserver_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_bluetooth.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_bluetooth_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_bootstrap_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_concurrent.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_concurrent_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_core.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_core_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_dbus.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_dbus_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_designer.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_designer_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_designercomponents_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_devicediscovery_support_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_egl_support_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_eventdispatcher_support_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_fb_support_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_fontdatabase_support_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_gamepad.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_gamepad_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_gui.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_gui_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_help.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_help_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_location.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_location_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_multimedia.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_multimedia_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_multimediawidgets.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_multimediawidgets_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_network.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_network_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_nfc.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_nfc_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_opengl.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_opengl_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_openglextensions.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_openglextensions_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_packetprotocol_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_platformcompositor_support_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_positioning.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_positioning_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_printsupport.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_printsupport_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qml.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qml_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qmldebug_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qmldevtools_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qmltest.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qmltest_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_qtmultimediaquicktools_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quick.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quick_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickcontrols2.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickcontrols2_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickparticles_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quicktemplates2_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickwidgets.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_quickwidgets_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_scxml.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_scxml_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_sensors.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_sensors_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_serialbus.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_serialbus_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_serialport.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_serialport_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_sql.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_sql_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_svg.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_svg_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_testlib.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_testlib_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_texttospeech.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_texttospeech_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_theme_support_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_uiplugin.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_uitools.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_uitools_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_webchannel.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_webchannel_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_websockets.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_websockets_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_webview.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_webview_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_widgets.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_widgets_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_winextras.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_winextras_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_xml.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_xml_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_xmlpatterns.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_xmlpatterns_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\modules\qt_lib_zlib_private.pri:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qt_functions.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qt_config.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\win32-msvc\qmake.conf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\spec_post.prf:
.qmake.stash:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\exclusive_builds.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\common\msvc-version.conf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\toolchain.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\default_pre.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\win32\default_pre.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\resolve_config.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\exclusive_builds_post.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\default_post.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qml_debug.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\precompile_header.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\warn_on.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qt.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\resources.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\moc.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\win32\opengl.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\uic.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\qmake_use.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\file_copies.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\win32\windows.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\testcase_targets.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\exceptions.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\yacc.prf:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\mkspecs\features\lex.prf:
InterLock.pro:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\qtmaind.prl:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Widgets.prl:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Gui.prl:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Network.prl:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Sql.prl:
F:\QT\Qt5.9.2\5.9.2\msvc2015_64\lib\Qt5Core.prl:
qmake: FORCE
	@$(QMAKE) -o Makefile InterLock.pro -spec win32-msvc "CONFIG+=debug" "CONFIG+=qml_debug"

qmake_all: FORCE

make_first: debug-make_first release-make_first  FORCE
all: debug-all release-all  FORCE
clean: debug-clean release-clean  FORCE
	-$(DEL_FILE) InterLock.exp
	-$(DEL_FILE) InterLock.vc.pdb
	-$(DEL_FILE) InterLock.ilk
	-$(DEL_FILE) InterLock.idb
distclean: debug-distclean release-distclean  FORCE
	-$(DEL_FILE) Makefile
	-$(DEL_FILE) .qmake.stash InterLock.lib InterLock.pdb

debug-mocclean:
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Debug mocclean
release-mocclean:
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Release mocclean
mocclean: debug-mocclean release-mocclean

debug-mocables:
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Debug mocables
release-mocables:
	@set MAKEFLAGS=$(MAKEFLAGS)
	$(MAKE) -f $(MAKEFILE).Release mocables
mocables: debug-mocables release-mocables

check: first

benchmark: first
FORCE:

$(MAKEFILE).Debug: Makefile
$(MAKEFILE).Release: Makefile
