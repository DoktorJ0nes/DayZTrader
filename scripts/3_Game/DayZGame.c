modded class DayZGame
{        
	ref array<vector> m_Trader_TraderPositions;
	ref array<int> m_Trader_TraderSafezones;

    void SetTraderPositionsAndSafezones(array<vector> traderPositions, array<int> traderSafezones)
    {
        m_Trader_TraderPositions = traderPositions;
        m_Trader_TraderSafezones = traderSafezones;
    }


    bool InSafezoneRadius(vector position)
    {
        if(!m_Trader_TraderPositions)
        {
            return false;
        }
        for (int k = 0; k < m_Trader_TraderPositions.Count(); k++)
        {
            vector safezonePos = m_Trader_TraderPositions.Get(k);
            float distanceToSafezone = vector.Distance(position, safezonePos);
            float distanceToSafezoneMax = m_Trader_TraderSafezones.Get(k);
			
			if(distanceToSafezoneMax  < 1.0)
			{
				continue;
			}

            if (distanceToSafezone <= distanceToSafezoneMax)
                return true;
        }
		return false;
    }

    override void FirearmEffects(Object source, Object directHit, int componentIndex, string surface, vector pos, vector surfNormal,
		 vector exitPos, vector inSpeed, vector outSpeed, bool isWater, bool deflected, string ammoType) 
	{		
        if(IsServer() && InSafezoneRadius(pos))
        {
            return;
        }
        super.FirearmEffects(source, directHit, componentIndex, surface, pos, surfNormal, exitPos, inSpeed, outSpeed, isWater, deflected, ammoType);
	}
};