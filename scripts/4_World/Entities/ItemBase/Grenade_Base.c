modded class Grenade_Base extends InventoryItemSuper
{
    override protected void ExplodeGrenade(EGrenadeType grenade_type)
	{
        if (isInSafezone())
        {
            OnExplode(); // deletes Grenade from Server
            return;
        }
		
        super.ExplodeGrenade(grenade_type);
	}

    bool isInSafezone()
    {
        PlayerBase player = getRandomValidPlayer();

        if (!player)
            return false;

        for (int k = 0; k < player.m_Trader_TraderPositions.Count(); k++)
        {
            if (vector.Distance(this.GetPosition(), player.m_Trader_TraderPositions.Get(k)) <= player.m_Trader_TraderSafezones.Get(k))
                return true;
        }

        return false;
    }

    PlayerBase getRandomValidPlayer()
    {
        if( GetGame().IsServer() )
        {
            ref array<Man> m_Players = new array<Man>;
		    GetGame().GetWorld().GetPlayerList(m_Players);

            for (int j = 0; j < m_Players.Count(); j++)
			{
				PlayerBase player = PlayerBase.Cast(m_Players.Get(j));
				
				if (!player)
					continue;

                if (!player.IsAlive())
					continue;

				if (!player.m_Trader_RecievedAllData)
					continue;

                return player;
            }
        }
        else
        {
            return GetGame().GetPlayer();
        }

        return null;
    }
}

/*modded class ActionUnpin extends ActionSingleUseBase
{

}*/