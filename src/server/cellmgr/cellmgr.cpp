
#include "cellmgr.h"
#include "cellmgr_interface.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "network/bundle_broadcast.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include <sstream>

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(CellMgrApp);

//-------------------------------------------------------------------------------------
CellMgrApp::CellMgrApp(Network::EventDispatcher& dispatcher, 
				 Network::NetworkInterface& ninterface, 
				 COMPONENT_TYPE componentType,
				 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
//logWatchers_(),
//buffered_logs_(),
timer_()
{
}

//-------------------------------------------------------------------------------------
CellMgrApp::~CellMgrApp()
{
}

//-------------------------------------------------------------------------------------		
bool CellMgrApp::initializeWatcher()
{
	//ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());

	//WATCH_OBJECT("stats/totalNumlogs", &totalNumlogs);
	//WATCH_OBJECT("stats/secsNumlogs", &secsNumlogs);
	//WATCH_OBJECT("stats/bufferedLogsSize", this, &MyCellMgrApp::bufferedLogsSize);
	return true;
}

//-------------------------------------------------------------------------------------
bool CellMgrApp::run()
{
	dispatcher_.processUntilBreak();
	return true;
}

//-------------------------------------------------------------------------------------
void CellMgrApp::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_TICK:
			this->handleTick();
			break;
		default:
			break;
	}

	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void CellMgrApp::handleTick()
{

	threadPool_.onMainThreadTick();
	networkInterface().processChannels(&CellappmgrInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool CellMgrApp::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool CellMgrApp::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool CellMgrApp::initializeEnd()
{
	// 由于logger接收其他app的log，如果跟踪包输出将会非常卡。
	//Network::g_trace_packet = 0;

	timer_ = this->dispatcher().addTimer(1000000 / 50, this,
							reinterpret_cast<void *>(TIMEOUT_TICK));
	return true;
}

//-------------------------------------------------------------------------------------
void CellMgrApp::finalise()
{

	timer_.cancel();
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------	
bool CellMgrApp::canShutdown()
{
	if(Components::getSingleton().getGameSrvComponentsSize() > 0)
	{
		INFO_MSG(fmt::format("CellMgrApp::canShutdown(): Waiting for components({}) destruction!\n", 
			Components::getSingleton().getGameSrvComponentsSize()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------

}
