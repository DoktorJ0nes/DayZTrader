modded class PlayerBase
{
	bool m_Trader_IsTrader = false;

    override void EEKilled( Object killer )
	{
		//--------------------------------------------------------------- TRADER BEGIN ------------------------------------------------------------------------
		if (m_Trader_IsTrader)
			return;

        if (this.GetIdentity() != NULL)
        {
            m_Trader_PlayerDiedInSafezone = m_Trader_IsInSafezone;
            GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_TRADER_PLAYER_DIED_IN_SAFEZONE, new Param1<bool>( m_Trader_PlayerDiedInSafezone ), true, this.GetIdentity());

            if (m_Trader_PlayerDiedInSafezone)
            {
				/*// KEEP ALIVE METHOD START
				//GetHive().CharacterExit(this); // AUSPROBIEREN!

				string characterName = GetGame().CreateRandomPlayer();
				vector pos = this.GetPosition();

				PlayerBase m_player;
				Entity playerEnt;
				playerEnt = GetGame().CreatePlayer(this.GetIdentity(), characterName, pos, 0, "NONE");
				Class.CastTo(m_player, playerEnt);
				//HumanInventory i = HumanInventory.Cast(GetInventory());
				
				GetGame().SelectPlayer(this.GetIdentity(), m_player);
				//GetGame().SelectPlayer(this.GetIdentity(), this);
				
				if( player ) player.OnConnect();		
				// Send list of players at all clients
				SyncEvents.SendPlayerList();

				//GetHive().CharacterKill(this);
				GetHive().CharacterSave(m_player);
				//GetHive().CharacterExit(m_player);
				//GetGame().DisconnectPlayer(m_player.GetIdentity(), m_player.GetIdentity().GetId());
				//m_player.Delete();
				////this.Delete();
				
				//GetGame().RPCSingleParam(m_player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "CHAR IS VALID!" ), true, m_player.GetIdentity());
				// KEEP ALIVE METHOD END*/
				
				this.SetHealth( "", "", this.m_Trader_HealthEnteredSafeZone );
				this.SetHealth( "","Blood", this.m_Trader_HealthBloodEnteredSafeZone );
				this.SetHealth( "","Shock", this.GetMaxHealth( "", "Shock" ) );

				if (GetHive())
					GetHive().CharacterExit(this);
					
				if (!this.IsAlive())
					SetPosition(vector.Zero);
				
				//GetGame().SelectPlayer(this.GetIdentity(), this);
				//GetHive().CharacterSave(this);

				// RELOG METHOD START
                //GetHumanInventory().LockInventory(LOCK_FROM_SCRIPT);

                //if (GetHive())
                //    GetHive().CharacterExit(this);

                ////GetGame().ObjectDelete(player);
				////GetGame().ObjectDelete(this);
				////this.Delete();
				//SetPosition(vector.Zero);

				//this.OnConnect();
				// RELOG METHOD END

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
		}
	}
}