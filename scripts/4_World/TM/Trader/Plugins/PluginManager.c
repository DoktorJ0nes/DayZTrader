modded class PluginManager
{
	override void Init()
	{
		super.Init();
		if (GetGame().IsServer())
		{
			RegisterPlugin( "PluginTraderWriteLog", false, true);
			RegisterPlugin( "PluginTraderServerConfig", false, true);
		}		
		RegisterPlugin( "PluginTraderData", true, true);
	}
};