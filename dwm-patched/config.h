/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

/*Macros->*/
/*Tag bit-mask macros*/
#define TAG(N) (1 << (N - 1))
#define TAGS2(A, B) (TAG(A) | TAG(B))
#define TAGS3(A, B, C) (TAG(A) | TAG(B) | TAG(C))

/* Rule macros*/
#define CLASS(C) (C), NULL, NULL
#define INSTANCE(I) NULL, (I), NULL
#define TITLE(T) NULL, NULL, (T)
#define CLASS_W_TITLE(C, T) (C), NULL, (T)
/*->Macros*/

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int minwsz    = 20;       /* Minimal heigt of a client for smfact */
static const unsigned int gappx     = 3;        /* gap pixel between windows */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;     /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const unsigned int maxwnln   = 64;       /* maximum length of windows names on status bar */
static const char *fonts[]          = { "monospace:size=10" };
static const char dmenufont[]       = "monospace:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_white[]       = "#fcfcfc";
static const char col_red[]         = "#ff0000";
static const char col_cyan[]        = "#005577";
static const char col_green[]       = "#557700";
static const char col_pink[]        = "#770055";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_white, col_gray1, col_cyan  },
	[SchemeUrg] = { col_gray3, col_gray1, col_red },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/*Match condition   Tags   Float   Terminal   Swallow   Monitor*/
	{ CLASS("Firefox|Chromium|Google-chrome|Vivaldi-stable"),   TAG(2),   0,   0 },
        { CLASS("VirtualBox"),   TAG(5),   0,   0 },
        { CLASS("St|UXTerm|XTerm|rxvt|URxvt|Urxvt-tabbed"),   TAG(1),   0,   1,   1,   0 },
        { CLASS("Thunar|Pcmanfm|pcmanfm-qt"),   TAG(3),   0,   0 },
	{ CLASS("vlc|smplayer|mpv|smplayer"),   TAG(1)|TAG(3)|TAG(4),   0,   0 },
	{ CLASS("Pragha"),  TAG(4) ,   1,   0 },

        /*Floating windows*/
	{ CLASS("Gimp"),   0,   1,   0 },
	{ CLASS("Dukto"),   0,   1,   0 },
	{ CLASS("lxsu|lxsudo"),   0,   1,   0 },
        { TITLE("File Operation Progress"),   0,   1,   0 },
        { TITLE("Module"),   0,   1,   0 },
        { TITLE("Search Dialog"),   0,   1,   0 },
        { TITLE("Goto"),   0,   1,   0 },
        { TITLE("IDLE Preferences"),   0,   1,   0 },
        { TITLE("Preferences"),   0,   1,   0 },
        { TITLE("File Transfer"),   0,   1,   0 },
        { TITLE("branchdialog"),   0,   1,   0 },
        { TITLE("pinentry"),   0,   1,   0 },
        { TITLE("confirm"),   0,   1,   0 },
        { INSTANCE("gpick"),   0,    1,    0 },
        { INSTANCE("eog"),   0,   1,    0 },
	{ CLASS_W_TITLE("Firefox","Firefox Preferences"),   TAG(2),   1,   0 },
	{ CLASS_W_TITLE("Firefox","Library"),   TAG(2),   1,   0 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const float smfact     = 0.00; /* factor of tiled clients [0.00..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define ALTMODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,          {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggletag,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,           {.ui = 1 << TAG} }, \
	{ MODKEY|ALTMODKEY,             KEY,      toggleview,    {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *roficmd[] = { "rofi", "-modi", "combi#window#run#drun", "-show", "combi", "-combi-modi", "window#run#drun", NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *termcmdalt1[]  = { "urxvt", NULL };
static const char *termcmdalt2[]  = { "qterminal", NULL };
static const char scratchpadname[] = "scratchpad";
static const char *scratchpadcmd[] = { "st", "-t", scratchpadname, "-g", "120x34", NULL };
static const char *firefox[] = { "firefox", NULL, NULL, NULL, "Firefox" };
static const char *filemanager[] = { "pcmanfm", NULL, NULL, NULL, "Pcmanfm" };
static const char *ranger[] = { "urxvt", "-e", "ranger" };
static const char *nnn[] = { "st", "-e", "nnnstart" };
static const char *cmus[] = { "st", "-e", "cmus" };
static const char *musicplayer[] = { "pragha", NULL, NULL, NULL, "Pragha" };
static const char *texteditor[] = { "kwrite", NULL, NULL, NULL, "kwrite" };
static const char *volumeup[] = { "amixer", "set", "Master", "5%+", NULL };
static const char *volumedown[] = { "amixer", "set", "Master", "5%-", NULL };
static const char *volumemute[] = { "amixer", "set", "Master", "toggle", NULL };
static const char *audionext[] = { "playerctl", "next", NULL };
static const char *audioprev[] = { "playerctl", "previous", NULL };
static const char *audiostop[] = { "playerctl", "stop" };
static const char *lock[] = { "slock", NULL };
static const char *screenshot[] = { "lximage-qt", "-s", NULL };
static const char *lxtask[] = { "lxtask", NULL, NULL, NULL, "Lxtask" };
static const char *htop[] = { "st", "-e", "htop", NULL };
#include "selfrestart.c"
#include "shiftview.c"

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_o,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_p,      spawn,          {.v = roficmd } },
	{ MODKEY|ControlMask,           XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmdalt1 } },
	{ MODKEY|ALTMODKEY,             XK_Return, spawn,          {.v = termcmdalt2 } },
        { MODKEY,                       XK_grave,  togglescratch,  {.v = scratchpadcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
        { MODKEY|ShiftMask,             XK_i,      shiftview,      {.i = +1 } },
        { MODKEY|ShiftMask,             XK_u,      shiftview,      {.i = -1 } },
        { MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_h,      setsmfact,      {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_l,      setsmfact,      {.f = -0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
        { MODKEY|ShiftMask,             XK_r,      self_restart,   {0} },
	/*{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },*/
	{ MODKEY|ShiftMask,             XK_Scroll_Lock,      spawn,           {.v = lock} },
	{ MODKEY|ShiftMask,             XK_q,      spawn,          SHCMD("~/.script/dwm-logout_menu") },
	{ MODKEY|ShiftMask,             XK_Pause,  spawn,          SHCMD("~/.script/dwm-rofi_exit_menu") },
	{ MODKEY|ControlMask,           XK_r,      spawn,          {.v = ranger} },
	{ MODKEY|ControlMask,           XK_n,      spawn,          {.v = nnn} },
	{ MODKEY|ControlMask,           XK_m,      spawn,          {.v = cmus} },
	{ MODKEY|ControlMask,           XK_p,      runorraise,     {.v = musicplayer} },
	{ 0,       XF86XK_AudioRaiseVolume,        spawn,          {.v = volumeup} },
	{ 0,       XF86XK_AudioLowerVolume,        spawn,          {.v = volumedown} },
	{ 0,       XF86XK_AudioMute,               spawn,          {.v = volumemute} },
	{ 0,       XF86XK_AudioNext,               spawn,          {.v = audionext} },
	{ 0,       XF86XK_AudioPrev,               spawn,          {.v = audioprev} },
	{ 0,       XF86XK_AudioStop,               spawn,          {.v = audiostop} },
        { MODKEY|ControlMask,           XK_w,      runorraise,     {.v = firefox} },
        { MODKEY|ControlMask,           XK_Home,   runorraise,     {.v = filemanager} },
        { MODKEY|ControlMask,           XK_e,      runorraise,     {.v = texteditor} },
        { MODKEY|ControlMask,           XK_Print,  runorraise,     {.v = screenshot} },
        { MODKEY|ControlMask,           XK_Delete, runorraise,     {.v = lxtask} },
        { MODKEY,                       XK_Delete, spawn,          {.v = htop} },
        { MODKEY,                       XK_Print,  spawn,          SHCMD("~/bin/winshot") },
        { 0,                            XK_Print,  spawn,          SHCMD("~/bin/screenshot") },


};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

/* signal definitions */
/* signum must be greater than 0 */
/* trigger signals using `xsetroot -name "fsignal:<signum>"` */
static Signal signals[] = {
	/* signum       function        argument  */
	{ 1,            setlayout,      {.v = 0} },
	{ 9,            quit,      {.v = 0} },
};
