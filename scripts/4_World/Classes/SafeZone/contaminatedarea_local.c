modded class ContaminatedArea_Local : ContaminatedArea_Dynamic
{
	override void DeferredInit()
	{
		if (GetGame().IsServer() || !GetGame().IsMultiplayer())
		{
			if(g_Game.InSafezoneRadius(GetPosition()))
			{
				Delete();
				return;
			}
		}

		super.DeferredInit();
	}	
};