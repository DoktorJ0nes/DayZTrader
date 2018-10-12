modded class PlayerBase
{
    override void EEKilled( Object killer )
	{
        //--------------------------------------------------------------- TRADER BEGIN ------------------------------------------------------------------------
        if (this.GetIdentity() != NULL)
        {
            m_Trader_PlayerDiedInSafezone = m_Trader_IsInSafezone;
            GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_TRADER_PLAYER_DIED_IN_SAFEZONE, new Param1<bool>( m_Trader_PlayerDiedInSafezone ), true, this.GetIdentity());

            if (m_Trader_PlayerDiedInSafezone)
            {
                //GetHumanInventory().LockInventory(LOCK_FROM_SCRIPT);

                if (GetHive())
                    GetHive().CharacterExit(this);

                //GetGame().ObjectDelete(player);
                //GetGame().ObjectDelete(this);
                SetPosition(vector.Zero);

                return;
            }
        }
        //---------------------------------------------------------------- TRADER END -------------------------------------------------------------------------

		Print("EEKilled, you have died");
		if( GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_CLIENT )
		{
			// @NOTE: this branch does not happen, EEKilled is called only on server
			if( GetGame().GetPlayer() == this )
			{
				super.EEKilled( killer );
			}
			if (GetHumanInventory().GetEntityInHands())
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ServerDropEntity,1000,false,( GetHumanInventory().GetEntityInHands() ));
		}
		else if( GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_SERVER)//server
		{
			if( GetBleedingManager() ) delete GetBleedingManager();
			if( GetHumanInventory().GetEntityInHands() )
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ServerDropEntity,1000,false,( GetHumanInventory().GetEntityInHands() ));
		}
		
		if ( GetSoftSkillManager() )
		{
			delete GetSoftSkillManager();
		} 
		
		GetStateManager().OnPlayerKilled();
		
		// kill character in database
		if (GetHive())
		{
			GetHive().CharacterKill(this);
            GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "CHAR KILLED FROM HIVE!" ), true, this.GetIdentity());
		}
	}

	/*override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, string component, string ammo, vector modelPos)
	{
        PlayerBase player = GetGame().GetPlayer();
        if (player.m_Trader_IsInSafezone)
               return;

		super.EEHitBy(damageResult, damageType, source, component, ammo, modelPos);
		if( damageResult != null && damageResult.GetDamage(component, "Shock") > 0)
		{
			m_LastShockHitTime = GetGame().GetTime();
		}
		//DamagePlayer(damageResult, source, modelPos, ammo);
		
		//new bleeding computation
		//---------------------------------------
		if ( damageResult != null && GetBleedingManager() )
		{
			float dmg = damageResult.GetDamage(component, "Blood");
			GetBleedingManager().ProcessHit(dmg, component, ammo, modelPos);
		}
		//Print(damageResult.GetDamage(component,"Health"));
		//---------------------------------------
		
		//if( GetBleedingManager() ) GetBleedingManager().ProcessHit(component, ammo, modelPos);
		#ifdef DEVELOPER
		if(DiagMenu.GetBool(DiagMenuIDs.DM_MELEE_DEBUG_ENABLE))
		{
			Print("EEHitBy() | Charater " + GetDisplayName() + " hit by " + source.GetDisplayName() + " to " + component);
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
	
	override void EEHitByRemote(int damageType, EntityAI source, string component, string ammo, vector modelPos)
	{
        PlayerBase player = GetGame().GetPlayer();
        if (player.m_Trader_IsInSafezone)
               return;

		super.EEHitByRemote(damageType, source, component, ammo, modelPos);
		
		if( GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_CLIENT )
		{
			SpawnBulletHitReaction();
		}	
		
		Print("DayZPlayerImplement : EEHitByRemote");
	}*/
}