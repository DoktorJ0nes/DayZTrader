//modded class LandMineTrap
modded class TrapBase
{
    override void StartActivate( PlayerBase player )
	{
        //--------------------------------------------------------------- TRADER BEGIN ------------------------------------------------------------------------
        if (player)
        {
            if (player.m_Trader_IsInSafezone)
               return;
        }
        //---------------------------------------------------------------- TRADER END -------------------------------------------------------------------------

        super.StartActivate(player);
        
        /*
        m_Timer = new Timer( CALL_CATEGORY_GAMEPLAY );
        HideSelection("safety_pin");
        
        if ( m_InitWaitTime > 0 )
        {
            m_IsInProgress = true;
            m_Timer.Run( m_InitWaitTime, this, "SetActive" );
            
            //if (player)
            //{
            //    player.MessageStatus( m_InfoActivationTime );
            //}
        
            Synch(NULL);
        }
        else
        {
            SetActive();
        }
        */
	}
}