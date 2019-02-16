
static const scandef scandefs[] = {
	 { "mode_main"    ,' ' }
	,{ "mode_program" ,'p' }
	,{ "mode_effects" ,'e' }
	,{ "mode_xlink"   ,'x' }
	,{ "mode_save"    ,'s' }
	,{ "mode_load"    ,'l' }
	,{ "mode_quit"    ,'q' }
	,{ "mode_trackmute",'t' }
	,{ "mode_step"     ,'z' }
	,{ "mode_waveforms",'w' }
	,{ "mode_global"   ,'g' }
	,{ "mode_record"   ,'r' }

	,{ "transport_erase"    ,CHAR_DEL }
	,{ "transport_repeat"   ,'=' }
	,{ "transport_full"     ,'f' }
	,{ "transport_sixteen"  ,'y' }
	,{ "transport_overdub"  ,'v' }
	,{ "transport_stop"     ,'b' }
	,{ "transport_play"     ,'n' }
	,{ "transport_playstart",'m' }
	,{ "transport_undo"     ,'u' }
	,{ "transport_tap"      ,'\\' }

	,{ "data_inc"           ,'.' }
	,{ "data_dec"           ,',' }
	,{ "data_open"          ,'o' }
	,{ "data_enter"         ,CHAR_ENTER }

	,{ "soft_f1"            ,CHAR_F1 }
	,{ "soft_f2"            ,CHAR_F2 }
	,{ "soft_f3"            ,CHAR_F3 }
	,{ "soft_f4"            ,CHAR_F4 }
	,{ "soft_f5"            ,CHAR_F5 }
	,{ "soft_f6"            ,CHAR_F6 }

	,{ "bank_a"             ,CHAR_F9 }
	,{ "bank_b"             ,CHAR_F10 }
	,{ "bank_c"             ,CHAR_F11 }
	,{ "bank_d"             ,CHAR_F12 }

	,{ "cursor_up"          ,CHAR_CURSOR_UP }
	,{ "cursor_down"        ,CHAR_CURSOR_DOWN }
	,{ "cursor_left"        ,CHAR_CURSOR_LEFT }
	,{ "cursor_right"       ,CHAR_CURSOR_RIGHT }

};

void Console::queueWheel( const int amt )
{
	INPUT_RECORD ir;
	DWORD events_written;
	ir.EventType = KEY_EVENT;
	if ( amt > 0 ) {
		ir.Event.KeyEvent.bKeyDown = 1;
		ir.Event.KeyEvent.dwControlKeyState = 0;
		ir.Event.KeyEvent.uChar.AsciiChar = '.';
		ir.Event.KeyEvent.wRepeatCount = amt;
		ir.Event.KeyEvent.wVirtualKeyCode = KEY_WHEEL_UP;
	} else {
		ir.Event.KeyEvent.bKeyDown = 1;
		ir.Event.KeyEvent.dwControlKeyState = 0;
		ir.Event.KeyEvent.uChar.AsciiChar = ',';
		ir.Event.KeyEvent.wRepeatCount = abs(amt);
		ir.Event.KeyEvent.wVirtualKeyCode = KEY_WHEEL_DOWN;
	}
	WriteConsoleInput( m_hStdin, &ir, 1, &events_written );
}
void Console::loadKeys( const char* const fn )
{
	char rawbuf[ 128 ];
	char buf[128];

	FILE* const f = fopen( fn, "r" );
MY_ASSERT(f);
	if ( !f ) {
		char cwd[ 128 ];
		_getcwd( cwd, 128 );
		printf( "fatal: loadkeys failed for %s\n", fn );
		printf( "       cwd is %s\n", cwd );
		exit(1);//XXX ok for now, something cleaner later
	}

	std::fill( m_scan_to_char.begin(), m_scan_to_char.end(), 0 );
	m_scan_to_char[ KEY_MIDI_TRIGGER ] = CHAR_MIDI_TRIGGER;
	m_scan_to_char[ KEY_PAD_PLAY_PAD ] = CHAR_PAD_PLAY_PAD;
	m_scan_to_char[ KEY_CC_LEVEL     ] = CHAR_CC_LEVEL;
	m_scan_to_char[ KEY_CC_SEND      ] = CHAR_CC_SEND;
	m_scan_to_char[ KEY_CC_PAN       ] = CHAR_CC_PAN;
	m_scan_to_char[ KEY_WHEEL_UP     ] = '.';
	m_scan_to_char[ KEY_WHEEL_DOWN   ] = ',';



	while ( !feof(f) ) {
		fgets( rawbuf, 127, f );

		if ( rawbuf[strlen(rawbuf)-1] == 10 ) rawbuf[ strlen(rawbuf)-1 ] = 0; 

		const int rawlen = (int)strlen(rawbuf);
		for ( int i=0; i<rawlen; i++ ) {
			switch ( rawbuf[i] ) {
				case '#' :
				case ';' : rawbuf[i] = 0; break;
			}
		}

		str_trim_copy( buf, rawbuf );
		const int buflen = strlen( buf );

		if ( buflen == 0 ) continue;
		
		int marked = 0;
		char *lp = &buf[0];
		char *rp = NULL;
		for ( int i=0; i<buflen; i++ ) {
			if ( buf[i] == ' ' ) {
				marked++;
				buf[i] = 0;
				rp = &buf[i]+1;
			}
		}


		int scandefcnt = sizeof(scandefs) / sizeof(struct scandef);
		for ( int i=0; i<scandefcnt; i++ ) {
			int hexval;
			sscanf( rp, "%04x", &hexval );
			if ( strcmp(lp,scandefs[i].txt)== 0 ) m_scan_to_char[ hexval ] = scandefs[i].val;
		}
	}
}

/*
#define KEY_PAD_PLAY_LOOP 0x1230
#define KEY_PAD_PLAY_TO   0x1231
#define KEY_PAD_PLAY_FROM 0x1232
#define KEY_PAD_PLAY_PLAY 0x1233
#define KEY_PAD_PLAY_PAD  0x1234
#define KEY_MIDI_TRIGGER  0x1235
#define KEY_CC_LEVEL      0x1236
#define KEY_CC_SEND       0x1237
#define KEY_CC_PAN        0x1238

#define KEY_WHEEL_UP 0x1239
#define KEY_WHEEL_DOWN 0x123a
*/

/*
#define CHAR_CURSOR_LEFT  -2
#define CHAR_CURSOR_UP    -3
#define CHAR_CURSOR_RIGHT -4
#define CHAR_CURSOR_DOWN  -5

#define CHAR_BACKSPACE -6
#define CHAR_ENTER -7
#define CHAR_DEL -8

#define CHAR_F1 -9
#define CHAR_F2 -10
#define CHAR_F3 -11
#define CHAR_F4 -12
#define CHAR_F5 -13
#define CHAR_F6 -14
#define CHAR_F7 -15
#define CHAR_F8 -16
#define CHAR_F9 -17
#define CHAR_F10 -18
#define CHAR_F11 -19
#define CHAR_F12 -20
*/
/*
#define CHAR_PAD0 -12
#define CHAR_PAD1 -13
#define CHAR_PAD2 -14
#define CHAR_PAD3 -15
#define CHAR_PAD4 -16
#define CHAR_PAD5 -17
#define CHAR_PAD6 -18
#define CHAR_PAD7 -19
#define CHAR_PAD8 -20
#define CHAR_PAD9 -21
#define CHAR_PADA -22
#define CHAR_PADB -23
#define CHAR_PADC -24
#define CHAR_PADD -25
#define CHAR_PADE -26
#define CHAR_PADF -27
*/

/*
#define CHAR_PAD_PLAY_PAD -21
#define CHAR_MIDI_TRIGGER -22
#define CHAR_CC_LEVEL -23
#define CHAR_CC_SEND -24
#define CHAR_CC_PAN -25
*/

//#define CURSOR_LEFT  0x25
//#define CURSOR_UP    0x26
//#define CURSOR_RIGHT 0x27
//#define CURSOR_DOWN  0x28

//#define BACKSPACE 0x08
//#define ENTER 0x0d
//#define DEL 0x2e

#define KEY_V 0x56
#define KEY_B 0x42
#define KEY_N 0x4e
#define KEY_M 0x4d


/*
struct TextMap {
	int width;
	int height;
	std::vector<CHAR_INFO> buf; }



struct scandef {
	char *txt;
	int val;
};
*/




DWORD Console::countEventsWaiting() const
{
	DWORD x;
	GetNumberOfConsoleInputEvents( d_stdin, &x );
	return x;
}

/*
void Console::writexy( const int x, const int y, const char* const str )
{
	DWORD written;
	COORD dest;
	dest.X = m_ox+x;
	dest.Y = m_oy+y;

	WriteConsoleOutputCharacter( d_stdout, str, strlen(str), dest, &written );
}
*/


/*
void Console::xyprintf( const int x, const int y, const char* const format, ... )
{
	va_list args;
	char buf[ 128 ];
	DWORD written;
	COORD dest;
	dest.X = m_ox+x;
	dest.Y = m_oy+y;

	va_start( args, format );
	vsprintf( buf, format, args );

	WriteConsoleOutputCharacter( d_stdout, buf, strlen(buf), dest, &written );
}
*/

void Console::WriteXY(const int x, const int y, const std::string& str, const char att) {
	thread_local vector<CHAR_INFO> buf;

	buf.clear();
	forn(i, sz(str)) {
		buf.emplace_back(CHAR_INFO{ s[i], att }); }

	const auto bufDim = COORD{ sz(str), 1 };
	const auto bufOrigin = COORD{ 0, 0 };

	SMALL_RECT destRect;
	destRect.Left = m_ox + x;                destRect.Top = m_oy + y;
	destRect.Right = m_ox + x + sz(str) - 1; destRect.Bottom = m_oy + y;
	WriteConsoleOutput(d_stdout, tmp.data(), bufDim, bufOrigin, &destRect); }


int Console::getEvent( const bool wait_for_real_key )
{
	INPUT_RECORD ibuf;
	DWORD events_read;
	int rtn;
	while ( 1 ) {
	rtn = ReadConsoleInput( d_stdin, &ibuf, 1, &events_read );
	if ( rtn == 0 ) return -1;
	if ( ibuf.EventType == KEY_EVENT ) {
		std::stringstream s;
		s << "lastkey:" << boost::format("%04x") % ibuf.Event.KeyEvent.wVirtualKeyCode;
		this->writexy( 67, 23, s.str() );
		m_activekeys[ ibuf.Event.KeyEvent.wVirtualKeyCode ] = ibuf.Event.KeyEvent.bKeyDown;
		m_last_repeat = ibuf.Event.KeyEvent.wRepeatCount;
/*		printf( "%d % 4d %04x %04x [%c] %08x\n"
			   ,ibuf.Event.KeyEvent.bKeyDown
			   ,ibuf.Event.KeyEvent.wRepeatCount
			   ,ibuf.Event.KeyEvent.wVirtualKeyCode
			   ,ibuf.Event.KeyEvent.wVirtualScanCode
			   ,ibuf.Event.KeyEvent.uChar.AsciiChar
			   ,ibuf.Event.KeyEvent.dwControlKeyState );*/
		if ( ibuf.Event.KeyEvent.bKeyDown ) {
			int cfg_val = m_scan_to_char[ ibuf.Event.KeyEvent.wVirtualKeyCode ];
			if ( cfg_val == 0 ) {
				return ibuf.Event.KeyEvent.uChar.AsciiChar;
			} else {
				return cfg_val;
			}
/*
			switch ( ibuf.Event.KeyEvent.wVirtualKeyCode ) {
				case CURSOR_LEFT  : return CHAR_CURSOR_LEFT; break;
				case CURSOR_UP    : return CHAR_CURSOR_UP; break;
				case CURSOR_RIGHT : return CHAR_CURSOR_RIGHT; break;
				case CURSOR_DOWN  : return CHAR_CURSOR_DOWN; break;
				case BACKSPACE : return CHAR_BACKSPACE; break;
				case ENTER : return CHAR_ENTER; break;
				case DEL : return CHAR_DEL; break;
				case KF1 : return CHAR_F1; break;
				case KF2 : return CHAR_F2; break;
				case KF3 : return CHAR_F3; break;
				case KF4 : return CHAR_F4; break;
				case KF5 : return CHAR_F5; break;
				case KF6 : return CHAR_F6; break;
				case KF7 : return CHAR_F7; break;
				case KF8 : return CHAR_F8; break;
				case KF9 : return CHAR_F9; break;
				case KF10 : return CHAR_F10; break;
				case KF11 : return CHAR_F11; break;
				case KF12 : return CHAR_F12; break;
				case KEY_PAD_PLAY_PAD : return CHAR_PAD_PLAY_PAD; break;
				case KEY_MIDI_TRIGGER : return CHAR_MIDI_TRIGGER; break;
				case KEY_NV : return CHAR_NV; break;

				default : return ibuf.Event.KeyEvent.uChar.AsciiChar;
			}
*/
		}
//		else {
//		  return -1;
//		}
	}
      if ( wait_for_real_key == false ) break;
	}
	return -1;
}


void Console::spinUntilKeyEvent(int& keyDown, int& ch, int& shifted) {
	while (1) {
		auto result = getKeyEvent(keyDown, ch, shifted);
		if (result == 1) {
			return;}}}


int Console::getKeyEvent( int& keydown, int& ch, int& shifted) {
	INPUT_RECORD ibuf;
	DWORD events_read;

	const int rtn = ReadConsoleInput( d_stdin, &ibuf, 1, &events_read );

	if (rtn == 0) {
		return -1; }  // failure XXX check with GetLastError

	if (ibuf.EventType == KEY_EVENT ) {

		std::stringstream s;
		s << "lastkey:" << boost::format("%04x") % ibuf.Event.KeyEvent.wVirtualKeyCode;
		writexy( 67, 23, s.str() );
		
		// update key state data
		m_activekeys[ibuf.Event.KeyEvent.wVirtualKeyCode] = ibuf.Event.KeyEvent.bKeyDown;
		m_last_repeat = ibuf.Event.KeyEvent.wRepeatCount;
		keydown = ibuf.Event.KeyEvent.bKeyDown;
		shifted = m_activekeys[0x0010];

		// remap?
		const int cfg_val = m_scan_to_char[ ibuf.Event.KeyEvent.wVirtualKeyCode ];
		if (cfg_val == -1) {
			ch = ibuf.Event.KeyEvent.uChar.AsciiChar; }
		else {
			ch = cfg_val; }
		return 1; }
	return 0;
}

void Console::GotoXY(int x, int y) {
	pos = ivec2{x,y}; }

void 

void Console::fillColor( const int x, const int y, const int width, const WORD att )
{
	DWORD written;
	COORD pos;
	pos.X = m_ox+x;
	pos.Y = m_oy+y;
	FillConsoleOutputAttribute(d_stdout, att, width, pos, &written );
}

void Console::fillChar( const int x, const int y, const int width, const char ch )
{
	DWORD written;
	COORD pos;
	pos.X = m_ox+x;
	pos.Y = m_oy+y;
	FillConsoleOutputCharacter(d_stdout, ch, width, pos, &written );
}



void boxline( char* const buf, const int width, const int l, const int m, const int r )
{
	buf[0] = l;
	for ( int i=1; i<width-1; i++ ) buf[i] = m;
	buf[width-1] = r;
	buf[width] = 0;
}


void Console::box( const int x, const int y, const int width, const int height, const char* const title ) {
	char buf[128];

	int i = 0;
    boxline( buf, width, BOX_1_UL, BOX_1_HL, BOX_1_UR );
	writexy( x, y+i, buf );
	fillColor( x, y+i, width, COLOR_NORMAL );

	for ( i=1; i<height-1; i++ ) {
		boxline( buf, width, BOX_1_VL, ' ', BOX_1_VL );
		writexy( x, y+i, buf );
		fillColor( x, y+i, width, COLOR_NORMAL );
	}

	boxline( buf, width, BOX_1_LL, BOX_1_HL, BOX_1_LR );
	writexy( x, y+i, buf );
	fillColor( x, y+i, width, COLOR_NORMAL );

	if ( title ){
		buf[0] = BOX_1_TL;
		buf[1] = ' ';
		i = 2;
		for ( i=0; title[i]; i++ ) {
			buf[2+i] = title[i];
		}
		buf[2+i] = ' ';
		buf[2+i+1] = BOX_1_TR;
		buf[2+i+2] = 0;
		writexy( x+1, y, buf );
	}
}


void Console::push() {
	COORD bufsiz;
	COORD bufpos;
	SMALL_RECT readregion;

	readregion.Top    = 3;
	readregion.Bottom = 22;
	readregion.Left   = 0;
	readregion.Right  = 79;

	bufsiz.X = 80;
	bufsiz.Y = 25;

	bufpos.X = 0;
	bufpos.Y = 0;

	ReadConsoleOutput( d_stdout, m_pushbuf[m_pushcnt++].data(), bufsiz, bufpos, &readregion ); }


void Console::pop() {
	COORD bufsiz;
	COORD bufpos;
	SMALL_RECT readregion;

	readregion.Top    = 3;
	readregion.Bottom = 22;
	readregion.Left   = 0;
	readregion.Right  = 79;

	bufsiz.X = 80;
	bufsiz.Y = 25;

	bufpos.X = 0;
	bufpos.Y = 0;

	WriteConsoleOutput( d_stdout, m_pushbuf[--m_pushcnt].data(), bufsiz, bufpos, &readregion ); }



void Console::queueKey( const int pressed, const int repeat, const int vkc, const int vsc, const char ascii, const int ctrl ) {
	INPUT_RECORD ir;
	DWORD events_written;
	ir.EventType = KEY_EVENT;
	ir.Event.KeyEvent.bKeyDown = pressed;
	ir.Event.KeyEvent.dwControlKeyState = ctrl;
	ir.Event.KeyEvent.uChar.AsciiChar = ascii;
	ir.Event.KeyEvent.wRepeatCount = repeat;
	ir.Event.KeyEvent.wVirtualKeyCode = vkc;
	WriteConsoleInput( d_stdin, &ir, 1, &events_written ); }

