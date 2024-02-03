modded class PlayerBase
{
    protected bool m_IsInSafeZone = false;
    bool m_Trader_IsTrader = false;
	bool m_Trader_SafezoneShowDebugShapes = false;
	protected int m_SafeZoneCount = 0;
	protected int m_PrevSafeZoneCount = 0;
    ref TraderMenu m_TraderMenu;
	
	void PlayerBase()
	{
		RegisterNetSyncVariableBool("m_IsInSafeZone");
		RegisterNetSyncVariableBool("m_Trader_SafezoneShowDebugShapes");
		RegisterNetSyncVariableBool("m_Trader_RecievedAllData");
	}

	bool IsInSafeZone()
	{
		return m_IsInSafeZone;
	}

	void AddSafeZoneTrigger()
	{
		if (!GetGame().IsServer())
			return;
		m_PrevSafeZoneCount = m_SafeZoneCount;
		m_SafeZoneCount++;
		//Print("Adding to count: " + m_SafeZoneCount + ", in safezone: " + m_IsInSafeZone);
		
		if (m_SafeZoneCount > 0)
		{
			if (!m_IsInSafeZone)
			{
				SendToGameLabsTrader(this, "", "", "has entered a safezone");
				SetAllowDamage(false);
				SetInSafeZone(true);
			}
			if(m_PrevSafeZoneCount == 0)
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OnExitSafeZoneCountdownComplete);
				TraderMessage.DeleteSafezoneMessages(this);
				TraderMessage.PlayerGreen("#tm_entered_safezone", this);
			}
		}
	}

	void RemoveSafeZoneTrigger(int exitTimer)
	{
		if (!GetGame().IsServer())
			return;

		if (m_SafeZoneCount > 0)
		{
			m_SafeZoneCount--;
			//Print("Removing from count: " + m_SafeZoneCount + ", in safezone: " + m_IsInSafeZone);
		}
		
		if (m_IsInSafeZone && m_SafeZoneCount == 0)
		{
			TraderMessage.SafezoneExit(this, exitTimer);			
			if(exitTimer > 0)
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OnExitSafeZoneCountdownComplete, exitTimer * 1000, false);
			}
			else
			{
				OnExitSafeZoneCountdownComplete();
			}
		}
	}

	protected void OnExitSafeZoneCountdownComplete()
	{
		TraderMessage.DeleteSafezoneMessages(this);
		TraderMessage.PlayerRed("#tm_left_safezone", this);

		SendToGameLabsTrader(this, "", "", "has left a safezone");
		
		bool godModeVPP, godModeCOT;
		EnScript.GetClassVar(this, "hasGodmode", 0, godModeVPP);
		EnScript.GetClassVar(this, "m_COT_GodMode", 0, godModeCOT);

		if (!godModeVPP && !godModeCOT)
			SetAllowDamage(true);

		SetInSafeZone(false);
	}
	
	void SetInSafeZone(bool state)
	{
		m_IsInSafeZone = state;
		SetSynchDirty();
	}
	
	bool Trader_IsAdmin()
    {
        for (int i = 0; i < m_Trader_AdminPlayerUIDs.Count(); i++)
        {
            if (m_Trader_AdminPlayerUIDs.Get(i) == m_Trader_PlayerUID)
                return true;
        }

        return false;
    }

    override void Init()
    {
        super.Init();

        RegisterNetSyncVariableBool("m_Trader_IsTrader");
    }

	override void SetSuicide(bool state)
	{
		super.SetSuicide(state);

		if (state && IsInSafeZone() && GetGame().IsServer())
			SetAllowDamage(true);
	}

    override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);

		AddAction(ActionUnlockVehicle, InputActionMap);
        AddAction(ActionLockVehicle, InputActionMap);
        AddAction(ActionUnlockVehicleInside, InputActionMap);
        AddAction(ActionLockVehicleInside, InputActionMap);
        AddAction(ActionTrade, InputActionMap);
	}
}