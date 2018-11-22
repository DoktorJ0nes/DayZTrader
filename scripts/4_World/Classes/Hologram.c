modded class Hologram
{
    override void EvaluateCollision()
    {
        super.EvaluateCollision();

        if (m_Player.m_Trader_IsInSafezone)
            SetIsColliding( true );
    }
}