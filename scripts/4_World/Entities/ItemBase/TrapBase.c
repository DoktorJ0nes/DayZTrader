modded class TrapBase
{
    override void StartActivate( PlayerBase player )
	{
        if (player)
        {
            if (player.IsInSafeZone())
               return;
        }


        super.StartActivate(player);
	}
}