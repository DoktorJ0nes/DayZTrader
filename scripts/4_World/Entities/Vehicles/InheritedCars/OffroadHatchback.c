modded class OffroadHatchback
{
    override bool CanReleaseAttachment( EntityAI attachment )
	{
        if (m_Trader_IsInSafezone)
            return false;

        return super.CanReleaseAttachment(attachment);
    }
}