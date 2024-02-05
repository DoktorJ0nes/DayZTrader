modded class Grenade_Base extends InventoryItemSuper
{
    bool m_Trader_IsInSafezone = false;

    override protected void ExplodeGrenade(EGrenadeType grenade_type)
	{
        if (IsInSafeZone())
        {
            OnExplode(); // deletes Grenade from Server
            return;
        }
		
        super.ExplodeGrenade(grenade_type);
	}

    void SetInSafezone(bool IsInSafeZone)
    {
        m_Trader_IsInSafezone = IsInSafeZone;
        if(IsInSafeZone)
        {
            TraderMessage.ServerLog(GetDisplayName() + " entered the safezone.");
        }
        else
        {
            TraderMessage.ServerLog(GetDisplayName() + " left the safezone.");
        }
    }

    bool IsInSafeZone()
    {
        return m_Trader_IsInSafezone;
    }
}