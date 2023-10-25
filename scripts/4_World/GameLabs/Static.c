static void SendToGameLabsTrader (PlayerBase player, string item, string target, string action) 
{
	#ifdef GAMELABS
    if (GetGameLabs() && GetGameLabs().IsServer())
	{
        _LogPlayerEx logObjectPlayer = new _LogPlayerEx(player);
        action = action + " ";
        _Payload_ItemInteract payload = new _Payload_ItemInteract(logObjectPlayer, item, target, action);
        GetGameLabs().GetApi().ItemInteract(new _Callback(), payload);
    }
	#endif
}