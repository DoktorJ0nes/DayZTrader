class TraderMessage
{
	static void ServerLog(string str)
	{
#ifndef TRADER_HIDE_SERVER_LOGS        
        PluginTraderServerLog m_Logger = PluginTraderServerLog.Cast(GetPlugin(PluginTraderServerLog));
        if(m_Logger)
        {
            m_Logger.Log(str);
        }
        else
        {
            Print(str);
        }
#endif
    }

	static void TradesLog(string str)
	{
#ifndef TRADER_HIDE_SERVER_LOGS        
        PluginTraderTradesLog m_Logger = PluginTraderTradesLog.Cast(GetPlugin(PluginTraderTradesLog));
        if(m_Logger)
        {
            m_Logger.Log(str);
        }
        else
        {
            Print(str);
        }
#endif
    }

    static void PlayerWhite(string message, PlayerBase player, float time = 10)
    {
        if (!player)
            return;
        SendNotification(message, player, time, 0);
    }

    static void PlayerRed(string message, PlayerBase player, float time = 10)
    {
        if (!player)
            return;
        SendNotification(message, player, time, COLOR_RED);
    }

    static void PlayerGreen(string message, PlayerBase player, float time = 10)
    {
        if (!player)
            return;
        SendNotification(message, player, time, COLOR_GREEN);
    }

    static void SafezoneExit(PlayerBase player, float time)
    {
        if (!player)
            return;

        SendNotification("", player, time, COLOR_RED, true);
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
                player.GetTraderNotifications().ShowExitSafezoneMessage(time);
            }
            else
            {
                player.GetTraderNotifications().ShowMessage(message, time, color);
            }
        }
    }
}