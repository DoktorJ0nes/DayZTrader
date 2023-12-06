modded class TrapBase
{
    override void StartActivate( PlayerBase player )
	{
        if (player && player.IsInSafeZone())
        {
            return;
        }

        super.StartActivate(player);
	}
};