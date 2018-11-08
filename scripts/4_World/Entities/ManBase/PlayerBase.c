modded class PlayerBase
{
	bool m_Trader_IsTrader = false;

    override void EEKilled( Object killer )
	{
		//--------------------------------------------------------------- TRADER BEGIN ------------------------------------------------------------------------
		
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
        //---------------------------------------------------------------- TRADER END -------------------------------------------------------------------------

		Print("EEKilled, you have died");
		
		DayZPlayerSyncJunctures.SendDeath(this, -1, 0);
		
		if( GetBleedingManagerServer() ) delete GetBleedingManagerServer();
		
		if( GetHumanInventory().GetEntityInHands() )
		{
			if( CanDropEntity(this) )
			{
				if( !IsRestrained() )
				{
					GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ServerDropEntity,1000,false,( GetHumanInventory().GetEntityInHands()));
				}
			}
			
		}
		
		// kill character in database
		if (GetHive())
		{
			GetHive().CharacterKill(this);
		}
	
		// disable voice communication
		GetGame().EnableVoN(this, false);
	
		
		if ( GetSoftSkillManager() )
		{
			delete GetSoftSkillManager();
		} 
		
		GetSymptomManager().OnPlayerKilled();
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos)
	{
		//--------------------------------------------------------------- TRADER BEGIN ------------------------------------------------------------------------
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
		//---------------------------------------------------------------- TRADER END -------------------------------------------------------------------------

		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos);
		if( damageResult != null && damageResult.GetDamage(dmgZone, "Shock") > 0)
		{
			m_LastShockHitTime = GetGame().GetTime();
		}
		//DamagePlayer(damageResult, source, modelPos, ammo);
		
		//new bleeding computation
		//---------------------------------------
		if ( damageResult != null && GetBleedingManagerServer() )
		{
			float dmg = damageResult.GetDamage(dmgZone, "Blood");
			GetBleedingManagerServer().ProcessHit(dmg, component, dmgZone, ammo, modelPos);
		}
		//Print(damageResult.GetDamage(dmgZone,"Health"));
		//---------------------------------------
		
		//if( GetBleedingManagerServer() ) GetBleedingManagerServer().ProcessHit(dmgZone, ammo, modelPos);
		#ifdef DEVELOPER
		if(DiagMenu.GetBool(DiagMenuIDs.DM_MELEE_DEBUG_ENABLE))
		{
			Print("EEHitBy() | Charater " + GetDisplayName() + " hit by " + source.GetDisplayName() + " to " + dmgZone);
		}
		
		PluginRemotePlayerDebugServer plugin_remote_server = PluginRemotePlayerDebugServer.Cast( GetPlugin(PluginRemotePlayerDebugServer) );
		if(plugin_remote_server)
		{
			plugin_remote_server.OnDamageEvent(this, damageResult);
		}
		#endif
		if (GetGame().IsDebugMonitor())
			m_DebugMonitorValues.SetLastDamage(source.GetDisplayName());
	}
}