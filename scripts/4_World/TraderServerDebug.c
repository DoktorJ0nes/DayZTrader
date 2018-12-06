//#define Trader_ShowServerLogs

class TraderServerLogs
{
	static void PrintS(string str)
	{
#ifdef Trader_ShowServerLogs
        Print(str);
#endif
    }
}