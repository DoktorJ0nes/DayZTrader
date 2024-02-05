#ifdef SERVER
modded class PluginManager
{
	override void Init()
	{
		super.Init();
		if (GetGame().IsServer())
		{
#ifndef TRADER_HIDE_SERVER_LOGS
			RegisterPlugin( "PluginTraderServerLog", false, true);
			RegisterPlugin( "PluginTraderTradesLog", false, true);
#endif
		}		
	}
};
#endif