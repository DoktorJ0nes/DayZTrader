modded class AreaExposureMdfr: ModifierBase
{
	override void OnTick(PlayerBase player, float deltaT)
	{
		#ifdef DEVELOPER
		if(!player.GetCanBeDestroyed())
			return;
		#endif
		if(!player.GetAllowDamage())
		{
			return;
		}
		super.OnTick(player, deltaT);		
	}	
};