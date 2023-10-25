modded class VONManagerBase
{
	void SetInSafeZone(bool state) {}
}

modded class VONManagerImplementation extends VONManagerBase
{
	protected bool m_IsInSafeZone;
	protected int m_VoiceLevelSafeZone = -1;

	override void SetInSafeZone(bool state)
	{
		m_IsInSafeZone = state;
		
		if (state)
		{
			m_VoiceLevelSafeZone = GetGame().GetVoiceLevel();
			GetGame().SetVoiceLevel(0);
		}
		else
		{
			if (m_VoiceLevelSafeZone > -1)
			{
				GetGame().SetVoiceLevel(m_VoiceLevelSafeZone);
			}
			else
			{
				GetGame().SetVoiceLevel(1);
			}
		}
	}
	
	override void HandleInput(Input inp)
	{
		if (m_IsInSafeZone)
		{
			if (inp.LocalPress("UAVoiceDistanceUp", false) || inp.LocalPress("UAVoiceDistanceDown", false))
				ShowVoiceNotification(GetGame().GetVoiceLevel(), !GetGame().GetMission().IsVoNActive());
			
			return;
		}
		
		super.HandleInput(inp);
	}
}