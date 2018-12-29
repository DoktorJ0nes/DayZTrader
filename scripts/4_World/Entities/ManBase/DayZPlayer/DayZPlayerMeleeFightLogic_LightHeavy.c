//modded class DayZPlayerMeleeFightLogic // Will likely be changed to this Class in a future DayZ Update!
modded class DayZPlayerMeleeFightLogic_LightHeavy
{
    override bool Process(int pCurrentCommandID, HumanInputController pInputs, EntityAI pEntityInHands, HumanMovementState pMovementState)
	{
        PlayerBase playerClone = PlayerBase.Cast(m_DZPlayer);
        
        if (playerClone)
        {
            if (playerClone.m_Trader_IsInSafezone)
            return false;
        }

        return super.Process(pCurrentCommandID, pInputs, pEntityInHands, pMovementState);
    }
}