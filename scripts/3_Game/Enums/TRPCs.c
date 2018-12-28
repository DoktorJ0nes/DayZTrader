//#define Trader_Debug

enum TRPCs
{
	// Client -> Server
	RPC_SPAWN_ITEM_ON_GROUND = 35466,
	RPC_SPAWN_VEHICLE,
	RPC_DELETE_VEHICLE,
	RPC_CREATE_ITEM_IN_INVENTORY,
	RPC_DELETE_ITEM,
	RPC_SET_ITEM_AMOUNT,
	RPC_INCREASE_PLAYER_CURRENCY,
	RPC_TRADER_MOD_IS_LOADED,
	RPC_TRADER_SERVER_LOG,
	RPC_BUY,
	RPC_SELL,
#ifdef Trader_Debug
	RPC_DEBUG_TELEPORT,
	RPC_TEST_PLACE_PREVIEW_OBJECT,
	RPC_TEST_PLACE_OBJECT,
	RPC_TEST_DELETE_OBJECT,
	RPC_TEST_REPOS_OBJECT,
	RPC_TEST_REDIR_OBJECT,
	RPC_TEST_SAVE_OBJECTS,
#endif
	
	// Server -> Client
	RPC_SEND_TRADER_CURRENCYTYPE_ENTRY,
	RPC_SEND_TRADER_MANCLASSNAME_ENTRY,
	RPC_SEND_TRADER_NAME_ENTRY,
	RPC_SEND_TRADER_CATEGORY_ENTRY,
	RPC_SEND_TRADER_ITEM_ENTRY,
	RPC_SEND_TRADER_MARKER_ENTRY,
	RPC_SEND_TRADER_DATA_CONFIRMATION,
	RPC_SEND_TRADER_CLEAR,
	RPC_SEND_TRADER_IS_IN_SAFEZONE,
	RPC_SEND_TRADER_PLAYER_DIED_IN_SAFEZONE,
	RPC_TRADER_MOD_IS_LOADED_CONFIRM,
	RPC_SYNC_OBJECT_ORIENTATION,
	RPC_SYNC_CARSCRIPT_ISINSAFEZONE,
	RPC_SEND_MENU_BACK,
}