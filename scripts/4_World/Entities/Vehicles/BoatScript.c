modded class BoatScript
{
    string m_Trader_LastDriverId = "";

    override void OnEngineStart()
	{
        super.OnEngineStart();

        PlayerBase player = PlayerBase.Cast(CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER));
        if (player)
            m_Trader_LastDriverId = player.GetIdentity().GetId();
	}
};