modded class PlayerBase
{
	bool m_Trader_IsTrader = false;

    override void EEKilled( Object killer )
	{		
		m_Trader_PlayerDiedInSafezone = m_Trader_IsInSafezone;
		GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_TRADER_PLAYER_DIED_IN_SAFEZONE, new Param1<bool>( m_Trader_PlayerDiedInSafezone ), true, this.GetIdentity());
		
		if (killer != NULL && (m_Trader_PlayerDiedInSafezone || m_Trader_IsTrader))
		{
			PlayerBase playerKiller;
			Class.CastTo(playerKiller, killer);

			if (playerKiller && playerKiller != this)
			{
				Print("[TRADER] Player with PlayerUID " + playerKiller.GetIdentity().GetId() + " killed someone in the Safezone!");
				playerKiller.SetPosition(this.GetPosition());
				playerKiller.SetHealth( "", "", 0 );
				playerKiller.SetHealth( "", "Blood", 0 );
			}
		}

		if (m_Trader_PlayerDiedInSafezone)
		{
			
			this.SetHealth( "", "", this.m_Trader_HealthEnteredSafeZone );
			this.SetHealth( "","Blood", this.m_Trader_HealthBloodEnteredSafeZone );
			this.SetHealth( "","Shock", this.GetMaxHealth( "", "Shock" ) );

			if (GetHive())
				GetHive().CharacterExit(this);
				
			if (!this.IsAlive())
				SetPosition(vector.Zero);

			return;
		}
		
		if (m_Trader_IsTrader) // TODO!
			return;


		super.EEKilled(killer);
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos)
	{
		if ( m_Trader_IsInSafezone )
		{
			DayZInfected sourceInfected;
			Class.CastTo(sourceInfected, source);

			DayZAnimal sourceAnimal;
			Class.CastTo(sourceAnimal, source);

			if (sourceInfected || sourceAnimal)
			{
				//GetGame().ObjectDelete(source);
				source.SetHealth( "", "", 0 );
				source.SetHealth( "", "Blood", 0 );
			}

			return;
		}


		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos);
	}
}