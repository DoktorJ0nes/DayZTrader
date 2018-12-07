modded class CarScript
{
    string m_Trader_OwnerPlayerUID = "NO_OWNER";

    bool m_Trader_IsInSafezone = false;

    /*override void Init()
	{
		super.Init();

		RegisterNetSyncVariableBool("m_Trader_IsInSafezone"); // doesnt do anything???????
	}*/

    /*override bool CanReleaseAttachment( EntityAI attachment ) // For Cars added in future Releases
	{
        if (m_Trader_IsInSafezone)
            return false;

        return super.CanReleaseAttachment(attachment);
    }*/
}