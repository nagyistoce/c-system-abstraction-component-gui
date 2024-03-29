#define NO_LOGGING
#define KEYS_DEFINED
#include <stdhdrs.h>
#include "consolestruc.h"
#include <keybrd.h>
#include <input.h> // RecallCommand
// included to have pending struct available to pass to
// command line update....
#include "WinLogic.h"

extern int myTypeID;

enum{
   KS_DELETE
};

static void CPROC _Key_KeystrokePaste( PCONSOLE_INFO pmdp )
{
	if( pmdp->KeystrokePaste )
      pmdp->KeystrokePaste( pmdp );
}

DECLTEXT( KeyStroke, WIDE("\x7f") );
//static TEXT KeyStroke[] = { DEFTEXT( WIDE("\x7f") )
//                    };

#if defined( GCC ) || defined( __LINUX__ )
CORECON_EXPORT( PSIKEYDEFINE, KeyDefs[256] ) = { [KEY_BACKSPACE]={WIDE("back"),WIDE("backspace"),0,{{KEYDATA}} }
                      , [KEY_TAB]={WIDE("tab"),0,0,{{KEYDATA}} }
                      , [KEY_ENTER]={WIDE("return"), WIDE("enter"),0,{{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}} }
                      //, [KEY_PAUSE]={WIDE("pause"),0,KDF_NODEFINE }
                      , [KEY_ESCAPE]={WIDE("esc"), WIDE("escape"), 0, {{KEYDATA}}} // 0x1b
                      , [KEY_SPACE]={WIDE("space"), WIDE("blank"), 0, {{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}} } //0x20
                      , [KEY_PGUP]={WIDE("prior"), WIDE("pgup"), 0, {{HISTORYKEY,(PTEXT)HistoryPageUp}} }
                      , [KEY_PGDN]={WIDE("next"), WIDE("pgdn"), 0, {{HISTORYKEY,(PTEXT)HistoryPageDown}} }
                      , [KEY_END]={WIDE("end"), 0, 0, {{COMMANDKEY, (PTEXT)KeyEndCmd}}}
                      , [KEY_HOME]={WIDE("home"), 0, 0, {{COMMANDKEY, (PTEXT)KeyHome}}}
                      , [KEY_LEFT]={WIDE("left"), 0, 0, {{COMMANDKEY, (PTEXT)KeyLeft}}}

                      , [KEY_UP]={WIDE("up") , 0, 0, { {COMMANDKEY, (PTEXT)KeyUp}
                                       , {HISTORYKEY, (PTEXT)HistoryLineUp}}}
                      , [KEY_RIGHT]={WIDE("right"), 0, 0, {{COMMANDKEY, (PTEXT)KeyRight}}}
                      , [KEY_DOWN]={WIDE("down"), 0, 0, {{COMMANDKEY, (PTEXT)HandleKeyDown}
                                       , {HISTORYKEY, (PTEXT)HistoryLineDown}}}
                      //, {WIDE("select")}
                      //, [KEY_PRINT]={WIDE("print")}
                      //, {WIDE("execute")}
                      //, {WIDE("snapshot")}
                      , [KEY_GREY_INSERT]={WIDE("insert"), 0, 0, {{COMMANDKEY, (PTEXT)KeyInsert}}}
                      , [KEY_GREY_DELETE]={WIDE("delete"), 0, 0, {{KEYDATA_DEFINED, &KeyStroke+KS_DELETE}}}
                      , {WIDE("help")}
                      , [KEY_0]={WIDE("0"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}      //0x30
                      , [KEY_1]={WIDE("1"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , [KEY_2]={WIDE("2"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}
                                    ,{KEYDATA}}}
                      , [KEY_3]={WIDE("3"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , [KEY_4]={WIDE("4"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , [KEY_5]={WIDE("5"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , [KEY_6]={WIDE("6"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , [KEY_7]={WIDE("7"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , [KEY_8]={WIDE("8"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , [KEY_9]={WIDE("9"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}

/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */
                      //..... to hmm 39
                      , [KEY_A] = {WIDE("a"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}} // 0x41 'A'
                      , [KEY_B] = {WIDE("b"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_C] = {WIDE("c"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_D] = {WIDE("d"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_E] = {WIDE("e"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_F] = {WIDE("f"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_G] = {WIDE("g"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_H] = {WIDE("h"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_I] = {WIDE("i"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_J] = {WIDE("j"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_K] = {WIDE("k"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_L] = {WIDE("l"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_M] = {WIDE("m"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_N] = {WIDE("n"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_O] = {WIDE("o"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_P] = {WIDE("p"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_Q] = {WIDE("q"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_R] = {WIDE("r"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_S] = {WIDE("s"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_T] = {WIDE("t"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_U] = {WIDE("u"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_V] = {WIDE("v"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}
                                           ,{0}
                                           ,{SPECIALKEY,(PTEXT)_Key_KeystrokePaste}}}
                      , [KEY_W] = {WIDE("w"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_X] = {WIDE("x"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_Y] = {WIDE("y"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_Z] = {WIDE("z"),0,KDF_NODEFINE,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , [KEY_PAD_0] = {WIDE("num0"), WIDE("pad0"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_1] = {WIDE("num1"), WIDE("pad1"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_2] = {WIDE("num2"), WIDE("pad2"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_3] = {WIDE("num3"), WIDE("pad3"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_4] = {WIDE("num4"), WIDE("pad4"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_5] = {WIDE("num5"), WIDE("pad5"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_6] = {WIDE("num6"), WIDE("pad6"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_7] = {WIDE("num7"), WIDE("pad7"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_8] = {WIDE("num8"), WIDE("pad8"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_9] = {WIDE("num9"), WIDE("pad9"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_MULT] = {WIDE("mult"), WIDE("mulitply"),KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_PLUS] = {WIDE("add"),0,KDF_NODEFINE,{{KEYDATA} }}
                      , {WIDE("separator"), 0, KDF_NODEFINE }
                      , [KEY_PAD_MINUS] = {WIDE("subtract"),0,KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_DOT] = {WIDE("decimal"),0,KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_PAD_DIV] = {WIDE("divide"),0,KDF_NODEFINE,{{KEYDATA}} }
                      , [KEY_F1]={WIDE("f1") }   // 0x70
                      , [KEY_F2]={WIDE("f2") }
                      , [KEY_F3]={WIDE("f3") }
                      , [KEY_F4]={WIDE("f4") }
                      , [KEY_F5]={WIDE("f5") }
                      , [KEY_F6]={WIDE("f6") }
                      , [KEY_F7]={WIDE("f7") }
                      , [KEY_F8]={WIDE("f8") }
                      , [KEY_F9]={WIDE("f9") }
                      , [KEY_F10]={WIDE("f10") }
                      , [KEY_F11]={WIDE("f11") }
                      , [KEY_F12]={WIDE("f12") }
                      //, {WIDE("numlock"),0,KDF_NODEFINE} // 0x90
                      //, {WIDE("scroll"),0,KDF_NODEFINE}

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
                      //, NONAMES // {WIDE("lshift"),0,KDF_NODEFINE}  // 0xa0
                      //, NONAMES // {WIDE("rshift"),0,KDF_NODEFINE}
                      //, NONAMES // {WIDE("lctrl"), WIDE("lcontrol"),0,KDF_NODEFINE}
                      //, NONAMES // {WIDE("rctrl"), WIDE("rcontrol"),0,KDF_NODEFINE}
                      //, NONAMES // {WIDE("lmenu"), WIDE("lalt"),0,KDF_NODEFINE }
                      //, NONAMES // {WIDE("rmenu"), WIDE("ralt"),0,KDF_NODEFINE }
                      , [KEY_SEMICOLON]={WIDE(";"), WIDE("semicolon"),0,{{KEYDATA}
                                            ,{KEYDATA}} }
                      , [KEY_EQUAL]={WIDE("="), WIDE("equal"),0    ,{{KEYDATA}
                                            ,{KEYDATA}}}
                      , [KEY_COMMA]={WIDE(","), WIDE("comma"),0    ,{{KEYDATA}
                                            ,{KEYDATA}} }
                      , [KEY_DASH]={WIDE("-"), WIDE("dash"),0     ,{{KEYDATA}
                                            ,{KEYDATA}}}
                      , [KEY_STOP]={WIDE("."), WIDE("period"),0   ,{{KEYDATA}
                                            ,{KEYDATA}}}
                      , [KEY_SLASH]={WIDE("/"),WIDE("slash"),0     ,{{KEYDATA}
                                            ,{KEYDATA}}}
                      , [KEY_ACCENT]={WIDE("`"), WIDE("accent"),0   ,{{KEYDATA}
                                            ,{KEYDATA}} } // 0xc0
                      , [KEY_LEFT_BRACKET]={ WIDE("["), WIDE("lbracket"),0  ,{{KEYDATA}
                                              ,{KEYDATA}} }
                      , [KEY_BACKSLASH]={ WIDE("\\"), WIDE("backslash"),0,{{KEYDATA}
                                              ,{KEYDATA}} }
                      , [KEY_RIGHT_BRACKET]={ WIDE("]"), WIDE("rbracket"),0  ,{{KEYDATA}
                                              ,{KEYDATA}} }
                      , [KEY_QUOTE]={ WIDE("'"), WIDE("quote"),0     ,{{KEYDATA}
                                              ,{KEYDATA}} }
                       };
#else
#define NONAMES {NULL,NULL,0}
CORECON_EXPORT( PSIKEYDEFINE, KeyDefs[] ) = { NONAMES
                      , {WIDE("lbutton"),0,0 }
                      , {WIDE("rbutton"),0,0 }
                      , {WIDE("cancel"),0,0}
                      , {WIDE("mbutton"),0,0} // 0x04
                      , NONAMES, NONAMES, NONAMES
                      , {WIDE("back"),WIDE("backspace"),0,{{KEYDATA}} }
                      , {WIDE("tab"),0,0,{{KEYDATA}} }
                      , NONAMES, NONAMES
                      , {WIDE("clear"),0,0 }   // 0x0c
                      , {WIDE("return"), WIDE("enter"),0,{{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}
                                             ,{KEYDATA}} }
                      , NONAMES, NONAMES
                      , {WIDE("shift"),0,0, {{CONTROLKEY,(PTEXT)KeyShift}
                                                 ,{CONTROLKEY,(PTEXT)KeyShift}
                                                 ,{CONTROLKEY,(PTEXT)KeyShift}
                                                 ,{CONTROLKEY,(PTEXT)KeyShift}
                                                 ,{CONTROLKEY,(PTEXT)KeyShift}
                                                 ,{CONTROLKEY,(PTEXT)KeyShift}
                                                 ,{CONTROLKEY,(PTEXT)KeyShift}
                                                 ,{CONTROLKEY,(PTEXT)KeyShift}} } // 0x10
                      , {WIDE("control"),0,0, {{CONTROLKEY,(PTEXT)KeyControl}
                                                 ,{CONTROLKEY,(PTEXT)KeyControl}
                                                 ,{CONTROLKEY,(PTEXT)KeyControl}
                                                 ,{CONTROLKEY,(PTEXT)KeyControl}
                                                 ,{CONTROLKEY,(PTEXT)KeyControl}
                                                 ,{CONTROLKEY,(PTEXT)KeyControl}
                                                 ,{CONTROLKEY,(PTEXT)KeyControl}
                                                 ,{CONTROLKEY,(PTEXT)KeyControl}} }
                      , {WIDE("menu"),0,0, {{CONTROLKEY,(PTEXT)KeyAlt}
                                                 ,{CONTROLKEY,(PTEXT)KeyAlt}
                                                 ,{CONTROLKEY,(PTEXT)KeyAlt}
                                                 ,{CONTROLKEY,(PTEXT)KeyAlt}
                                                 ,{CONTROLKEY,(PTEXT)KeyAlt}
                                                 ,{CONTROLKEY,(PTEXT)KeyAlt}
                                                 ,{CONTROLKEY,(PTEXT)KeyAlt}
                                                 ,{CONTROLKEY,(PTEXT)KeyAlt}} }
                      , {WIDE("pause"),0,0 }
                      , {WIDE("captial"),0,0 }
                      , NONAMES, NONAMES, NONAMES, NONAMES, NONAMES, NONAMES
                      , {WIDE("esc"), WIDE("escape"), 0, {{KEYDATA}}} // 0x1b
                      , NONAMES, NONAMES
                      , NONAMES, NONAMES
                      , {WIDE("space"), WIDE("blank"), 0, {{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}
                                              ,{KEYDATA}} } //0x20
                      , {WIDE("prior"), WIDE("pgup"), 0, {{HISTORYKEY,(PTEXT)HistoryPageUp}} }
                      , {WIDE("next"), WIDE("pgdn"), 0, {{HISTORYKEY,(PTEXT)HistoryPageDown}} }
                      , {WIDE("end"), 0, 0, {{COMMANDKEY, (PTEXT)KeyEndCmd}}}
                      , {WIDE("home"), 0, 0, {{COMMANDKEY, (PTEXT)KeyHome}}}
                      , {WIDE("left"), 0, 0, {{COMMANDKEY, (PTEXT)KeyLeft}}}

                      , {WIDE("up") , 0, 0, { {COMMANDKEY, (PTEXT)KeyUp}
                                       , {HISTORYKEY, (PTEXT)HistoryLineUp}}}
                      , {WIDE("right"), 0, 0, {{COMMANDKEY, (PTEXT)KeyRight}}}
                      , {WIDE("down"), 0, 0, {{COMMANDKEY, (PTEXT)HandleKeyDown}
                                       , {HISTORYKEY, (PTEXT)HistoryLineDown}}}
                      , {WIDE("select")}
                      , {WIDE("print")}
                      , {WIDE("execute")}
                      , {WIDE("snapshot")}
                      , {WIDE("insert"), 0, 0, {{COMMANDKEY, (PTEXT)KeyInsert}}}
                      , {WIDE("delete"), 0, 0, {{KEYDATA_DEFINED, (PTEXT)&KeyStroke+KS_DELETE}}}
                      , {WIDE("help")}
                      , {WIDE("0"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}      //0x30
                      , {WIDE("1"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , {WIDE("2"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}
                                    ,{KEYDATA}}}
                      , {WIDE("3"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , {WIDE("4"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , {WIDE("5"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , {WIDE("6"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , {WIDE("7"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , {WIDE("8"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , {WIDE("9"), 0, 0, {{KEYDATA}
                                    ,{KEYDATA}}}
                      , NONAMES, NONAMES
                      , NONAMES, NONAMES
                      , NONAMES, NONAMES
                      , NONAMES
/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */
                      //..... to hmm 39
                      , {WIDE("a"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}} // 0x41 'A'
                      , {WIDE("b"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("c"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("d"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("e"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("f"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("g"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("h"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("i"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("j"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("k"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("l"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("m"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("n"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("o"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("p"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("q"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("r"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("s"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("t"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("u"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("v"),0,0,{{KEYDATA}
											,{KEYDATA}
											,{SPECIALKEY,(PTEXT)_Key_KeystrokePaste}
											,{SPECIALKEY,(PTEXT)_Key_KeystrokePaste}
											//,{KEYDATA}
							 }}
                      , {WIDE("w"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("x"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("y"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("z"),0,0,{{KEYDATA}
                                           ,{KEYDATA}
                                           ,{KEYDATA}}}
                      , {WIDE("lwin") }
                      , {WIDE("rwin") }
                      , {WIDE("apps") }
                      , NONAMES, NONAMES
                      , {WIDE("num0"), WIDE("pad0"),0,{{KEYDATA}} }
                      , {WIDE("num1"), WIDE("pad1"),0,{{KEYDATA}} }
                      , {WIDE("num2"), WIDE("pad2"),0,{{KEYDATA}} }
                      , {WIDE("num3"), WIDE("pad3"),0,{{KEYDATA}} }
                      , {WIDE("num4"), WIDE("pad4"),0,{{KEYDATA}} }
                      , {WIDE("num5"), WIDE("pad5"),0,{{KEYDATA}} }
                      , {WIDE("num6"), WIDE("pad6"),0,{{KEYDATA}} }
                      , {WIDE("num7"), WIDE("pad7"),0,{{KEYDATA}} }
                      , {WIDE("num8"), WIDE("pad8"),0,{{KEYDATA}} }
                      , {WIDE("num9"), WIDE("pad9"),0,{{KEYDATA}} }
                      , {WIDE("mult"), WIDE("mulitply"),0,{{KEYDATA}} }
                      , {WIDE("add"),0,0,{{KEYDATA} }}
                      , {WIDE("separator"), 0, 0 }
                      , {WIDE("subtract"),0,0,{{KEYDATA}} }
                      , {WIDE("decimal"),0,0,{{KEYDATA}} }
                      , {WIDE("divide"),0,0,{{KEYDATA}} }
                      , {WIDE("f1") }   // 0x70
                      , {WIDE("f2") }
                      , {WIDE("f3") }
                      , {WIDE("f4") }
                      , {WIDE("f5") }
                      , {WIDE("f6") }
                      , {WIDE("f7") }
                      , {WIDE("f8") }
                      , {WIDE("f9") }
                      , {WIDE("f10") }
                      , {WIDE("f11") }
                      , {WIDE("f12") }
                      , {WIDE("f13") }
                      , {WIDE("f14") }
                      , {WIDE("f15") }
                      , {WIDE("f16") }
                      , {WIDE("f17") }
                      , {WIDE("f18") }
                      , {WIDE("f19") }
                      , {WIDE("f20") }
                      , {WIDE("f21") }
                      , {WIDE("f22") }
                      , {WIDE("f23") }
                      , {WIDE("f24") } // 0x87
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , {WIDE("numlock"),0,0} // 0x90
                      , {WIDE("scroll"),0,0}

                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
                      , NONAMES // {WIDE("lshift"),0,0}  // 0xa0
                      , NONAMES // {WIDE("rshift"),0,0}
                      , NONAMES // {WIDE("lctrl"), WIDE("lcontrol"),0,0}
                      , NONAMES // {WIDE("rctrl"), WIDE("rcontrol"),0,0}
                      , NONAMES // {WIDE("lmenu"), WIDE("lalt"),0,0 }
                      , NONAMES // {WIDE("rmenu"), WIDE("ralt"),0,0 }
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES // 0xb0
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , {WIDE(";"), WIDE("semicolon"),0,{{KEYDATA}
                                            ,{KEYDATA}} }
                      , {WIDE("="), WIDE("equal"),0    ,{{KEYDATA}
                                            ,{KEYDATA}}}
                      , {WIDE(","), WIDE("comma"),0    ,{{KEYDATA}
                                            ,{KEYDATA}} }
                      , {WIDE("-"), WIDE("dash"),0     ,{{KEYDATA}
                                            ,{KEYDATA}}}
                      , {WIDE("."), WIDE("period"),0   ,{{KEYDATA}
                                            ,{KEYDATA}}}
                      , {WIDE("/"),WIDE("slash"),0     ,{{KEYDATA}
                                            ,{KEYDATA}}}
                      , {WIDE("`"), WIDE("accent"),0   ,{{KEYDATA}
                                            ,{KEYDATA}} } // 0xc0
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES // 0xd0
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , { WIDE("["), WIDE("lbracket"),0  ,{{KEYDATA}
                                              ,{KEYDATA}} }
                      , { WIDE("\\"), WIDE("backslash"),0,{{KEYDATA}
                                              ,{KEYDATA}} }
                      , { WIDE("]"), WIDE("rbracket"),0  ,{{KEYDATA}
                                              ,{KEYDATA}} }
                      , { WIDE("'"), WIDE("quote"),0     ,{{KEYDATA}
                                              ,{KEYDATA}} }
                      , NONAMES
                      , NONAMES // 0xe0
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES // 0xf0
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES
                      , NONAMES  // 0xff
                       };
#endif
/*
#if(WINVER >= 0x0400)
#define VK_PROCESSKEY     0xE5
#endif

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE
*/
//----------------------------------------------------------------------------

int KeyUp( PCONSOLE_INFO pci )
{
	RecallCommand( pci->common.CommandInfo, TRUE );
   return UPDATE_COMMAND;
}

//----------------------------------------------------------------------------

int HandleKeyDown( PCONSOLE_INFO pci )
{
   RecallCommand( pci->common.CommandInfo, FALSE );
   return UPDATE_COMMAND;
}

//----------------------------------------------------------------------------

int KeyHome( PCONSOLE_INFO pci )
{
	SetCommandPosition( pci->common.CommandInfo, 0, COMMAND_POS_SET );
   return UPDATE_COMMAND; 
}

//----------------------------------------------------------------------------

int KeyEndCmd( PCONSOLE_INFO pci )
{
	SetCommandPosition( pci->common.CommandInfo, -1, COMMAND_POS_SET );
   return UPDATE_COMMAND;
}

//----------------------------------------------------------------------------

int KeyInsert( PCONSOLE_INFO pci )
{
	SetCommandInsert( pci->common.CommandInfo, -1 );
   return UPDATE_COMMAND;
}

//----------------------------------------------------------------------------

int KeyRight( PCONSOLE_INFO pci )
{
	SetCommandPosition( pci->common.CommandInfo, 1, COMMAND_POS_CUR );
	return UPDATE_COMMAND;
}

//----------------------------------------------------------------------------

int KeyLeft( PCONSOLE_INFO pci )
{
	SetCommandPosition( pci->common.CommandInfo, -1, COMMAND_POS_CUR );
	return UPDATE_COMMAND;
}

//----------------------------------------------------------------------------

int KeyShift( P_32 pKeyState, LOGICAL bDown )
{
   if( bDown )
   {
      *pKeyState |= KEY_MOD_SHIFT;
   }
   else
   {
      *pKeyState &= ~KEY_MOD_SHIFT;
   }
   return FALSE;
}

//----------------------------------------------------------------------------

int KeyControl( P_32 pKeyState, LOGICAL bDown )
{
   if( bDown )
   {
      *pKeyState |= KEY_MOD_CTRL;
   }
   else
   {
      *pKeyState &= ~KEY_MOD_CTRL;
   }
   return FALSE;
}

//----------------------------------------------------------------------------

int KeyAlt( P_32 pKeyState, LOGICAL bDown )
{
   if( bDown )
   {
      *pKeyState |= KEY_MOD_ALT;
   }
   else
   {
      *pKeyState &= ~KEY_MOD_ALT;
   }
   return FALSE;
}

//----------------------------------------------------------------------------
// Extensions and usage of keybinding data
// -- so far seperated so that perhaps it could be a seperate module...
//----------------------------------------------------------------------------

#define NUM_MODS ( sizeof( ModNames ) / sizeof( TEXTCHAR * ) )

TEXTCHAR *ModNames[] = { WIDE("shift"), WIDE("ctrl"), WIDE("alt")
                   , NULL, WIDE("control"), NULL
                   , WIDE("$"), WIDE("^"), WIDE("@") };

int FindMod( PTEXT pMod )
{
   int i;
   for( i = 0; i < NUM_MODS; i++ )
   {
      if( ModNames[i] )
         if( TextLike( pMod, ModNames[ i ] ) )
            break;
   }
   if( i < NUM_MODS )
      return ( 1 << ( i % 3 ) );
   return 0;
}

int FindKey( PTEXT pKey )
{
   int i;
   for( i = 0; i < NUM_KEYS; i++ )
   {
		//if( ( !KeyDefs[i].flags ) ||
		//    ( KeyDefs[i].flags & (KDF_NODEFINE) ) )
      //   continue;
      if( KeyDefs[i].name1 && TextLike( pKey, KeyDefs[i].name1 ) )
      {
         return i;
      }
      else if( KeyDefs[i].name2 && TextLike( pKey, KeyDefs[i].name2 ) )
      {
         return i;
      }
   }
   return 0;
}

//----------------------------------------------------------------------------
#ifdef __DEKWARE_PLUGIN__
static void DestroyKeyMacro( PMACRO pm )
{
   PTEXT temp;
   INDEX idx;
   if( pm->flags.un.macro.bUsed )
   {
      pm->flags.un.macro.bDelete = TRUE;
      return;
   }
   LineRelease( pm->pArgs );
   LineRelease( pm->pName );
   LineRelease( pm->pDescription );
   LIST_FORALL( pm->pCommands, idx, PTEXT, temp )
   {
      LineRelease( temp );
   }
   DeleteList( &pm->pCommands );
   Release( pm );
}
#endif

// Usage: /KeyBind shift-F1
//        ... #commands
//        /endmac
// Usage: /KeyBind shift-F1 kill
// Usage: /KeyBind $F1 ... ^F1 $^F1
//  if parameters follow the keybind key-def, those params
//  are taken as keystrokes to type...
//  if no parameters follow, the definition is assumed to
//  be a macro definition, and the macro is invoked by
//  the processing entity...
#ifdef __DEKWARE_PLUGIN__

CORECON_NPROC( int, KeyBind )( PDATAPATH pdp, PSENTIENT ps, PTEXT parameters )
{
   PTEXT pKey;
   int ModVal;
   int Mod = 0;
   int KeyVal;
	PVARTEXT vt;
   PCONSOLE_INFO pmdp = (PCONSOLE_INFO)pdp;

   do{
      pKey = GetParam( ps, &parameters );
      if( pKey )
      {
         ModVal = FindMod( pKey );
         Mod |= ModVal;
      }
      else
         break;
   }while( ModVal );
   if( pKey )
      KeyVal = FindKey( pKey );
   else
   {
      DECLTEXT( msg, WIDE("Not enough parameters to KeyBind...") );
      EnqueLink( &ps->Command->Output, &msg );
      return 0;
   }

   if( !KeyVal )
   {
      DECLTEXT( msg, WIDE("First parameters to KeyBind were not a known key...") );
      EnqueLink( &ps->Command->Output, &msg );
      return 0;
   }
   else
   {
      //DECLTEXTSZ( msg, 256 );
      PMACRO pm;
      PTEXT pStroke;
      if( pmdp->Keyboard[KeyVal][Mod].flags.bMacro )
      {
         DECLTEXT( msg, WIDE("Destroyed prior key macro") );
         EnqueLink( &ps->Command->Output, &msg );
         DestroyKeyMacro( pmdp->Keyboard[KeyVal][Mod].data.macro );
      }
      if( pmdp->Keyboard[KeyVal][Mod].flags.bStroke )
      {
         DECLTEXT( msg, WIDE("Destroyed prior key stroke") );
         EnqueLink( &ps->Command->Output, &msg );
         LineRelease( pmdp->Keyboard[KeyVal][Mod].data.stroke );
      }
      {
         PTEXT pSubst;
         pSubst = MacroDuplicateEx( ps, parameters, FALSE, TRUE );
         if( pSubst )
         {
            pStroke = BuildLine( pSubst );
            LineRelease( pSubst );
         }
         else
            pStroke = NULL;
      }
      if( pStroke )
      {
         pmdp->Keyboard[KeyVal][Mod].flags.bStroke = TRUE;
         pmdp->Keyboard[KeyVal][Mod].flags.bMacro = FALSE;
         pmdp->Keyboard[KeyVal][Mod].data.stroke = pStroke;
         return 0;
      }
      vt = VarTextCreate();
      if( Mod & KEYMOD_SHIFT )
      {
         if( Mod & KEYMOD_CTRL )
            if( Mod & KEYMOD_ALT )
               vtprintf( vt, WIDE("shift-ctrl-alt-%s")
                           ,KeyDefs[KeyVal].name1 );
            else
               vtprintf( vt, WIDE("shift-ctrl-%s")
                           ,KeyDefs[KeyVal].name1 );
         else
            if( Mod & KEYMOD_ALT )
               vtprintf( vt, WIDE("shift-alt-%s")
                           ,KeyDefs[KeyVal].name1 );
            else
               vtprintf( vt, WIDE("shift-%s")
                           ,KeyDefs[KeyVal].name1 );
      }
      else
      {
         if( Mod & KEYMOD_CTRL )
            if( Mod & KEYMOD_ALT )
               vtprintf( vt, WIDE("ctrl-alt-%s")
                           ,KeyDefs[KeyVal].name1 );
            else
               vtprintf( vt, WIDE("ctrl-%s")
                           ,KeyDefs[KeyVal].name1 );
         else
            if( Mod & KEYMOD_ALT )
               vtprintf( vt, WIDE("alt-%s")
                           ,KeyDefs[KeyVal].name1 );
            else
               vtprintf( vt, WIDE("%s")
                           ,KeyDefs[KeyVal].name1 );
      }
      pm = (PMACRO)Allocate( sizeof( MACRO ) );
      MemSet( pm, 0, sizeof( MACRO ) );
      pm->flags.bMacro = TRUE;
      pm->pName = VarTextGet( vt );
      pm->pDescription = NULL;
      pm->nArgs = 0;
      pm->pArgs = NULL;
      ps->pRecord = pm;
      pmdp->Keyboard[KeyVal][Mod].flags.bMacro = TRUE;
      pmdp->Keyboard[KeyVal][Mod].flags.bStroke = FALSE;
      pmdp->Keyboard[KeyVal][Mod].data.macro = pm;
		VarTextDestroy( &vt );
   }
   return 0;
}
#endif
//----------------------------------------------------------------------------

#ifdef __DEKWARE_PLUGIN__
CORECON_NPROC( int, KeyUnBind )( PDATAPATH pdp, PSENTIENT ps, PTEXT parameters )
{
   PTEXT pKey;
   int ModVal;
   int Mod = 0;
   int KeyVal;
   PCONSOLE_INFO pmdp = (PCONSOLE_INFO)pdp;

   do{
      pKey = GetParam( ps, &parameters );
      if( pKey )
      {
         ModVal = FindMod( pKey );
         Mod |= ModVal;
      }
      else
         break;
   }while( ModVal );
   if( pKey )
      KeyVal = FindKey( pKey );
   else
   {
      DECLTEXT( msg, WIDE("Not enough parameters to KeyUnBind...") );
      EnqueLink( &ps->Command->Output, &msg );
      return 0;
   }

   if( !KeyVal )
   {
      DECLTEXT( msg, WIDE("First parameters to KeyUnBind were not a known key...") );
      EnqueLink( &ps->Command->Output, &msg );
      return 0;
   }
   else
   {
      if( pmdp->Keyboard[KeyVal][Mod].flags.bMacro )
      {
         DECLTEXT( msg, WIDE("Destroyed prior key macro") );
         EnqueLink( &ps->Command->Output, &msg );
         DestroyKeyMacro( pmdp->Keyboard[KeyVal][Mod].data.macro );
         pmdp->Keyboard[KeyVal][Mod].data.macro = NULL;
         pmdp->Keyboard[KeyVal][Mod].flags.bMacro = FALSE;

      }
      else if( pmdp->Keyboard[KeyVal][Mod].flags.bStroke )
      {
         DECLTEXT( msg, WIDE("Destroyed prior key stroke") );
         EnqueLink( &ps->Command->Output, &msg );
         LineRelease( pmdp->Keyboard[KeyVal][Mod].data.stroke );
         pmdp->Keyboard[KeyVal][Mod].data.stroke = NULL;
         pmdp->Keyboard[KeyVal][Mod].flags.bStroke = FALSE;
      }
      else
      {
         DECLTEXT( msg, WIDE("Key was not bound...") );
         EnqueLink( &ps->Command->Output, &msg );
      }
   }

   return 0;
}
#endif
//----------------------------------------------------------------------------

#ifdef __DEKWARE_PLUGIN__
int KeyList( PSENTIENT ps, PTEXT parameters )
{
	// somewhere I should have macro dump 
	// this would be useful for this thing also..
	// header's a little different though
	return 0;
}
#endif
//----------------------------------------------------------------------------

int DoStroke( PCONSOLE_INFO pdp, PTEXT stroke )
{
   INDEX i;
   int bOutput = FALSE;
   DECLTEXT( key, WIDE(" ") );
   //Log1( WIDE("Do Stroke with %c"), stroke->data.data[0] );
   while( stroke )
   {
      for( i = 0; i < stroke->data.size; i++ )
      {
         switch( key.data.data[0] = stroke->data.data[i] )
         {
         case '\r':
            // output is always prefix linefed...
            key.data.data[0] = '\n'; // carriage return = linefeed
            goto normal_process;  // do not output return... extra lines otherwise
         case 9:
            key.data.data[0] = ' ';
         case 27:
         case '\b':
            //pdp->bUpdateToEnd = FALSE; // need update ALL not just to end
         default:
         normal_process:
            {
               PTEXT pLine;
               if( pdp->flags.bDirect && pdp->flags.bCharMode )
               {
                  PTEXT newseg = TextDuplicate( stroke, FALSE );
                  if( stroke->data.data[0] == '\r' )
                  {
                     newseg->flags &= ~TF_NORETURN;
                     newseg->data.size = 0;
                  }
                  else
							newseg->flags |= TF_NORETURN;
#ifdef __DEKWARE_PLUGIN__
						newseg->flags |= TF_RELAY; // just pipe it through to the final output...
                  // direct mode is REALLY direct.
						EnqueLink( &pdp->common.Input, newseg );
#else
//						EnqueLink( &pdp->Input, newseg );
#endif
                  i = stroke->data.size;
                  break;
               }
               else
               {
						if( ( pLine =
							  GatherCommand( pdp->
#ifdef __DEKWARE_PLUGIN__
												 common.
#endif
												 CommandInfo
												, (PTEXT)&key ) ) )
                  {
                     // I dunno - maybe we shouldn't enque the
                     // command... maybe we should count on whatever
                     // the destination is to echo it
                     // or - perhaps this should be another
                     // option only effective while bDirect...
                     if( pdp->flags.bDirect && !pdp->flags.bNoLocalEcho )
                     {
                        PTEXT pEcho;
                        pEcho = BuildLine( pLine );
                        pEcho->data.size--; // trim the last character (probably cr)
                        pEcho->flags |= TF_NORETURN;
#ifdef __DEKWARE_PLUGIN__
								EnqueLink( &pdp->common.Output, pEcho );
#else
								LineRelease( pEcho );
#endif
                        lprintf( WIDE("Should this local echo be marked somehow?") );
                        //pdp->History.flags.bEnqueuedLocalEcho = 1;
                        //pdp->flags.bLastEnqueCommand = TRUE;
							}
#ifdef __DEKWARE_PLUGIN__
                     EnqueLink( &pdp->common.Input, TextDuplicate( pLine, FALSE ) );
							WakeAThread( pdp->common.Owner );
#endif
                  }
                  else // didn't get a linefeed... so.. safe echo
                  {
                     //Log( WIDE("Failed to Gather Line...") );
                  }
               }
            }
            bOutput = TRUE;
            break;
         }
      }
      stroke = NEXTLINE( stroke );
   }
   return bOutput;
}

//----------------------------------------------------------------------------

void KeyPressHandler( PCONSOLE_INFO pdp
						  , _8 key_index
						  , _8 mod
						  , PTEXT characters
						  )
{
	int bOutput = 0;
	// check current keyboard override...
   if( KeyDefs[key_index].flags & KDF_CAPSKEY )
   {
      //if( event.dwControlKeyState & CAPSLOCK_ON )
      //{
      //   mod ^= KEYMOD_SHIFT;
      //}
   }


	if( pdp->Keyboard[key_index][mod].flags.bStroke ||
		pdp->Keyboard[key_index][mod].flags.bMacro )
	{
		if( pdp->Keyboard[key_index][mod].flags.bStroke )
		{
			extern void CPROC WinLogicDoStroke( PCONSOLE_INFO pdp, PTEXT stroke );
			WinLogicDoStroke(pdp, pdp->Keyboard[key_index][mod].data.stroke);
		}
#ifdef __DEKWARE_PLUGIN__
		else if( pdp->Keyboard[key_index][mod].flags.bMacro )
		{
			if( pdp->common.Owner->pRecord != pdp->Keyboard[key_index][mod].data.macro )
				InvokeMacro( pdp->common.Owner
							  , pdp->Keyboard[key_index][mod].data.macro
							  , NULL );
		}
#endif
	}
	else // key was not overridden
	{
		int result = 0;
		//Log1( WIDE("Keyfunc = %d"), KeyDefs[key_index].op[mod].bFunction );
		switch( KeyDefs[key_index].op[mod].bFunction )
		{
		case KEYDATA_DEFINED:
			//Log( WIDE("Key data_defined") );
			{
				extern void CPROC WinLogicDoStroke( PCONSOLE_INFO pdp, PTEXT stroke );
				WinLogicDoStroke( pdp, KeyDefs[key_index].op[mod].data.pStroke );
			}
			result = UPDATE_NOTHING; // unsure about this - recently added.
			// well it would appear that the stroke results in whether to update
			// the command prompt or not.
			break;
		case KEYDATA:
			if( GetTextSize( characters ) )
			{
				extern void CPROC WinLogicDoStroke( PCONSOLE_INFO pdp, PTEXT stroke );
				WinLogicDoStroke( pdp, characters );
			}
			result = UPDATE_NOTHING; // already taken care of?!
			break;
		case COMMANDKEY:
			result = KeyDefs[key_index].op[mod].data.CommandKey( pdp );
			break;
		case HISTORYKEY:
			result = KeyDefs[key_index].op[mod].data.HistoryKey( pdp->pHistoryDisplay );
			break;
		case CONTROLKEY:
			KeyDefs[key_index].op[mod].data.ControlKey( &pdp->dwControlKeyState, TRUE );
			result = UPDATE_NOTHING;
			break;
		case SPECIALKEY:
			result = KeyDefs[key_index].op[mod].data.SpecialKey( pdp );
			break;
		}
		switch( result )
		{
		case UPDATE_COMMAND:
			{
				PENDING_RECT upd;
				extern void RenderCommandLine( PCONSOLE_INFO pdp, POINTER region );
				upd.flags.bHasContent = 0;
				upd.flags.bTmpRect = 1;
				RenderCommandLine( pdp, &upd );
				{
					RECT r;
					r.left = upd.x;
					r.right = upd.x + upd.width;
					r.top = upd.y;
					r.bottom = upd.y + upd.height;
					pdp->Update( pdp, &r );
				}
			}

			bOutput = TRUE;
			break;
		case UPDATE_HISTORY:
			{
				extern int CPROC UpdateHistory( PCONSOLE_INFO pdp );
				if( UpdateHistory( pdp ) )
				{
               extern void CPROC RenderConsole( PCONSOLE_INFO pdp );
					RenderConsole( pdp );
				}
			}
			break;
		case UPDATE_DISPLAY:
			{
            extern void CPROC ChildCalculate( PCONSOLE_INFO pdp );
				ChildCalculate( pdp );
			}
			break;
		}
	}
}
//----------------------------------------------------------------------------

// $Log: keydefs.c,v $
// Revision 1.37  2005/08/08 15:24:12  d3x0r
// Move updated rectangle struct to common space.  Improved curses console handling....
//
// Revision 1.36  2005/04/22 18:34:09  d3x0r
// Fix CPROC declaration of keystroke paste handler.
//
// Revision 1.35  2005/04/15 07:28:39  d3x0r
// Okay this all seems to work sufficicnetly - disabled logging entirely.
//
// Revision 1.34  2005/02/24 00:47:14  d3x0r
// Updated vc projects - use $(SACK_BASE) refernece instead of hard coded path.  Also ported to studio 2003
//
// Revision 1.33  2005/01/28 09:53:34  d3x0r
// Okay all forms of graphical interface work on windows platform with appropriate updates
//
// Revision 1.32  2005/01/27 17:31:37  d3x0r
// psicon works now, added some locks to protect multiple accesses to datapath (render/update_write)
//
// Revision 1.31  2005/01/26 20:00:01  d3x0r
// Okay - need to do something about partial updates - such as command typing should only update that affected area of the screen...
//
// Revision 1.30  2005/01/23 04:07:57  d3x0r
// Hmm somehow between display rendering stopped working.
//
// Revision 1.29  2005/01/20 06:10:19  d3x0r
// One down, 3 to convert... concore library should serve to encapsulate drawing logic and history code...
//
// Revision 1.28  2004/09/29 09:31:32  d3x0r
//  Added support to make page breaks on output.  Fixed up some minor issues with scrollback distances
//
// Revision 1.27  2004/09/27 16:06:17  d3x0r
// Checkpoint - all seems well.
//
// Revision 1.26  2004/09/23 09:53:33  d3x0r
// Looks like it's in a usable state for windows... perhaps an alpha/beta to be released soon.
//
// Revision 1.25  2004/09/09 13:41:04  d3x0r
// works much better passing correct structures...
//
// Revision 1.24  2004/08/13 09:29:50  d3x0r
// checkpoint
//
// Revision 1.23  2004/07/30 14:08:40  d3x0r
// More tinkering...
//
// Revision 1.22  2004/06/12 08:42:34  d3x0r
// Well it initializes, first character causes failure... all windows targets build...
//
// Revision 1.21  2004/06/10 22:11:00  d3x0r
// more progress...
//
// Revision 1.20  2004/06/08 00:23:26  d3x0r
// Display and history combing proceeding...
//
// Revision 1.19  2004/05/14 18:35:40  d3x0r
// Checkpoint
//
// Revision 1.18  2004/05/13 23:35:07  d3x0r
// checkpoint
//
// Revision 1.17  2004/05/12 10:05:11  d3x0r
// checkpoint
//
// Revision 1.16  2003/12/10 21:59:51  panther
// Remove all LIST_ENDFORALL
//
// Revision 1.15  2003/11/08 00:09:41  panther
// fixes for VarText abstraction
//
// Revision 1.14  2003/04/17 06:40:32  panther
// More merging of wincon/psicon.  Should nearly have the renderer totally seperate now from the logic
//
// Revision 1.13  2003/03/28 12:16:29  panther
// Fix some minor issues with PSI interface
//
// Revision 1.12  2003/03/26 02:31:19  panther
// Windows keydefs - use HandleKeyDown not KeyDown
//
// Revision 1.11  2003/03/26 02:08:59  panther
// And cleanup FindMyDataPath more migration of handling options
//
// Revision 1.10  2003/03/26 02:05:21  panther
// begin updating option handlers to real option handlers
//
// Revision 1.9  2003/03/26 01:42:24  panther
// Update each section implicitly by drawing it.
//
// Revision 1.8  2003/03/26 01:08:20  panther
// Fix CVS conflict error.  Clean some warnings with typecasting
//
// Revision 1.7  2003/03/26 00:44:17  panther
// Enable mouse.  Perform updates to display.  Misc.
//
// Revision 1.6  2003/03/25 08:59:02  panther
// Added CVS logging
//
