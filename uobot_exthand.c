#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uobot_log.h"
#include "uobot_net.h"
#include "uobot_handles.h"
#include "uobot_obj.h"
#include <time.h>
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot_exthand.h"
#include "uobot_threads.h"
#include "uobot.h"

PacketHandler ExternalHandlers[256];

void InitPacketHandler(void)
{
	memset(ExternalHandlers, 0, sizeof(ExternalHandlers));
}

int AddPacketHandler(int PacketID, void *Handler)
{
	if(ExternalHandlers[PacketID] != 0)
	{
		LogPrint(3, "[PACKETHANDLER] FULL: Handler: %X Packet: %X\n", Handler, PacketID);
		return 0;
	}

	ExternalHandlers[PacketID] = (PacketHandler)Handler;
	LogPrint(3, "[PACKETHANDLER] ADDED: Handler: %X Packet: %X\n", Handler, PacketID);
	return 1;
}