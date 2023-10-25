modded class PluginTransmissionAgents extends PluginBase
{
	override protected void InjectAgentsWithPlayer(EntityAI target, int agents, float protection, int dose_size, int inject_type)
	{
		if (inject_type == InjectTypes.PLAYER_AIR_PLAYER)
		{
			if (target.IsPlayer())
			{
				PlayerBase player = PlayerBase.Cast(target);
				if (player && player.IsInSafeZone())
				{
					return;
				}
			}
		}
		
		super.InjectAgentsWithPlayer(target, agents, protection, dose_size, inject_type);
	}	
	
	override protected void InjectAgentsWithPlayerCount(EntityAI target, int agents, float protection, int dose_size, int inject_type)
	{
		if (inject_type == InjectTypes.PLAYER_AIR_PLAYER)
		{
			if (target.IsPlayer())
			{
				PlayerBase player = PlayerBase.Cast(target);
				if (player && player.IsInSafeZone())
				{
					return;
				}
			}
		}

		super.InjectAgentsWithPlayerCount(target, agents, protection, dose_size, inject_type);
	}
}