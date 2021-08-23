#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "uobot_handles.h"
#include "uobot_net.h"
#include "uobot_log.h"
#include "uobot_obj.h"
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot_exthand.h"
#include "uobot_threads.h"
#include "uobot_windows.h"
#include "uobot_commands.h"
#include "uobot_menu.h"
#include "uobot_gumps.h"
#include "uobot_plugins.h"
#include "uobot_teleto.h"
#include "uobot_vend.h"
#include "uobot.h"

int DebugClientMsg = 0;
int IsConnected = 0;
int IsSkillUpdated = 0;
int UltimaMessenger = 1;
BOOL PluginThreadSuspended=FALSE;
int AntiMacro=0;
int LastStaffCheck=0;

// Thread pro ping
HANDLE hThread=0;

unsigned int GetPacketLen(unsigned char *Buf, unsigned int Size);


struct gmrecv am[] = { // Botao pre-definido(x,y relativos ao gumppic)
	{270, 75, 0, 0},
	{300, 75, 0, 0},
	{330, 75, 0, 0},
	{360, 75, 0, 0},
	{390, 75, 0, 0},
	{270, 105, 0, 0},
	{300, 105, 0, 0},
	{330, 105, 0, 0},
	{360, 105, 0, 0},
	{390, 105, 0, 0}
};


// Uo Handles
struct hmsg hnmsg[0xcd] = {
	{"Create Character", 0x68, 0}, // 0x00
	{"Disconnect", 0x05, 0},
	{"Walk Request", 0x07, 0},
    {"Client Talk", SIZE_VARIABLE, 0},
    {"?", 0x02, 0},
    {"Attack", 0x05, 0},
    {"Double Click", 0x05, 0},
    {"Pick Up Item", 0x07, uo_handle_pickup},
    {"Drop Item", 0x0e, uo_handle_dropitem}, // 0x08
    {"Single Click", 0x05, 0},
    {"?", 0x0b, 0},
    {"?", 0x10a, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x03, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x3d, 0},
    {"?", 0xd7, 0}, // 0x10
    {"Character Status", SIZE_VARIABLE, uo_handle_char_stats},
    {"Perform Action", SIZE_VARIABLE, 0},
    {"Client Equip Item", 0x0a, 0},
    {"?", 0x06, 0},
    {"?", 0x09, 0},
    {"?", 0x01, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0}, // 0x18
    {"?", SIZE_VARIABLE, 0},
    {"Update Item", SIZE_VARIABLE, uo_handle_update_item},
    {"Enter World", 0x25, uo_handle_loguin_confirm},
    {"Server Talk", SIZE_VARIABLE, uo_handle_server_speech},
    {"Delete Object", 0x05, uo_handle_delete},
    {"?", 0x04, 0},
    {"?", 0x08, 0},
    {"Update Player", 0x13, uo_handle_draw_player}, // 0x20
	{"Deny Walk", 0x08, 0},
    {"Confirm Walk", 0x03, uo_handle_confirm_walk},
    {"Drag Animation", 0x1a, 0},
    {"Open Container", 0x07, uo_handle_opencontainer},  // 0x24
    {"Update Contained Item", 0x14, uo_handle_addtocontainer}, //0x25
    {"?", 0x05, 0},
    {"Deny Move Item", 0x02, 0},
    {"?", 0x05, 0}, // 0x28
    {"?", 0x01, 0},
    {"?", 0x05, 0},
    {"?", 0x02, 0},
    {"Death Dialog", 0x02,  0},
    {"?", 0x11, 0},
    {"Server Equip Item", 0x0f, uo_handle_equipitem},
    {"Combat Notification", 0x0a, 0},
    {"?", 0x05, 0}, // 0x30
    {"?", 0x01, 0},
    {"?", 0x02, 0},
    {"Pause Control", 0x02, 0},
    {"Status Request", 0x0a, 0},
    {"?", 0x28d, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x08, 0},
    {"?", 0x07, 0}, // 0x38
    {"?", 0x09, 0},
    {"Update Skills", SIZE_VARIABLE, uo_handle_updateskill},
    {"Vendor Buy Reply", SIZE_VARIABLE, 0},
    {"Update Contained Items", SIZE_VARIABLE, uo_handle_updatecontainer},    //0x3c
    {"?", 0x02, 0},
    {"?", 0x25, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0xc9, 0}, // 0x40
    {"?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x229, 0},
    {"?", 0x2c9, 0},
    {"?", 0x05, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x0b, 0},
    {"?", 0x49, 0}, // 0x48
    {"?", 0x5d, 0},
    {"?", 0x05, 0},
    {"?", 0x09, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Object Light Level", 0x06, 0},
    {"Global Light Level", 0x02, 0},
    {"?", SIZE_VARIABLE, 0}, // 0x50
    {"?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Error Code", 0x02, uo_handle_error_code},   // Idle message
    {"Sound Effect", 0x0c, 0},
    {"Login Complete", 0x01, uo_handle_loguin_complete},
    {"Map Data", 0x0b, 0},
    {"?", 0x6e, 0},
    {"?", 0x6a, 0}, // 0x58
    {"?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Set Time?", 0x04, 0},
    {"?", 0x02, 0},
    {"Select Character", 0x49, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x31, 0},
    {"?", 0x05, 0}, // 0x60
    {"?", 0x09, 0},
    {"?", 0x0f, 0},
    {"?", 0x0d, 0},
    {"?", 0x01, 0},
    {"Set Weather", 0x04, 0},
    {"Book Page Data", SIZE_VARIABLE, 0},
    {"?", 0x15, 0},
    {"?", SIZE_VARIABLE, 0}, // 0x68
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x03, 0},
    {"?", 0x09, 0},
    {"Target Data", 0x13, uo_handle_target},
    {"Play Music", 0x03, 0},
    {"Character Animation", 0x0e, 0},
    {"Secure Trading", SIZE_VARIABLE, uo_handle_trading},
    {"Graphic Effect", 0x1c, 0}, // 0x70
    {"Message Board Data", SIZE_VARIABLE, 0},
    {"War Mode", 0x05, 0},
    {"Ping", 0x02, 0},
    {"Vendor Buy List", SIZE_VARIABLE, 0},
    {"Rename Character", 0x23, 0},
    {"?", 0x10, 0},
    {"Update Character", 0x11, uo_handle_update_playerpos},
    {"Update Object", SIZE_VARIABLE, uo_handle_draw_object}, // 0x78
    {"?", 0x09, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x02, 0},
    {"Open Menu Gump", SIZE_VARIABLE, uo_handle_menu},
    {"Menu Choice", 0x0d, 0},
    {"?", 0x02, 0},
    {"?", SIZE_VARIABLE, 0},
    {"First Login", 0x3e, 0}, // 0x80
    {"?", SIZE_VARIABLE, 0},
    {"Login Error", 0x02, uo_handle_login_denied},
    {"Delete Character", 0x27, 0},
    {"?", 0x45, 0},
    {"?", 0x02, 0},
    {"Character List 2", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Open Paperdoll", 0x42, uo_handle_paperdoll}, // 0x88
    {"Corpse Equipment", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Relay Server", 0x0b, uo_handle_relay},
    {"?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Display Map", 0x13, 0}, // 0x90
    {"Second Login", 0x41, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Open Book", 0x63, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Dye Data", 0x09, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x02, 0},
    {"?", SIZE_VARIABLE, 0}, // 0x98
    {"Multi Placement", 0x1a, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Help Request", 0x102, 0},
    {"?", 0x135, 0},
    {"?", 0x33, 0},
    {"Vendor Sell List", SIZE_VARIABLE, uo_handle_sell},
    {"Vendor Sell Reply", SIZE_VARIABLE, 0},
    {"Select Server", 0x03, 0}, // 0xa0
    {"Update Hitpoints", 0x09, uo_handle_updatehp},
    {"Update Mana", 0x09, uo_handle_updatemana},
    {"Update Stamina", 0x09, uo_handle_updatestam},
    {"System Information", 0x95, 0},
    {"Open URL", SIZE_VARIABLE, 0},
    {"Tip Window", SIZE_VARIABLE, 0},
    {"Request Tip", 0x04, 0},
    {"Server List", SIZE_VARIABLE, uo_handle_servers_list}, // 0xa8
    {"Character List", SIZE_VARIABLE, uo_handle_list_chars},
    {"Attack Reply", 0x05, 0},
    {"Text Input Dialog", SIZE_VARIABLE, uo_handle_textdlg},
    {"Text Input Reply", SIZE_VARIABLE, 0},
    {"Unicode Client Talk", SIZE_VARIABLE, 0},
    {"Unicode Server Talk", SIZE_VARIABLE, uo_handle_unispeech},
    {"Display Death",0x0d, 0},
    {"Open Dialog Gump", SIZE_VARIABLE, uo_handle_open_gump}, // 0xb0
    {"Dialog Choice", SIZE_VARIABLE, 0},
    {"Chat Data", SIZE_VARIABLE, 0},
    {"Chat Text ?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"Open Chat Window", 0x40, 0},
    {"Popup Help Request", 0x09, 0},
    {"Popup Help Data", SIZE_VARIABLE, 0},
    {"Character Profile", SIZE_VARIABLE, 0}, // 0xb8
    {"Chat Enable", 0x03, 0},
    {"Display Guidance Arrow", 0x06, 0},
    {"Account ID ?", 0x09, 0},
    {"Season ?", 0x03, 0},
    {"Client Version", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"New Commands", SIZE_VARIABLE, 0},
    {"?", 0x24, 0}, // 0xc0
    {"Display cliloc String", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", SIZE_VARIABLE, 0},
    {"?", 0x06, 0},
    {"?", 0xcb, 0},
    {"?", 0x01, 0},
    {"?", 0x31, 0},
    {"?", 0x02, 0}, // 0xc8
    {"?", 0x06, 0},
    {"?", 0x06, 0},
    {"?", 0x07, 0},
    {"?", SIZE_VARIABLE, 0},
};

char *PacketNames[256] =
{
		"Create character", /* 0x00 */
    "Disconnect notification", /* 0x01 */
    "Move request", /* 0x02 */
    "Talk request", /* 0x03 */
		"GOD: God mode on/off request", /* 0x04 */
    "Attack request", /* 0x05 */
    "Double click on object", /* 0x06 */
    "Pick up object request", /* 0x07 */
    "Drop item request", /* 0x08 */
    "Single click on object|Request look", /* 0x09 */
    "Edit dynamics and statics", /* 0x0a */
    "Edit area", /* 0x0b */
    "Alter tiledata", /* 0x0c */
    "Send new NPC data to the server", /* 0x0d */
    "Edit template data", /* 0x0e */
    "Paperdoll", /* 0x0f */
    "Modify hue data", /* 0x10 */
    "Status window info", /* 0x11 */
    "Request skill/magic/action usage", /* 0x12 */
    "Equip/Unequip item", /* 0x13 */
    "Change item's Z value", /* 0x14 */
    "Follow character", /* 0x15 */
    "Request scripts list", /* 0x16 */
    "Script modifcation commands", /* 0x17 */
    "Add new script to server", /* 0x18 */
    "Modify NPC speech data", /* 0x19 */
    "Object information", /* 0x1a */
    "Login Confirm|Character location and body type", /* 0x1b */
    "Server/player speech", /* 0x1c */
    "Delete object", /* 0x1d */
    "Control animation", /* 0x1e */
    "Cause explosion", /* 0x1f */
    "Draw game player", /* 0x20 */
    "Character move reject", /* 0x21 */
    "Character move ok|Resync request", /* 0x22 */
    "Drag item", /* 0x23 */
    "Draw container", /* 0x24 */
    "Add item to container", /* 0x25 */
    "Kick player", /* 0x26 */
    "Unable to pickup object", /* 0x27 */
    "Unable to drop object|Clear square", /* 0x28 */
    "Object dropped ok", /* 0x29 */
    "Blood mode", /* 0x2a */
    "GOD: response to on/off request", /* 0x2b */
    "Resurrection menu choice", /* 0x2c */
    "Health", /* 0x2d */
    "Character worn item", /* 0x2e */
    "Fight occurring", /* 0x2f */
    "Attack granted ", /* 0x30 */
    "Attack ended", /* 0x31 */
    "GOD: Admin command", /* 0x32 */
    "Pause/resume client", /* 0x33 */
    "Get player status", /* 0x34 */
    "Get resource type", /* 0x35 */
    "Resource type data", /* 0x36 */
    "Move object", /* 0x37 */
    "Follow move", /* 0x38 */
    "Groups", /* 0x39 */
    "Update skills", /* 0x3a */
    "Buy items", /* 0x3b */
    "Items in container", /* 0x3c */
    "Ship", /* 0x3d */
    "Version retrieval", /* 0x3e */
    "Update object chunk", /* 0x3f */
    "Update terrain chunk", /* 0x40 */
    "Update tile data", /* 0x41 */
    "Update art", /* 0x42 */
    "Update animations", /* 0x43 */
    "Update hues", /* 0x44 */
    "Version ok", /* 0x45 */
    "New art work", /* 0x46 */
    "New terrain", /* 0x47 */
    "New animation", /* 0x48 */
    "New hues", /* 0x49 */
    "Destroy art", /* 0x4a */
    "Check client version", /* 0x4b */
    "Modify script names", /* 0x4c */
    "edit script file", /* 0x4d */
    "Personal light level", /* 0x4e */
    "Global light level", /* 0x4f */
    "Bulletin board header", /* 0x50 */
    "Bulleting board message", /* 0x51 */
    "Post bulleting board message", /* 0x52 */
    "Login rejected|Idle warning", /* 0x53 */
    "Play sound effect", /* 0x54 */
    "Login complete", /* 0x55 */
    "Plot course for ships", /* 0x56 */
    "Update regions", /* 0x57 */
    "Create new region", /* 0x58 */
    "Create new effect", /* 0x59 */
    "Update effect", /* 0x5a */
    "Set time of day", /* 0x5b */
    "Restart version", /* 0x5c */
    "Select character", /* 0x5d */
    "Server list", /* 0x5e */
    "Add server", /* 0x5f */
    "Remove server", /* 0x60 */
    "Delete static", /* 0x61 */
    "Move static", /* 0x62 */
    "Load area", /* 0x63 */
    "Attempt to load area request", /* 0x64 */
    "Set weather", /* 0x65 */
    "Show book page", /* 0x66 */
    "Simped", /* 0x67 */
    "Add LS script", /* 0x68 */
    "Friends", /* 0x69 */
    "Notify friend", /* 0x6a */
    "Use key", /* 0x6b */
    "Targeting cursor", /* 0x6c */
    "Play midi music", /* 0x6d */
    "Show animation", /* 0x6e */
    "Secure trading window", /* 0x6f */
    "Play graphical effect", /* 0x70 */
    "Bulleting board commands", /* 0x71 */
    "Set war/peace mode request", /* 0x72 */
    "Ping", /* 0x73 */
    "Open buy window", /* 0x74 */
    "Rename mobile", /* 0x75 */
    "New subserver", /* 0x76 */
    "Update mobile", /* 0x77 */
    "Draw object", /* 0x78 */
    "Get resource", /* 0x79 */
    "Resource data", /* 0x7a */
    "Sequence", /* 0x7b */
    "Open dialog box|Pick object", /* 0x7c */
    "Response to dialog box|Picked object", /* 0x7d */
    "GOD: Get god view data", /* 0x7e */
    "GOD: God view data", /* 0x7f */
    "Account login request", /* 0x80 */
    "Account login ok", /* 0x81 */
    "Account login failed", /* 0x82 */
    "Account delete character", /* 0x83 */
    "Change account password", /* 0x84 */
    "Change character response", /* 0x85 */
    "Characters list", /* 0x86 */
    "Send resources", /* 0x87 */
    "Open paperdoll", /* 0x88 */
    "Corpse clothing", /* 0x89 */
    "Edit trigger", /* 0x8a */
    "Show sign", /* 0x8b */
    "Relay to game server", /* 0x8c */
    "UNUSED PACKET 3", /* 0x8d */
    "Move character", /* 0x8e */
    "UNUSED PACKET 4", /* 0x8f */
    "Open map plot", /* 0x90 */
    "Login to game server request", /* 0x91 */
    "Update multi", /* 0x92 */
    "Open book", /* 0x93 */
    "Alter skill", /* 0x94 */
    "Dye window", /* 0x95 */
    "GOD: Monitor game", /* 0x96 */
    "Player move", /* 0x97 */
    "Alter mobile name", /* 0x98 */
    "Targeting cursor for multi", /* 0x99 */
    "Console entry prompt|Text entry", /* 0x9a */
    "Request GM assistance|Page a GM", /* 0x9b */
    "Assitance response", /* 0x9c */
    "GM Single", /* 0x9d */
    "Sell list", /* 0x9e */
    "Sell reply", /* 0x9f */
    "Select server", /* 0xa0 */
    "Update current health", /* 0xa1 */
    "Update current mana", /* 0xa2 */
    "Update current stamina", /* 0xa3 */
    "Spy on client|Hardware info", /* 0xa4 */
    "Open URL", /* 0xa5 */
    "Tips/notices window", /* 0xa6 */
    "Request tips/notices", /* 0xa7 */
    "Game servers list", /* 0xa8 */
    "List characters and starting cities", /* 0xa9 */
    "OK/Not ok to attack", /* 0xaa */
    "Gump text entry dialog", /* 0xab */
    "Gump text entry response", /* 0xac */
    "Unicode speech (eeew 12 bit!)", /* 0xad */
    "Unicode server speech", /* 0xae */
    "Death animation", /* 0xaf */
    "Generic gump dialog", /* 0xb0 */
    "Generic gump choice", /* 0xb1 */
    "Chat message", /* 0xb2 */
    "Chat text", /* 0xb3 */
    "Target object list", /* 0xb4 */
    "Chat window", /* 0xb5 */
    "Request popup help", /* 0xb6 */
    "Display popup help", /* 0xb7 */
    "Request character profile", /* 0xb8 */
    "Enable T2A/LBR features", /* 0xb9 */
    "Quest arrow", /* 0xba */
    "Ultima messenger", /* 0xbb */
    "Season change", /* 0xbc */
    "Client version message", /* 0xbd */
    "Assist version", /* 0xbe */
    "General information", /* 0xbf */
    "Play hued graphical effect", /* 0xc0 */
    "Predefined client messages", /* 0xc1 */
    "Unicode text entry", /* 0xc2 */
    "GQ request (whatever that is)", /* 0xc3 */
    "Semi visible", /* 0xc4 */
    "Invalid map", /* 0xc5 */
    "Invalid map enable", /* 0xc6 */
    "3D particle effect", /* 0xc7 */
    "Update range change", /* 0xc8 */
    "Trip time", /* 0xc9 */
    "UTrip time", /* 0xca */
    "GQ count (eeps)", /* 0xcb */
    "Text ID and string", /* 0xcc */
    "Unused packet", /* 0xcd */
    "Unknown draw?", /* 0xce */
    "IGR Account login request", /* 0xcf */
    "Configuration File", /* 0xd0 */
    "Logout status", /* 0xd1 */
    "Extended Draw game player", /* 0xd2 */
    "Extended Draw object", /* 0xd3 */
    "Open book", /* 0xd4 */
    "Bogus packet", /* 0xd5 */
    "Property list content", /* 0xd6 */
    "Fight book/system", /* 0xd7 */
    "Custom house data", /* 0xd8 */
    "Improved system info", /* 0xd9 */
    "Mahjong board dialog", /* 0xda */
    "Character transfer data", /* 0xdb */
    "Equipment Description", /* 0xdc */
    "", /* 0xdd */
    "", /* 0xde */
    "", /* 0xdf */
    "", /* 0xe0 */
    "", /* 0xe1 */
    "", /* 0xe2 */
    "", /* 0xe3 */
    "", /* 0xe4 */
    "", /* 0xe5 */
    "", /* 0xe6 */
    "", /* 0xe7 */
    "", /* 0xe8 */
    "", /* 0xe9 */
    "", /* 0xea */
    "", /* 0xeb */
    "", /* 0xec */
    "", /* 0xed */
    "", /* 0xee */
    "", /* 0xef */
    "Custom client packet", /* 0xf0 */
    "", /* 0xf1 */
    "", /* 0xf2 */
    "", /* 0xf3 */
    "", /* 0xf4 */
    "", /* 0xf5 */
    "", /* 0xf6 */
    "", /* 0xf7 */
    "", /* 0xf8 */
    "", /* 0xf9 */
    "", /* 0xfa */
    "", /* 0xfb */
    "", /* 0xfc */
    "", /* 0xfd */
    "", /* 0xfe */
    "", /* 0xff */ /* this is what the buffer returns when the socket is closed, 4 bytes */
};

int PacketLengths[256] =
{
	/* 0x00 */ 0x0068, 0x0005, 0x0007, 0x8000, 0x0002, 0x0005, 0x0005, 0x0007, 0x000E, 0x0005, 0x000B, 0x010A, 0x8000, 0x0003, 0x8000, 0x003D,
	/* 0x10 */ 0x00D7, 0x8000, 0x8000, 0x000A, 0x0006, 0x0009, 0x0001, 0x8000, 0x8000, 0x8000, 0x8000, 0x0025, 0x8000, 0x0005, 0x0004, 0x0008,
	/* 0x20 */ 0x0013, 0x0008, 0x0003, 0x001A, 0x0007, 0x0014, 0x0005, 0x0002, 0x0005, 0x0001, 0x0005, 0x0002, 0x0002, 0x0011, 0x000F, 0x000A,
	/* 0x30 */ 0x0005, 0x0001, 0x0002, 0x0002, 0x000A, 0x028D, 0x8000, 0x0008, 0x0007, 0x0009, 0x8000, 0x8000, 0x8000, 0x0002, 0x0025, 0x8000,
	/* 0x40 */ 0x00C9, 0x8000, 0x8000, 0x0229, 0x02C9, 0x0005, 0x8000, 0x000B, 0x0049, 0x005D, 0x0005, 0x0009, 0x8000, 0x8000, 0x0006, 0x0002,
	/* 0x50 */ 0x8000, 0x8000, 0x8000, 0x0002, 0x000C, 0x0001, 0x000B, 0x006E, 0x006A, 0x8000, 0x8000, 0x0004, 0x0002, 0x0049, 0x8000, 0x0031,
	/* 0x60 */ 0x0005, 0x0009, 0x000F, 0x000D, 0x0001, 0x0004, 0x8000, 0x0015, 0x8000, 0x8000, 0x0003, 0x0009, 0x0013, 0x0003, 0x000E, 0x8000,
	/* 0x70 */ 0x001C, 0x8000, 0x0005, 0x0002, 0x8000, 0x0023, 0x0010, 0x0011, 0x8000, 0x0009, 0x8000, 0x0002, 0x8000, 0x000D, 0x0002, 0x8000,
	/* 0x80 */ 0x003E, 0x8000, 0x0002, 0x0027, 0x0045, 0x0002, 0x8000, 0x8000, 0x0042, 0x8000, 0x8000, 0x8000, 0x000B, 0x8000, 0x8000, 0x8000,
	/* 0x90 */ 0x0013, 0x0041, 0x8000, 0x0063, 0x8000, 0x0009, 0x8000, 0x0002, 0x8000, 0x001A, 0x8000, 0x0102, 0x0135, 0x0033, 0x8000, 0x8000,
	/* 0xa0 */ 0x0003, 0x0009, 0x0009, 0x0009, 0x0095, 0x8000, 0x8000, 0x0004, 0x8000, 0x8000, 0x0005, 0x8000, 0x8000, 0x8000, 0x8000, 0x000D,
	/* 0xb0 */ 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x0040, 0x0009, 0x8000, 0x8000, 0x0003, 0x0006, 0x0009, 0x0003, 0x8000, 0x8000, 0x8000,
	/* 0xc0 */ 0x0024, 0x8000, 0x8000, 0x8000, 0x0006, 0x00CB, 0x0001, 0x0031, 0x0002, 0x0006, 0x0006, 0x0007, 0x8000, 0x0001, 0x8000, 0x004E,
	/* 0xd0 */ 0x8000, 0x0002, 0x0019, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x010C, 0x8000, 0x8000, 0x0009, 0x0000, 0x0000, 0x0000,
	/* 0xe0 */ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	/* 0xf0 */ 0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0004
};


unsigned int GetPacketLen(unsigned char *Buf, unsigned int Size)
{
	unsigned int PacketLen = PacketLengths[Buf[0]&0xff];
	if(PacketLen >= 0x8000){
	 if(Size < 3)	return Size;
	 else return ((unsigned int)Buf[1] << 8 | (unsigned int)Buf[2]);
	}
	return PacketLen;
}

// Enviar loguinseed e loguin request
void uo_sendloguin(char *loguin, char *senha)
{
	unsigned char buf[70];

	//LogPrint(3, "[ARG] (%s) (%s)\n", loguin, senha); 
	// LoginSeed (192.168.1.31)
	buf[0] = 0xc0;
	buf[1] = 0xa8;
	buf[2] = 0x01;
	buf[3] = 0x1f;
	// Enviando LoginSeed(4 bytes)
	send_server(buf, 4);

	// Enviando loguin
	memset(buf, 0x00, sizeof(buf));
	buf[0] = 0x80;
	// Loguin
	strcpy(buf+1, loguin);
	// Senha
	strcpy(buf+31, senha);
	buf[61] = 0xFF;
	send_server(buf, 62);
}

/* -------------------- HANDLES -------------------- */

int uo_handle_servers_list(char *buf, int len) // 0xa8 Game servers list
{
	unsigned char buff[3];
	unsigned short numshards=0;
	numshards = unpack16(buf+4);
	// Send Select(0x01) server
	buff[0] = 0xa0;
	buff[1] = 0x00;
	buff[2] = 0x01;
	send_server(buff, 3);
	return 0;
}

int uo_handle_server_speech(char *buf, int len) // 0x1c
{
	unsigned char *buff=buf;
	GameObject *TmpObj;
	unsigned int serial=unpack32(buf+3);
	if(buf[14]==0 || buf[44] == 0)
		return 0;

	if(buf[9] == 0)
		LogPrint(1, "[SPEECH] [%s] [%s]\n", buff+14, buff+44);
	else if(buf[9] == 1)
		LogPrint(1, "[SYSMSG] [%s] [%s]\n", buff+14, buff+44);
	else if(buf[9] == 6)
		LogPrint(1, "[YOUSEE] [%s]\n", buff+44);
	else if(buf[9] == 10)
		LogPrint(1, "[CAST] [%s] [%s]\n", buff+14, buff+44);
	else
		LogPrint(1, "[SPEECH] [%d] [%s] [%s]\n",buf[9], buff+14, buff+44);

	if((buff[9] & 0x0F) == JOURNAL_YOUSEE)
	{
		JournalAdd("You see", buff+44, buff[9] & 0x0F);
		if((TmpObj=FindSerial(serial))!=0)
		{
			strncpy(TmpObj->Name, buff+44, 30);
			TmpObj->Name[30] = 0;
		}
	}
	else JournalAdd(buff+14, buff+44, buff[9] & 0x0F);

	if(!strcmp(buf+14, "System"))
	{
		if(strstr(buf+44, "Salvando"))
		{
			if(PluginThreadSuspended==FALSE)
			{
				LogPrint(3, "[SAVE] Suspendendo os plugins\n");
				SuspendAllPThreads();
				PluginThreadSuspended = TRUE;
			}
		}
		else if(strstr(buf+44, "salvo."))
		{
			if(PluginThreadSuspended==TRUE)
			{
				LogPrint(3, "[SAVE] Retomando os plugins\n");
				ResumeAllPThreads();
				PluginThreadSuspended = FALSE;
			}
		}
	}

	if(buf[9] == 0 || buf[9] == 6)
	{
		if((TmpObj = FindSerial(serial)))
		{
			if(strstr(TmpObj->Name, "Staff"))
			{
				LogPrint(3, "[SPEECH] [%s] [%s]\n", buff+14, buff+44);

				if(LastStaffCheck && time(0) - LastStaffCheck < 50)
				{
					LastStaffCheck = time(0);
					return 0;
				}
				LastStaffCheck = time(0);

				LogPrint(3, "[ALERTA] Algum staff falou algo perto de vc. Se ligue ingame\n");
				
				if(AntiStaff)
				{
					if(hThread)
						SuspendThread(hThread);
					SuspendAllPThreads();
					ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
					MBOut("Alerta", "Algum staff falou algo perto de vc\nSe ligue ingame\n");
					if(hThread)
						ResumeThread(hThread);
					ResumeAllPThreads();
				}
				else
				{
					ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
				}
				
				return 0;
			}
		}
				
		if(strstr(buf+14, "Staff"))
		{
			LogPrint(3, "[SPEECH] [%s] [%s]\n", buff+14, buff+44);
			
			if(LastStaffCheck && time(0) - LastStaffCheck < 50)
			{
				LastStaffCheck = time(0);
				return 0;
			}
			LastStaffCheck = time(0);

			LogPrint(3, "[ALERTA] Algum staff falou algo perto de vc. Se ligue ingame\n");
			if(AntiStaff)
			{
				if(hThread)
					SuspendThread(hThread);
				SuspendAllPThreads();
				ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
				MBOut("Alerta", "Algum staff falou algo perto de vc\nSe ligue ingame\n");
				if(hThread)
					ResumeThread(hThread);
				ResumeAllPThreads();
			}
			else
				ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);

			return 0;
		}
	}
	
	return 0;
}


// Relay to game server
int uo_handle_relay(char *buf, int len) // 0x8C
{
	//char addr[30], port[10];
	unsigned char buff[80];
	LogPrint(3, "[NET] Closed socket %d\n", sockshard);
	closesocket(sockshard);
	//sprintf(addr, "%u.%u.%u.%u", *(buf+1)&0xff, *(buf+2)&0xff, *(buf+3)&0xff, *(buf+4)&0xff);
	//sprintf(port, "%s", ntohs(unpack16(buf+5)));
	LogPrint(3, "[NET] Reconnecting to (%s:%d)\n", SHARDADDR, SHARDPORT);
gtrycon:
	if((sockshard=net_connect(SHARDADDR, SHARDPORT))==SOCKET_ERROR)
	{
		LogPrint(3, "[ERROR] Nao foi possivel recriar o socket\n");
		Sleep(2500);
		goto gtrycon;
	}
	// Enviar seed
	firstpacket = 1;
	send_server(buf+7, 4);
	// Enviar loguin/senha
	memset(buff, 0x00, sizeof(buff));
	buff[0] = 0x91;
	memcpy(buff+1, buf+7, 4);
	strcpy(buff+5, LOGUIN);
	strcpy(buff+35, SENHA);
	send_server(buff, 65);
	LoguinSeed = 0;
	return 0;
}

int uo_handle_list_chars(char *buf, int len) // 0xA9
{
	unsigned char buff[75],*name=buf+4;
	// Selecionar o personagem
	memset(buff, 0x00, sizeof(buff));
	buff[0] = 0x5d;
	buff[1] = 0xed;
	buff[2] = 0xed;
	buff[3] = 0xed;
	buff[4] = 0xed;
	while(1)
	{
		if(name>=buf+len)
		{
			LogPrint(3, "[ERROR] Nao foi possivel achar char (%s)\n", CHARNAME);
			CloseShardSocket();
			return 0;
		}
		while(*name==0) name++;
		if(strstr(name, CHARNAME)!=NULL) break;
		name+=strlen(name);
	}
	memcpy(&buff[5], name, 30); // Nome
	strcpy(CHARNAME, name);
	buff[0x28] = 0x03;
	buff[0x2D] = 0x04;
	buff[0x45] = 0xc0;
	buff[0x46] = 0xa8;
	buff[0x47] = 0x01;
	buff[0x48] = 0x1f;
	buff[68] = (name-buf+4)/60;
	send_server(buff, 73);
	return 0;
}

int uo_handle_loguin_confirm(char *buf, int len) // 0x1B
{
	unsigned char buff[15];
	GameObject *gobj=&gobjects;
	unsigned int serial = unpack32(buf+1);
	unsigned int cont=0;

	gobj=AddObj(serial);
	gobj->Graphic = unpack16(buf+9);
	gobj->X = unpack16(buf+11);
	gobj->Y = unpack16(buf+13);
	gobj->Z = (char)unpack16(buf+15);
	gobj->Direction = buf[17]&7;
	mserial = serial;
	LogPrint(3, "[SERIAL] S:[0x%X] G:[0x%X] X:[%d] Y:[%d] Z:[%d] D:[%s]\n",
		gobj->Serial, gobj->Graphic, gobj->X, gobj->Y, gobj->Z, DirNames[gobj->Direction].Name);

	// Setar o pickupobj = 0
	//pickupobj.Serial = 0;
	CleanJournal();

	// Enviar get object status meu.
	buff[0] = 0x34;
	buff[1] = 0xed;
	buff[2] = 0xed;
	buff[3] = 0xed;
	buff[4] = 0xed;
	buff[5] = 0x04;
	memcpy(buff+6, buf+1, 4); //Copiar o meu serial
	send_server(buff, 10); 

	//LogPrint((PrintDebug ? 3:0), "[TESTE] Chegay aki\n");
	return 0;
}

int uo_handle_loguin_complete(char *buf, int len) //0x55
{
	unsigned char buff[50];
	unsigned short vers=strlen(VERSION)+4;
	//Enviar a versao do client
	buff[0] = 0xbd;
	pack16(&buff[1], vers);
	strcpy(&buff[3], VERSION);
	buff[3+vers] = '\0';
	send_server(buff, vers);
	IsConnected = 1;
	IsSkillUpdated = 0;
    RecvTarget = 0;
	GumpDone = 0;

	LoginTime = time(0);

	if((hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)uo_thread_ping, (LPVOID)&send_server, 0, (LPDWORD)&hThread))==0){
		LogPrint(3, "[ERROR] Nao foi possivel criar a thread\n");
		exit(0);
	}
//	LogPrint(3, "[THREAD] Criada PingThread %d\n", hThread);
	if(CmdThreadsCount)
	{
		LogPrint(3, "[THREAD] Resumindo %d threads\n", CmdThreadsCount);
		ResumeAllPThreads();
	}
	return 0;
}


// Inutil!
int uo_handle_ping(char *buf, int len) // 0x73
{
	send_server(buf, 2);
	return 0;
}

int uo_handle_open_gump(char *buf, int len) //0xB0 ANTI-MACRO?
{
	char opa[2048];
	struct gmrecv amk[10];
	struct gumprecv gmr;
	char *line,*Line=0;
	int x=0,i=0,LineSize=0;
	int gx,gy;
	char temp[200];

	int RandTime = 2000;

	typedef struct {
		char *Text;
	} LineType;

	LineType *Lines=0;

	srand(time(0));

	RandTime += (rand()%6)*1000;

	gmr.cmd = buf[0]&0xff;
	gmr.dtlen = unpack16(buf+1);
	gmr.serial = unpack32(buf+3);
	gmr.tipo = unpack32(buf+7);
	gmr.unk1 = unpack32(buf+11);
	gmr.unk2 = unpack32(buf+15);
	gmr.laylen = unpack16(buf+19);
	gmr.lay = (char *)buf+21;
	gmr.nlines = unpack16(buf+21+gmr.laylen);
	gmr.linelen = unpack16(buf+21+gmr.laylen+2);
	gmr.lines = (char *)buf+21+gmr.laylen+2;


	memset(temp, 0x00, sizeof(temp));

	Line = gmr.lines;

	Lines = malloc(sizeof(LineType)*gmr.nlines);

	gmr.lay[gmr.laylen] = 0;

	/* Reinterpretar todas as linhas, transformando em ascii */
	for(x=0; x < gmr.nlines; x++)
	{
		/* Tamanho da linha unicode */
		LineSize = unpack16(Line);
		/* Alocar espaco para o texto em ascii */
		Lines[x].Text = malloc(LineSize+1);
		/* Transformar o texto em ascii e guarda na struct */
		UnicodeToAscii(Line+2, LineSize*2, Lines[x].Text);
		/* Proxima linha */
		Line += 2+(LineSize*2);
	}

	memset(opa, 0x00, sizeof(opa));

	if(strstr(Lines[0].Text, "Marcar")!=0 && AceitarCoisas)
	{
		LogPrint(1, "[GUMP] Responder automatico ao marcar item\n");
		opa[0] = (char)0xb1;
		pack16(opa+1, 0x17);
		memcpy(opa+3, buf+3, 8);
		if((line = strstr(gmr.lay, "2129 1 0 "))==NULL){
			LogPrint(3, "[RESPGUMP] Nao achei botao sim\n");
			exit(-1);
		}
		line+=9;
		chartoint32(line, 0x20, &x);
		pack16(opa+11, (unsigned short)x);

        /* Liberar espaco alocado pelo gump */
        for(x=0; x<gmr.nlines; x++)
            free(Lines[x].Text);
        free(Lines);

		send_server(opa, 0x17);
		return 0;
	}

	else if(strstr(Lines[0].Text, "gate?")!=0 && AceitarCoisas)
	{
		LogPrint(1, "[GUMP] Responder automatico ao entrar no gate\n");
		opa[0] = (char)0xb1;
		pack16(opa+1, 0x17);
		memcpy(opa+3, buf+3, 8);
		if((line = strstr(gmr.lay, "2129 1 0 "))==NULL){
			LogPrint(3, "[RESPGUMP] Nao achei botao sim\n");
			exit(-1);
		}
		line+=9;
		chartoint32(line, 0x20, &x);
		pack16(opa+11, (unsigned short)x);

        /* Liberar espaco alocado pelo gump */
        for(x=0; x<gmr.nlines; x++)
            free(Lines[x].Text);
        free(Lines);

		send_server(opa, 0x17);
		return 0;
	}


	else if(strstr(Lines[0].Text, "Aceitar")!=0 && AceitarCoisas)
	{
		LogPrint(1, "[GUMP] Responder automatico ao aceitar animal\n");
		opa[0] = (char)0xb1;
		pack16(opa+1, 0x17);
		memcpy(opa+3, buf+3, 8);
		if((line = strstr(gmr.lay, "2129 1 0 "))==NULL){
			LogPrint(3, "[RESPGUMP] Nao achei botao sim\n");
			exit(-1);
		}
		line+=9;
		chartoint32(line, 0x20, &x);
		pack16(opa+11, (unsigned short)x);

        /* Liberar espaco alocado pelo gump */
        for(x=0; x<gmr.nlines; x++)
            free(Lines[x].Text);
        free(Lines);

		send_server(opa, 0x17);
		return 0;
	}

	else if((gmr.nlines==4)&&(!strcmp(Lines[0].Text, "Ajuda da Mystic Second Age")))
	{
		LogPrint(3, "[SEESTAFF] Tem staffs logados no shard\n");

        /* Liberar espaco alocado pelo gump */
        for(x=0; x<gmr.nlines; x++)
            free(Lines[x].Text);
        free(Lines);

		if(DetectStaff)
		{
			ErrNum = 3;
			Plugin_Done(ErrNum);
			CmdKillAll();
			RemoveAllObjs();
			if(hThread)
			{
				TerminateThread(hThread, 0);
				hThread = 0;
			}
			LogClose();
			CloseShardSocket();

			IsConnected = 0;
			DetectStaff = 0;
			TerminateThread(DetectStaffHandle, 0);
			TerminateThread(hMainThread, 0);
			hMainThread = 0;
		}

		return 0;
	}

	else if((gmr.nlines==34)&&(!strcmp(Lines[0].Text, "Ajuda da Mystic Second Age")))
	{
		unsigned char StaffPacket[23];
		memset(StaffPacket, 0, 23);
		StaffPacket[0] = 0xb1;
		StaffPacket[2] = 0x17;
		memcpy(StaffPacket+3, buf+3, 8);

		if(CancelPage)
		{
			/* cancelar as pages */
			StaffPacket[14] = 0x03;
			send_server(StaffPacket, 23);
			CancelPage = 0;
			return 0;
		}

		if(SeeStaffType == 1)
		{
			unsigned char HelpPacket[258];

			/* enviar help de novo para verificar gms */
			memset(HelpPacket, 0, 258);
			HelpPacket[0] = 0x9b;

			/* selecionar conselheiro */
			StaffPacket[14] = 0x01;
			SeeStaffType = 2;
			send_server(StaffPacket, 23);

			send_server(HelpPacket, 258);
		}
		else if(SeeStaffType == 2)
		{
			/* selecionar gms */
			StaffPacket[14] = 0x02;
			SeeStaffType = 0;
			send_server(StaffPacket, 23);
		}

        /* Liberar espaco alocado pelo gump */
        for(x=0; x<gmr.nlines; x++)
            free(Lines[x].Text);
        free(Lines);

		return 0;
	}

	else if(RespAm && (gmr.nlines==1)&&(!strcmp(Lines[0].Text, "ANTI-MACRO"))){
		LogPrint(3, "[ANTIMACRO] Botao dourado do anti-macro\n");
		//JournalAdd("anti-macro", "anti-macro", -2);
		AntiMacro++;

		if(AntiMacro>1)
		{
			if(LastAm)
			{
				if(time(0) - LastAm < 1800)
				{
					if(LastStaffCheck && time(0) - LastStaffCheck < 50)
					{
						LastStaffCheck = time(0);
					}
					else
					{
						LastStaffCheck = time(0);
						LogPrint(3, "[LASTAM] Cuidado!! Possivel am enviado por staff! Ultimo anti-macro foi ha [%d] minutos\n", (time(0) - LastAm)/60);
						LogPrint(3, "[LASTAM] Para desativar o respondedor de anti-macro, use: /setrespam off\n");
						//MBOut("Alerta", "Possivel staff presente ingame\nPara desativar anti-am\nUse: /setrespam off\n");
						ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
					}
				}
			}
			LastAm = time(0);
		}

		opa[0] = (char)0xb1;
		pack16(opa+1, 0x17);
		memcpy(opa+3, buf+3, 8);
		if((line = strstr(gmr.lay, "6 1 0 "))==NULL){
			LogPrint(3, "[ANTIMACRO] Nao consegui achar o id, am mudou?");
			exit(-1);
		}
		line+=6;
		chartoint32(line, 0x20, &x);
		pack16(opa+11, (unsigned short)x);
        /* Liberar espaco alocado pelo gump */
        for(x=0; x<gmr.nlines; x++)
            free(Lines[x].Text);
        free(Lines);
		Thread_SendServer(opa, 0x17, RandTime);
		return 0;
	} 
	else if(RespAm && (gmr.nlines==21)&&(!strcmp(Lines[1].Text, "Este eh um teste de"))){
		LogPrint(3, "[ANTIMACRO] Tela com numeros do anti-macro\n");
		opa[0] = (char)0xb1;
		pack16(opa+1, 0x001b);
		memcpy(opa+3, buf+3, 8);
		if((line=strstr(gmr.lay, "gumppic "))==NULL){
			LogPrint(3, "[ANTIMACRO] Nao consegui achar gumppic, am mudou?");
			exit(-1);
		}
		line+=8;
		i=chartoint32(line, 0x20, &gx);
		line += (i+1);
		chartoint32(line, 0x20,&gy);
		i = strlen((Lines[9].Text+8));
		if(i>=4){
			if((line=strstr(gmr.lay, "2128 2129"))==NULL){
				LogPrint(3, "[ANTIMACRO] Nao consegui achar id botao OKAY, am mudou?");
				exit(-1);
			}
			line += 14;
			chartoint32(line, 0x20, &len);
			pack32(opa+11, len);
			goto m5;
		}
		for(x=0;x<10;++x){
			amk[x].x = am[x].x + gx;
			amk[x].y = am[x].y + gy;
			sprintf(temp, "%d %d 5230", amk[x].x, amk[x].y);
			if((line=strstr(gmr.lay, temp))==NULL){
				LogPrint(3, "[ANTIMACRO] Nao consegui achar x,y"/*: %d,%d,*/" am mudou?"/*, amk[a].x, amk[a].y*/);
				exit(-1);
			}
			line+=22;
			chartoint32(line, 0x20, &amk[x].id);
			amk[x].num = (int)Lines[11+x].Text[0];
			if(amk[x].num == (int)Lines[4+i].Text[0]){
				pack32(opa+11, amk[x].id);
			}
		}
m5:
		line=buf+22;
		*line = 0x01;
		line += 2;
		*line = 0x02;

        /* Liberar espaco alocado pelo gump */
        for(x=0; x<gmr.nlines; x++)
            free(Lines[x].Text);
        free(Lines);

		Thread_SendServer(opa, 0x1b, RandTime);
		return 0;
	}
	else
    {
        /* Liberar espaco alocado pelo gump */
        for(x=0; x<gmr.nlines; x++)
            free(Lines[x].Text);
        free(Lines);

		uo_handle_gump(buf, len);
    }
	return 0;
}


int uo_handle_draw_player(char *buf, int len) // 0x20
{
	unsigned int x=0;
	unsigned int serial=unpack32(buf+1);
	GameObject *gobj=&gobjects;
	unsigned char buff[30];
	if(UltimaMessenger){
		// Enviar Ultima Messenger
		buff[0] = 0xbb;
		memcpy(&buff[1], buf+1, 4);
		memcpy(&buff[5], buf+1, 4);
		send_server(buff, 9);
		memset(buff, 0x00, sizeof(buff));
		buff[0] = 0xbf;
		buff[2] = 0x0d;
		buff[4] = 0x05;
		buff[7] = 0x02;
		buff[8] = 0x80;
		send_server(buff, 13);
		UltimaMessenger = 0;
		return 0;
	}
	gobj=AddObj(serial);
	gobj->Graphic = unpack16(buf+5);
	gobj->Color = unpack16(buf+8);
    gobj->Flags = buf[10];
	gobj->X = unpack16(buf+11);
	gobj->Y = unpack16(buf+13);
	gobj->Direction = buf[17]&0x7;
	gobj->Z = buf[18]&0xff;
	return 0;
}

int uo_handle_error_code(char *buf, int len) // 0x53
{
	unsigned char buff[5];
	// Responde idle msg
	memset(buff, 0x00, sizeof(buff));
	buff[0] = 0x09;
	Thread_SendServer(buff, 5, 5000);
	return 0;
}

int uo_handle_draw_object(char *buf, int len) // 0x78
{
	unsigned int serial = unpack32(buf+3);
	unsigned int ItemSerial;
	unsigned char GetInfo[20];
	GameObject *TmpObj=FindSerial(serial&0x7FFFFFFF);
	GameObject *Obj=FindSerial(serial&0x7FFFFFFF);
	if(TmpObj==0) 
		TmpObj=AddCharacter(serial&0x7FFFFFFF);
	TmpObj->Graphic = unpack16(buf+7);

	if(PrintDebug)
		LogPrint(3, "[DRAW] Adicionado objeto s[0x%X] g[0x%X]\n", TmpObj->Serial, TmpObj->Graphic);

	if(TmpObj->Graphic == 0x03DB)
	{
		if(LastStaffCheck && time(0) - LastStaffCheck < 50)
		{
			LastStaffCheck = time(0);
		}
		else
		{
			LastStaffCheck = time(0);

			LogPrint(3, "[ANTISTAFF] Detectado presenca de staff, serial: [0x%X]\n", TmpObj->Serial);
			ShellExecute(0, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
			MessageBox(0, "Algum staff apareceu pra vc\nSe ligue ingame\n", "ANTI-STAFF", 0);
		}
	}

	if(serial&0x80000000)
	{
		TmpObj->Color = unpack16(buf+9);
		buf+=2;
	}
	TmpObj->Serial &= 0x7FFFFFFF;
	TmpObj->X = unpack16(buf+9);
	TmpObj->Y = unpack16(buf+11);
	if(TmpObj->X&0x8000)
	{
		TmpObj->Direction = buf[13]&7;
		buf++;
	}
	TmpObj->Z = *(buf+13)&0xff;
	TmpObj->Color = unpack16(buf+15);
	TmpObj->Flags = *(buf+17);
	TmpObj->X &= 0x7fff;
	TmpObj->Y &= 0x3fff;
	buf+=19;
	if(TmpObj->Graphic)
	{
		GetInfo[0] = 0x34;
		pack32(GetInfo + 1, 0xedededed);
		GetInfo[5] = 0x04; /* type: get basic stats */
		pack32(GetInfo + 6, TmpObj->Serial);
		//LogPrint((PrintDebug ? 3: 0), "[DRAW] Player %#X x:%d y:%d c:%d g:%#x\n",
		//	TmpObj->Serial, TmpObj->X, TmpObj->Y, TmpObj->Color, TmpObj->Graphic);
		send_server(GetInfo, 10);
	}
	while(*((unsigned int*)buf) != 0)
	{
		ItemSerial = unpack32(buf)&0x7FFFFFFF;
		Obj = AddObj(ItemSerial);
		Obj->Container = serial;
		Obj->Graphic = unpack16(buf+4);
		Obj->Layer = buf[6]&0xff;
		if(Obj->Graphic & 0x8000)
		{
			Obj->Color = unpack16(buf+7);
			buf+=2;
		}
		Obj->Graphic &= 0x7FFF;
		if(Obj->Graphic==0x190 || Obj->Graphic==0x191)
		{
			GetInfo[0] = 0x34;
			pack32(GetInfo + 1, 0xedededed);
			GetInfo[5] = 0x04; /* type: get basic stats */
			pack32(GetInfo + 6, Obj->Serial);
			//LogPrint((PrintDebug ? 3: 0), "[ADDCHARACTER] 0x78-2 Draw request info: %#X\n", TmpObj->Serial);
			send_server(GetInfo, 10);
		}
		else if(CheckCont(Obj->Graphic))
		{
			Obj->IsContainer = TRUE;
			UseObject(Obj->Serial);
		}

		if((TmpObj->Graphic==0x190 || TmpObj->Graphic==0x191) && Obj->Graphic&0x7FFF == 0x204F)
		{
			if(LastStaffCheck && time(0) - LastStaffCheck < 50)
			{
				LastStaffCheck = time(0);
			}
			else
			{
				LastStaffCheck = time(0);

				LogPrint(3, "[ANTISTAFF] Detectado presenca de staff, serial: [0x%X]\n", TmpObj->Serial);
				ShellExecute(0, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
				MessageBox(0, "Algum staff apareceu pra vc\nSe ligue ingame\n", "ANTI-STAFF", 0);
			}

		}
		if(serial==mserial && Obj->Layer == LAYER_BACKPACK)
		{
			mbackpack = ItemSerial;
			//LogPrint((PrintDebug ? 3: 0), "[DRAW] Player 0x%08x with backpack 0x%08x\n", mserial, mbackpack);
			UseObject(mbackpack);
			if(!IsSkillUpdated)
			{
				GetInfo[0] = 0x34;
				pack32(GetInfo + 1, 0xedededed);
				GetInfo[5] = 0x05; /* type: get skills stats */
				pack32(GetInfo + 6, mserial);
				LogPrint(3, "[SKILLS] Requisitando skills\n");
				send_server(GetInfo, 10);
				IsSkillUpdated = 1;
			}
		}
		buf+=7;
	}
	return 0;
}

int uo_handle_delete(char *buf, int len) // 0x1D
{
	unsigned int serial=unpack32(buf+1);
	RemoveObj(serial);
	//LogPrint((PrintDebug ? 3:0), "[DELOBJ] Removing %#X\n", serial);
	return 0;
}

int uo_handle_update_item(char *buf, int len) // 0x1A
{
	unsigned int serial=unpack32(buf+3);
	GameObject *TmpObj=AddObj(serial&0x7FFFFFFF);
	unsigned short Graphic = unpack16(buf+7);
	TmpObj->Graphic = Graphic&0x7FFF;
	if(TmpObj->Graphic == 0x03DB)
	{
		if(LastStaffCheck && time(0) - LastStaffCheck < 50)
		{
			LastStaffCheck = time(0);
		}
		else
		{
			LastStaffCheck = time(0);

			LogPrint(3, "[ANTISTAFF] Detectado presenca de staff, serial: [0x%X]\n", TmpObj->Serial);
			ShellExecute(0, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
			MessageBox(0, "Algum staff apareceu pra vc\nSe ligue ingame\n", "ANTI-STAFF", 0);
		}
	}

    if(serial&0x80000000)
	{
		if(Graphic!=0x620)
		{
			TmpObj->Quantity = unpack16(buf+9);
			buf+=2;
		}
	}
	if(Graphic&0x8000)
		buf++;
	TmpObj->X = unpack16(buf+9);
	TmpObj->Y = unpack16(buf+11);
	if(TmpObj->X&0x8000)
		buf++;
	TmpObj->Z = buf[13]&0xFF;
	if(TmpObj->Y&0x8000)
		TmpObj->Color = unpack16(buf+14);
	TmpObj->X &=0x7FFF;
	TmpObj->Y &=0x3FFF;
	return 0;
}

int uo_handle_updatehp(char *buf, int len)
{
	unsigned int serial=unpack32(buf+1);
	char TitleText[300];
	GameObject *TmpObj=FindSerial(serial);
	if(TmpObj==0) AddObj(serial);
	TmpObj = AddCharacter(serial);
	TmpObj->Character->HitPoints = unpack16(buf+7);
	TmpObj->Character->MaxHitPoints = unpack16(buf+5);
	if(MainWnd && serial == mserial)
	{
		sprintf(TitleText, "UoBot - %s (Mystic) Hp:%d/%d Mana:%d/%d Stam:%d/%d Peso:%d Ar:%d",
		TmpObj->Name, TmpObj->Character->HitPoints, TmpObj->Character->MaxHitPoints,
		TmpObj->Character->Mana, TmpObj->Character->MaxMana, TmpObj->Character->Stamina, TmpObj->Character->MaxStamina,
		TmpObj->Character->Weight, TmpObj->Character->Armor);
		SetWindowText(MainWnd, TitleText);
	}
	//LogPrint(1,"[UPDATE] HP %s(%#x) change to %d/%d\n", TmpObj->Character->Name, TmpObj->Serial, TmpObj->Character->HitPoints, TmpObj->Character->MaxHitPoints);
	return 0;
}

int uo_handle_updatemana(char *buf, int len)
{
	unsigned int serial=unpack32(buf+1);
	char TitleText[300];
	GameObject *TmpObj=FindSerial(serial);
	if(TmpObj==0) AddObj(serial);
	TmpObj = AddCharacter(serial);
	TmpObj->Character->Mana = unpack16(buf+7);
	TmpObj->Character->MaxMana = unpack16(buf+5);
	if(MainWnd && serial == mserial)
	{
		sprintf(TitleText, "UoBot - %s (Mystic) Hp:%d/%d Mana:%d/%d Stam:%d/%d Peso:%d Ar:%d",
		TmpObj->Name, TmpObj->Character->HitPoints, TmpObj->Character->MaxHitPoints,
		TmpObj->Character->Mana, TmpObj->Character->MaxMana, TmpObj->Character->Stamina, TmpObj->Character->MaxStamina,
		TmpObj->Character->Weight, TmpObj->Character->Armor);
		SetWindowText(MainWnd, TitleText);
	}
	return 0;
}

int uo_handle_updatestam(char *buf, int len)
{
	unsigned int serial=unpack32(buf+1);
	GameObject *TmpObj=FindSerial(serial);
	char TitleText[300];
	if(TmpObj==0) AddObj(serial);
	TmpObj = AddCharacter(serial);
	TmpObj->Character->Stamina = unpack16(buf+7);
	TmpObj->Character->MaxStamina = unpack16(buf+5);
	if(MainWnd && serial == mserial)
	{
		sprintf(TitleText, "UoBot - %s (Mystic) Hp:%d/%d Mana:%d/%d Stam:%d/%d Peso:%d Ar:%d",
		TmpObj->Name, TmpObj->Character->HitPoints, TmpObj->Character->MaxHitPoints,
		TmpObj->Character->Mana, TmpObj->Character->MaxMana, TmpObj->Character->Stamina, TmpObj->Character->MaxStamina,
		TmpObj->Character->Weight, TmpObj->Character->Armor);
		SetWindowText(MainWnd, TitleText);
	}
	return 0;
}

int uo_handle_char_stats(char *buf, int len) // 0x11
{
	unsigned int serial=unpack32(buf+3);
	char TitleText[300];
	GameObject *TmpObj;
	TmpObj = AddCharacter(serial);
	TmpObj->Character->MaxHitPoints = unpack16(buf+39);
	TmpObj->Character->HitPoints = unpack16(buf+37);
	strncpy(TmpObj->Name, (const char*)(buf+7), 30);
	TmpObj->Name[30] = 0;
	if(serial==mserial)
	{
		if(TmpObj->Character->STR != 0 && TmpObj->Character->STR != unpack16(buf+44))
			LogPrint(3, "[STATS] Forca mudou de [%d] para [%d]\n", TmpObj->Character->STR, unpack16(buf+44));
		if(TmpObj->Character->DEX != 0 && TmpObj->Character->DEX != unpack16(buf+44+2))
			LogPrint(3, "[STATS] Dextreza mudou de [%d] para [%d]\n", TmpObj->Character->DEX, unpack16(buf+44+2));
		if(TmpObj->Character->INT != 0 && TmpObj->Character->INT != unpack16(buf+44+4))
			LogPrint(3, "[STATS] Inteligencia mudou de [%d] para [%d]\n", TmpObj->Character->INT, unpack16(buf+44+4));
	}
		
	TmpObj->Character->STR = unpack16(buf+44);
	TmpObj->Character->DEX = unpack16(buf+44+2);
	TmpObj->Character->INT = unpack16(buf+44+4);
	TmpObj->Character->Stamina = unpack16(buf+44+6);
	TmpObj->Character->MaxStamina = unpack16(buf+44+8);
	TmpObj->Character->Mana = unpack16(buf+44+10);
	TmpObj->Character->MaxMana = unpack16(buf+44+12);
	TmpObj->Character->Gold = unpack16(buf+44+14);
	TmpObj->Character->Armor = unpack16(buf+44+18);
	TmpObj->Character->Weight = unpack16(buf+44+20);
	if(MainWnd && serial == mserial)
	{
		sprintf(TitleText, "UoBot - %s (Mystic) Hp:%d/%d Mana:%d/%d Stam:%d/%d Peso:%d Ar:%d",
		TmpObj->Name, TmpObj->Character->HitPoints, TmpObj->Character->MaxHitPoints,
		TmpObj->Character->Mana, TmpObj->Character->MaxMana, TmpObj->Character->Stamina, TmpObj->Character->MaxStamina,
		TmpObj->Character->Weight, TmpObj->Character->Armor);
		SetWindowText(MainWnd, TitleText);
	}
	return 0;
}

int uo_handle_update_playerpos(char *buf, int len) // 0x77
{
	unsigned int serial=unpack32(buf+1);
	GameObject *TmpObj=FindSerial(serial);
	if(TmpObj==0) TmpObj=AddObj(serial);
	TmpObj = AddCharacter(serial);
	TmpObj->X = unpack16(buf+7);
	TmpObj->Y = unpack16(buf+9);
	TmpObj->Z = buf[11]&0xff;
	TmpObj->Direction = buf[12]&0x7;
	TmpObj->Color = unpack16(buf+13);
	TmpObj->Flags = buf[15];
	TmpObj->Character->HighLight = buf[16];
	return 0;
}

int uo_handle_pickup(char *buf, int len) // 0x07
{
	unsigned int serial=unpack32(buf+1);
	GameObject *TmpObj=FindSerial(serial);
	if(TmpObj==0) TmpObj = AddObj(serial);
	pickupobj.Serial = serial;
	pickupobj.Quantity = unpack32(buf+5);
	return 0;
}

// Nunca vira aqui(duh)
int uo_handle_dropitem(char *buf, int len) // 0x08
{
	unsigned int serial=unpack32(buf+1);
	pickupobj.Serial = 0;
	return 0;
}

int uo_handle_opencontainer(char *buf, int len) // 0x24
{
	unsigned int serial = unpack32(buf+1);
	GameObject *TmpObj=FindSerial(serial);
	if(TmpObj==0) TmpObj = AddObj(serial&0x7FFFFFFF);
	TmpObj->IsContainer = TRUE;
	return 0;
}

int uo_handle_addtocontainer(char *buf, int len) // 0x25
{
	unsigned int ItemSerial = unpack32(buf+1)&0x7FFFFFFF, ContainerSerial=unpack32(buf+14)&0x7FFFFFFF;
	unsigned int LastContSerial=0;

	GameObject *TmpObj=AddObj(ItemSerial), *TmpCont=AddObj(ContainerSerial);
	LastContSerial = TmpObj->Container;

	TmpCont->IsContainer = TRUE;
	TmpObj->Container = ContainerSerial;
	TmpObj->Graphic = unpack16(buf+5);
	if(CheckCont(TmpObj->Graphic))
	{
		TmpObj->IsContainer = TRUE;
		UseObject(TmpObj->Serial);
	}
	TmpObj->Quantity = unpack16(buf+8);
	TmpObj->X = unpack16(buf+10);
	TmpObj->Y = unpack16(buf+12);
	TmpObj->Color = unpack16(buf+18);
	if(IsCatchBagSet && ContainerSerial == mbackpack && LastContSerial != mbackpack)
	{
		if(ItemSerial != LastCaught)
		{
			LastCaught = ItemSerial;
			MoveToContainer(ItemSerial, TmpObj->Quantity, CatchBag);
		}
	}
	return 0;
}

int uo_handle_equipitem(char *buf, int len) // 0x2E
{
	unsigned int ItemSerial=unpack32(buf+1), OwnerSerial=unpack32(buf+9);
	GameObject *TmpObj=AddObj(ItemSerial), *OwnObj=AddObj(OwnerSerial);
	OwnObj = AddCharacter(OwnerSerial);
	OwnObj->IsContainer = TRUE;
	TmpObj->Container = OwnerSerial;
	TmpObj->Graphic = unpack16(buf+5);
	TmpObj->Layer = buf[8] & 0xff;
	TmpObj->Color = unpack16(buf+13);
	return 0;
}

int uo_handle_updatecontainer(char *buf, int len) // 0x3C
{
	unsigned int ItemSerial, ContainerSerial;
	int i=0, ItemCount=unpack16(buf+3);
	GameObject *TmpObj=0,*ContObj=0;
	if(ItemCount <= 0)
	{
		//LogPrint(3, "[ERROR] Update container packet is fucked up\n");
		return 0;
	}
	for(i=0; i < ItemCount; i++)
	{
		ItemSerial = unpack32(buf+5+(19*i))&0x7FFFFFFF;
		TmpObj = AddObj(ItemSerial&0x7FFFFFFF);
		ContainerSerial = unpack32(buf+5+(19*i)+13);
		ContObj = AddObj(ContainerSerial&0x7FFFFFFF);
		ContObj->IsContainer = TRUE;
		
		TmpObj->Container = ContainerSerial;
		TmpObj->Graphic = unpack16(buf+5+(19*i)+4);
		if(CheckCont(TmpObj->Graphic))
		{
			TmpObj->IsContainer = TRUE;
			UseObject(TmpObj->Serial);
		}
		TmpObj->Quantity = unpack16(buf+5+(19*i)+7);
		TmpObj->X = unpack16(buf+5+(19*i)+9);
		TmpObj->Y = unpack16(buf+5+(19*i)+11);
		TmpObj->Color = unpack16(buf+5+(19*i)+17);
	}
	return 0;
}

int uo_handle_paperdoll(char *buf, int len) // 0x88
{
	unsigned int Serial = unpack32(buf+1);
	GameObject *TmpObj=AddCharacter(Serial);
	TmpObj->Flags = buf[65];
	return 0;
}

int uo_handle_trading(char *buf, int len) // 0x6F
{
	unsigned int Serial = unpack32(buf+4);
	unsigned char flag = buf[3]&0xff;
	unsigned int Id = unpack32(buf+8);
	unsigned char buff[30];
	if(AceitarCoisas && flag==2 && Id==0) // Aceita todos os itens q for posto no trade
	{
		memset(buff, 0x00, 17);
		buff[0] = 0x6F;
		buff[1] = 0x00;
		buff[2] = 0x11;
		buff[3] = 0x02;
		memcpy(buff+4, buf+4, 4);
		pack32(buff+8, 0x01);
		send_server(buff, 17);
	}
	return 0;
}

int uo_handle_unispeech(char *buf, int len) // 0xAE
{
	unsigned int serial = unpack32(buf+3);
	unsigned char *Texto,*buff=buf;
	GameObject *TmpObj;
	//if(buf[18]==0 || buf[48] == 0 || len-48 == 0 || len-48 == 1)
	//	return 0;
	//LogPrint(1, "[TESTE] Len: %d\n", len-48);
	if((Texto=malloc(len-48))==NULL){
		LogPrint(3, "[ERROR] Nao foi possivel alocar espaco pro unispeech\n");
		return 0;
	}
	UnicodeToAscii(buf+48, len-48, Texto);
	if(Texto[0] == 0)
		return 0;
	
	if(buf[9] == 0)
		LogPrint(3, "[SPEECH] [%s] [%s]\n", buf+18, Texto);
	else if(buf[9] == 1)
		LogPrint(3, "[SYSMSG] [%s] [%s]\n", buf+18, Texto);
	else if(buf[9] == 6)
		LogPrint(1, "[YOUSEE] [%s]\n", Texto);
	else if(buf[9] == 10)
		LogPrint(1, "[CAST] [%s] [%s]\n", buf+18, Texto);
	else
		LogPrint(3, "[SPEECH] [%d] [%s] [%s]\n", buf[9], buf+18, Texto);

	if(!strcmp(Texto, "debug on")) PrintDebug = TRUE;
	if(!strcmp(Texto, "debug off")) PrintDebug = FALSE;
	if(!strcmp(Texto, "fecha tudo")) CloseShardSocket();
	if((buff[9] & 0x0F) == JOURNAL_YOUSEE)
	{
		JournalAdd("You see", Texto, buff[9] & 0x0F);
		if((TmpObj=FindSerial(serial))!=0)
		{
			strncpy(TmpObj->Name, Texto, 30);
			TmpObj->Name[30] = 0;
		}
	}
	else JournalAdd(buf+18, Texto, buff[9] & 0x0F);

	TmpObj = FindSerial(serial);

	if(DetectSpeech && TmpObj != 0 && TmpObj->Serial != mserial)
	{
		if(LastStaffCheck && time(0) - LastStaffCheck < 50)
		{
			LastStaffCheck = time(0);
			free(Texto);
			return 0;
		}
		LastStaffCheck = time(0);

		LogPrint(3, "[ALERTA] Algum staff falou algo perto de vc. Se ligue ingame\n");

		if(AntiStaff)
		{
			if(hThread)
				SuspendThread(hThread);
		
			SuspendAllPThreads();
			ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
			
			MBOut("Alerta", "Algum staff falou algo perto de vc\nSe ligue ingame\n");
			if(hThread)
				ResumeThread(hThread);
			ResumeAllPThreads();
		}
		else
			ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
	}

	if(buf[9] == 0 || buf[9] == 6)
	{
		if((TmpObj = FindSerial(serial)))
		{
			if(strstr(TmpObj->Name, "Staff"))
			{
				if(LastStaffCheck && time(0) - LastStaffCheck < 50)
				{
					LastStaffCheck = time(0);
					free(Texto);
					return 0;
				}
				LastStaffCheck = time(0);

				LogPrint(3, "[ALERTA] Algum staff falou algo perto de vc. Se ligue ingame\n");
				if(AntiStaff)
				{
					if(hThread)
						SuspendThread(hThread);
					SuspendAllPThreads();
					ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
				
					MBOut("Alerta", "Algum staff falou algo perto de vc\nSe ligue ingame\n");
					if(hThread)
						ResumeThread(hThread);
					ResumeAllPThreads();
				}
				else
					ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
			}
		}
				
		if(strstr(buf+18, "Staff"))
		{
			if(LastStaffCheck && time(0) - LastStaffCheck < 50)
			{
				LastStaffCheck = time(0);
				free(Texto);
				return 0;
			}
			LastStaffCheck = time(0);

			LogPrint(3, "[ALERTA] Algum staff falou algo perto de vc. Se ligue ingame\n");
			if(AntiStaff)
			{
				if(hThread)
					SuspendThread(hThread);
				SuspendAllPThreads();
				ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
				
				MBOut("Alerta", "Algum staff falou algo perto de vc\nSe ligue ingame\n");
				if(hThread)
					ResumeThread(hThread);
				ResumeAllPThreads();
			}
			else
				ShellExecute(MainWnd, "open", AvisoFile, "", ".", SW_SHOWNORMAL);
		}
	}
	
	free(Texto);
	return 0;
}

int uo_handle_target(char *buf, int len) // 0x6C
{
	TargetSent = 1;
    RecvTargetSequence = unpack32(buf + 2);

    if(RecvTarget)
        return 0;

	if(WaitTargetRequest)
	{
		//LogPrint((PrintDebug?3:0), "[TARGET] Replying server target\n");
		if(WaitTargetType == TARGET_OBJECT)
			TargetReplyObj(WaitTargetSerial, WaitTargetGraphic, unpack32(buf + 2));
		/*else if(WaitTargetType == TARGET_TILE)
			TargetReplyTile(WaitTargetGraphic, WaitTargetX, WaitTargetY, WaitTargetZ, UnpackUInt32(Packet + 2));*/
		
		WaitTargetRequest = FALSE;
		return 0;
	}
	CancelTargetRequest(unpack32(buf+2));
	return 0;
}

int uo_handle_updateskill(char *buf, int len) // 0x3A
{
	unsigned int TimeNow = time(0),count=0;
	unsigned char flag=buf[3],*p=buf+4;
	if(flag == 0)
	{
		while(*((unsigned short *)p)!=0x0000)
		{
			SkillsName[p[1]-1].LastValue = unpack16(p+2);
			p += 7;
			count++;
		}
		LogPrint(3, "[SKILLS] Updated %d skills\n", count);
	}
	else if(flag == 0xFF)
	{
		FILE *logSkills;
		time_t tempo;
		struct tm *timeinfo;

		time ( &tempo );
		timeinfo = localtime ( &tempo );

		logSkills = fopen("logskills.txt", "a");

		if(SkillsTimeList[buf[5]] != 0)
		{
			LogPrint(3, "[SKILLUP] Skill (%s) mudou de (%.1f) para (%.1f) em (%d) segundos\n",
				SkillsName[buf[5]].Name, (double)SkillsName[buf[5]].LastValue/10, (double)unpack16(buf+6)/10, TimeNow - SkillsTimeList[buf[5]]);
			if(logSkills)
				fprintf(logSkills, "[%02d:%02d] [%02d:%02d:%02d] [SKILLUP] Skill (%s) mudou de (%.1f) para (%.1f) em (%d) segundos\n",
					timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, SkillsName[buf[5]].Name, (double)SkillsName[buf[5]].LastValue/10, (double)unpack16(buf+6)/10, TimeNow - SkillsTimeList[buf[5]]);

		}
		else
		{
			LogPrint(3, "[SKILLUP] Skill (%s) mudou de (%.1f) para (%.1f)\n",
				SkillsName[buf[5]].Name, (double)SkillsName[buf[5]].LastValue/10, (double)unpack16(buf+6)/10);
			if(logSkills)
				fprintf(logSkills, "[%02d:%02d] [%02d:%02d:%02d] [SKILLUP] Skill (%s) mudou de (%.1f) para (%.1f)\n",
					timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, SkillsName[buf[5]].Name, (double)SkillsName[buf[5]].LastValue/10, (double)unpack16(buf+6)/10);
		}

		if(logSkills)
			fclose(logSkills);

		SkillsTimeList[buf[5]] = TimeNow;
		SkillsName[buf[5]].LastValue = unpack16(buf+6);
	}
	return 0;
}

int uo_handle_login_denied(char *buf, int len) //0x82
{
	if(buf[1] == 0)
		LogPrint(3, "[LOGINERRO] Unknown user\n");
	else if(buf[1] == 1)
		LogPrint(3, "[LOGINERRO] Account already in use\n");
	else if(buf[1] == 2)
		LogPrint(3, "[LOGINERRO] Account disabled\n");
	else if(buf[1] == 3)
		LogPrint(3, "[LOGINERRO] Password errado\n");
	else if(buf[1] >= 4)
		LogPrint(3, "[LOGINERRO] Communications failed\n");

	return 0;
}

int uo_handle_play_effect(char *buf, int len) // 0x70
{
	unsigned int FromSerial=unpack32(buf+2),ToSerial=unpack32(buf+6);
	unsigned short Type=unpack16(buf+10);
	if(WaitGraphicEffect)
	{
		if((WaitGraphicFromSerial==INVALID_SERIAL || WaitGraphicFromSerial==FromSerial) && (WaitGraphicToSerial==INVALID_SERIAL || WaitGraphicToSerial==ToSerial) && (WaitGraphicType==Type || WaitGraphicType==0xFFFF))
		{
			GraphicEffectSent=1;
			WaitGraphicEffect = FALSE;
			return 0;
		}
	}
	return 0;
}

/* -------------------- HANDLES END -------------------- */

DWORD WINAPI uo_thread_ping(LPVOID lpParameter)
{
	unsigned char buf[5];
	unsigned char lastPingSeq=0;
	send_shard threadsend=(send_shard)lpParameter;
gthread1:
	if(ReconnectTime)
	{
		if((time(0) - LoginTime) >= ReconnectTime)
		{
			LogPrint(3, "[RECONNECT] Timeout reconnect time [%d], logintime [%d] minutos\n", ReconnectTime/60, (time(0) - LoginTime)/60);
			LogPrint(3, "[RECONNECT] Desconectando\n");
			ErrNum = 2;
			CloseShardSocket();
		}
	}
	buf[0] = 0x73;
	buf[1] = lastPingSeq;
	threadsend(buf, 2);
	lastPingSeq++;
	Sleep(60000);
	goto gthread1;
}

void uo_processpkt(unsigned char *buf, int len)
{
	LogPrint((PrintDebug ? 3 : 0), "[S->C] [%#x] %s [%d]\n", buf[0]&0xff, PacketNames[buf[0]&0xff], len);
	if(PrintDebug) LogDump(buf, len);
	if(buf[0] <= 205)
	{
		if(!hnmsg[buf[0]&0xff].uo_hand)
		{
			//LogPrint((PrintDebug ? 3 : 0), "[PROCESSPKT] hnmsg[%#x].uo_hand not set\n", *buf&0xff);
			return;
		}
		hnmsg[*buf&0xff].uo_hand(buf, len);
		if(ExternalHandlers[buf[0]&0xff])
		{
			ExternalHandlers[buf[0]&0xff](buf, len);
		}
	}
	else LogPrint(3, "[ERROR] Sizeof hnmsg(%d) < %d\n", 0xcd, buf[0]&0xff);
}

void uo_processclientmsg(char *buf, int len)
{
	unsigned char *buff=buf;
	LogPrint((DebugClientMsg|| PrintDebug ? 2 : 0) , "[C->S] [%#x] %s [%u]\n", *buf&0xff, PacketNames[*buf&0xff], len);
	if(DebugClientMsg || PrintDebug) LogDump(buf, len);
	if(buf[0] == 0x2)
	{
		uo_handle_walk_req(buf, len); 
	}
}