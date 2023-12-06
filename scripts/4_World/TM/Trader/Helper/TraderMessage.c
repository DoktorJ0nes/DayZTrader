class TraderMessage
{
    static void PlayerWhite(string message, PlayerBase player, float time = 20)
    {
        if (!player)
            return;
        SendNotification(message, player, time, 0);
    }

    static void PlayerRed(string message, PlayerBase player, float time = 20)
    {
        if (!player)
            return;
        SendNotification(message, player, time, 0xFFFA6B6B);
    }

    static void SafezoneExit(PlayerBase player, float time)
    {
        if (!player)
            return;

        SendNotification("", player, time, 0xFFFA6B6B, true);
    }

    static void DeleteSafezoneMessages(PlayerBase player)
    {
        if (!player)
            return;

        if (GetGame().IsServer())
        {
            GetGame().RPCSingleParam(player, TRPCs.RPC_DELETE_SAFEZONE_MESSAGES, new Param1<bool>( true ), false, player.GetIdentity());
        }
    }

    static void SendNotification(string message, PlayerBase player, float time, int color, bool isExitSafezoneMsg = false)
    {        
        if (GetGame().IsServer())
        {
            GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_NOTIFICATION, new Param4<string, float, int, bool>( message, time, color, isExitSafezoneMsg), false, player.GetIdentity());
        }
        else
        {
            if(isExitSafezoneMsg)
            {
                player.ShowExitSafezoneMessage(time);
            }
            else
            {
                player.ShowTraderMessage(message, time, color);
            }
        }
    }
}