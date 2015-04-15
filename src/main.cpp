#include "stdafx.h"
#include "IRender.h"
#include "IInput.h"
#include "ISystem.h"
#include "ISound.h"
#include "ILogger.h"
#include "Game.h"
#include "Common.h"


#ifdef ALLEGRO_ANDROID
#include <allegro5/allegro_android.h>
#endif

  // We only use Google Breakpad on Windows
#if defined(WIN32) && !defined(_DEBUG)
	#define USE_BREAKPAD
	#include "exception_handler.h"
	#include "client/windows/sender/crash_report_sender.h"
#endif

#include <cctype>

#ifdef USE_BREAKPAD
  // Callback for sending reports
bool DmpCallback(const wchar_t* dump_path,
	const wchar_t* minidump_id,
	void* context,
	EXCEPTION_POINTERS* exinfo,
	MDRawAssertionInfo* assertion,
	bool succeeded)
{
	google_breakpad::CrashReportSender sender( L"crash.checkpoint" );
	sender.set_max_reports_per_day( 5 );
	std::map<std::wstring, std::wstring> params;
	std::wstring filename = dump_path;
	filename += L"\\";
	filename += minidump_id;
	filename += L".dmp";
	google_breakpad::ReportResult r = sender.SendCrashReport( L"http://your-website.com/crash/post", params, filename, 0 );

	if ( r == google_breakpad::RESULT_SUCCEEDED )
		MessageBox( 0, "Crash report was sent. Thank you!", "Crash report", MB_OK|MB_ICONINFORMATION );
	else
		MessageBox( 0, "Could not send crash report. Thank you for trying, though! Please report this crash at http:\\\\your-website.com", "Crash report", MB_OK|MB_ICONWARNING );

	return false;
}

  // Callback that gets called when the game crashes. It should ask the user to send the report to our server
  // If user refuses, do nothing.
bool DmpFilter( void* context, EXCEPTION_POINTERS* exinfo,
	MDRawAssertionInfo* assertion)
{
	const char *msg = "The game has crashed. Do you want to send a report to developer?\r\n\r\n"
		"If this is a first time you encounter this crash, please allow crash report to be sent, it would help me to find and fix it sooner.\r\n\r\n"
		"Crash reports contain no identifying information about your computer.";
	if ( MessageBox( 0, msg, "Crash detected", MB_YESNO|MB_ICONERROR ) != IDYES )
		return false;

	return true;
}

#endif

#ifdef WIN32
int main()
#else
int main(int argc, char **argv) 
#endif
{
#ifdef USE_BREAKPAD
	google_breakpad::ExceptionHandler *pHandler = new google_breakpad::ExceptionHandler( L"dumps\\", DmpFilter, DmpCallback, 0, google_breakpad::ExceptionHandler::HANDLER_ALL, MiniDumpNormal, L"", 0 );
#endif

	srand( time( 0 ) );

	ISystem & sys = GetSystem();
	if ( !sys.Init( "Return of Dr. Destructo", "game.log" ) )
		return -1;

#ifdef ALLEGRO_ANDROID
	al_android_set_apk_file_interface();
#endif

	sys.SetConfigName( "dd.cfg" );
	sys.LoadConfig();

	++g_options_version;

	ISound & sound = GetSound();
	sound.Init();

	const int masterVolume = sys.HasConfigValue( "DD_Sound", "Master" ) ? sys.GetConfigValue_Int( "DD_Sound", "Master" ) : 100;
	const int soundVolume = sys.HasConfigValue( "DD_Sound", "Sound" ) ? sys.GetConfigValue_Int( "DD_Sound", "Sound" ) : 100;
	const int musicVolume = sys.HasConfigValue( "DD_Sound", "Music" ) ? sys.GetConfigValue_Int( "DD_Sound", "Music" ) : 100;


	sound.SetMasterVolume( masterVolume / 100.0f );
	sound.SetVolumeByType( ISample::TYPE_SOUND, soundVolume / 100.0f );
	sound.SetVolumeByType( ISample::TYPE_MUSIC, musicVolume / 100.0f );

	int cfgW	= sys.GetConfigValue_Int( "DD_Graphics", "Width" );
	int cfgH	= sys.GetConfigValue_Int( "DD_Graphics", "Height" );
	int cfgB	= sys.GetConfigValue_Int( "DD_Graphics", "Depth" );
	int scaling	= sys.GetConfigValue_Int( "DD_Graphics", "Scaling" );
	if ( scaling == 0 )
		scaling = SCALING_KEEP_ASPECT;
	bool fullscreen = sys.GetConfigValue_Int( "DD_Graphics", "Fullscreen" );
	bool vsync = sys.HasConfigValue( "DD_Graphics", "VSync" ) ? sys.GetConfigValue_Int( "DD_Graphics", "VSync" ) : false;

	if ( cfgW <= 0 ) cfgW = 1280;
	if ( cfgH <= 0 ) cfgH = 960;
	if ( cfgB <= 0 ) cfgB = 32;
	//cfgW = 320;
	//cfgH = 200;
	
	IRender & render = GetRender();

	int renderType = sys.GetConfigValue_Int( "DD_Graphics", "RenderMode" );
	ERenderType realType = renderType == 0 ? RENDERTYPE_DIRECT3D : RENDERTYPE_OPENGL;
	if ( !render.Init( realType, cfgW, cfgH, cfgB, fullscreen, vsync, "Data/Fonts/111-catatan-perjalanan.otf" ) )
		return -1;
	render.SetGlobalScaling( (EScalingMode)scaling, SCREEN_W, SCREEN_H );

	IInput & input = GetInput();
	if ( !input.Init( true, true, true, true ) )
		return -1;

	sys.SetWindowTitle( "Return of Dr. Destructo v1.0" );

	Game game;
	game.Run();

	render.Clear();	

	return 0;
}
