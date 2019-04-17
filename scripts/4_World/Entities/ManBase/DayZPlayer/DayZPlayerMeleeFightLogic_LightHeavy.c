modded class DayZPlayerMeleeFightLogic_LightHeavy
{
    override bool HandleFightLogic(int pCurrentCommandID, HumanInputController pInputs, EntityAI pEntityInHands, HumanMovementState pMovementState)
	{
        PlayerBase playerClone = PlayerBase.Cast(m_DZPlayer);
        
        if (playerClone)
        {
            if (playerClone.m_Trader_IsInSafezone)
            return false;
        }

        return super.HandleFightLogic(pCurrentCommandID, pInputs, pEntityInHands, pMovementState);
    }
}