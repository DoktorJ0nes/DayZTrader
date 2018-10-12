//modded class LandMineTrap
modded class TrapBase
{
    void StartActivate( PlayerBase player )
	{
        //--------------------------------------------------------------- TRADER BEGIN ------------------------------------------------------------------------
        if (player)
        {
            if (player.m_Trader_IsInSafezone)
               return;
        }
        //---------------------------------------------------------------- TRADER END -------------------------------------------------------------------------

        m_Timer = new Timer( CALL_CATEGORY_GAMEPLAY );
        HideSelection("safety_pin");
        
        if ( m_InitWaitTime > 0 )
        {
            m_IsInProgress = true;
            m_Timer.Run( m_InitWaitTime, this, "SetActive" );
            
            /*
            if (player)
            {
                player.MessageStatus( m_InfoActivationTime );
            }
            */
        
            Synch(NULL);
        }
        else
        {
            SetActive();
        }
	}
    
    /*override void StartActivate( PlayerBase player )
	{
        if (player.m_Trader_IsInSafezone)
            return;

		super.StartActivate( player );
		
		if (GetGame().IsClient())
		{
			PlaySound("landmine_safetyPin", 3);
			m_TimerLoop = PlaySoundLoop("landmine_timer2", 3);
		}
	}*/
}