/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

***********************************************************************/

#include "../Generated/GuiRemoteProtocolSchema.h"

namespace vl::presentation::remoteprotocol
{
	/*
	* dom id:
	* 
	* root:              -1
	* element:           (elementId << 2) + 0
	* parent of element: (elementId << 2) + 1
	* hittest:           (compositionId << 2) + 2
	* parent of hittest: (compositionId << 2) + 3
	*/
	extern Ptr<RenderingDom>		BuildDomFromRenderingCommands(Ptr<collections::List<RenderingCommand>> commandListRef);
}