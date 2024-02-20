modded class Grenade_Base extends InventoryItemSuper
{    
	override protected void InitiateExplosion()
	{
        if (isInSafezone())
        {
            OnExplode(); // deletes Grenade from Server
            return;
        }

        super.InitiateExplosion();
	}

    bool isInSafezone()
    {
        return g_Game.InSafezoneRadius(GetPosition());
    }
};