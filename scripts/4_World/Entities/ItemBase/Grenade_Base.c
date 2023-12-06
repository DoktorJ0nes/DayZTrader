modded class Grenade_Base extends InventoryItemSuper
{
    override protected void ExplodeGrenade(EGrenadeType grenade_type)
	{
        if (IsInSafezone())
        {
            OnExplode(); // deletes Grenade from Server
            return;
        }
		
        super.ExplodeGrenade(grenade_type);
	}

    bool IsInSafezone()
    {
        if(GetGame().IsServer())
        {
            PluginTraderServerConfig config = PluginTraderServerConfig.Cast(GetPlugin(PluginTraderServerConfig));
            if(config)
            {
                return config.IsPositionInSafeZone(GetPosition());
            }
        }
        return false;
    }
};