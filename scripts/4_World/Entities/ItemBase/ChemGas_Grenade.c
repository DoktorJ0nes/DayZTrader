modded class Grenade_ChemGas
{
	override protected void OnExplode()
	{
        if (isInSafezone())
        {
		    m_Exploded = true;
            if (GetGame().IsServer())
            {
                DeleteSafe();
            }
            return;
        }

        super.OnExplode();
	}
};

modded class Ammo_40mm_Explosive: Ammo_40mm_Base
{
	override void OnActivatedByItem(notnull ItemBase item)
	{
        if(g_Game.InSafezoneRadius(GetPosition()))
        {
            return;
        }
		super.OnActivatedByItem(item);
	}
	
	override void EEKilled(Object killer)
	{
        if(g_Game.InSafezoneRadius(GetPosition()))
        {
		    GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( DeleteSafe, 1000, false);
            return;
        }
		super.EEKilled(killer);
	}
};

modded class Ammo_40mm_ChemGas: Ammo_40mm_Base
{
	override void OnActivatedByItem(notnull ItemBase item)
	{
        if(g_Game.InSafezoneRadius(GetPosition()))
        {
            return;
        }
		super.OnActivatedByItem(item);
	}
	
	override void EEKilled(Object killer)
	{
        if(g_Game.InSafezoneRadius(GetPosition()))
        {
		    GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( DeleteSafe, 1000, false);
            return;
        }
		super.EEKilled(killer);
	}
};