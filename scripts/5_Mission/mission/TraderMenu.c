class TraderMenu extends UIScriptedMenu
{
    MultilineTextWidget m_InfoBox;
    ButtonWidget m_BtnBuy;
	ButtonWidget m_BtnSell;
	ButtonWidget m_BtnCancel;
	//ButtonWidget m_BtnShow;
	TextListboxWidget m_ListboxItems;
	TextWidget m_Saldo;
	TextWidget m_SaldoValue;
	TextWidget m_TraderName;
	XComboBoxWidget m_XComboboxCategorys;
	float m_UiUpdateTimer = 0;
	
	ref InspectMenuNew menu;
	
	
	//static const string m_TraderType = "ZmbM_SoldierNormal"; //"SurvivorM_Boris";
	static const string filePath = "DZ/Trader/scripts/5_Mission/mission/TraderConfig.txt";
	//static const string filePath = "$profile:TraderConfig.txt";
	
	int m_TraderID = -1;
	
	int m_Player_CurrencyAmount;
	int m_ColorBuyable;
	int m_ColorTooExpensive;
	int m_CategorysCurrentIndex;
	
	bool updateListbox = false;
	
	ref array<string> m_Categorys; // unnoetig ????
	ref array<int> m_CategorysTraderKey;
	ref array<int> m_CategorysKey;
	
	ref array<string> m_ListboxItemsClassnames;
	ref array<int> m_ListboxItemsQuantity;
	ref array<int> m_ListboxItemsBuyValue;
	ref array<int> m_ListboxItemsSellValue;
	
	ref TStringArray m_FileContent;

	void TraderMenu()
	{		
		m_Player_CurrencyAmount = 0;
		m_ColorBuyable = 0;
		m_ColorTooExpensive = 1;
	}	
	
	void ~TraderMenu()
	{
	}

    override Widget Init()
    {		
        //layoutRoot = GetGame().GetWorkspace().CreateWidgets( "scripts/5_Mission/mission/TraderMenu.layout" );
		layoutRoot = GetGame().GetWorkspace().CreateWidgets( "DZ/Trader/scripts/5_Mission/mission/TraderMenu.layout" );

        m_BtnBuy = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_buy" ) );
		m_BtnSell = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_sell" ) );
		m_BtnCancel = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_cancel" ) );
		//m_BtnShow = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_show" ) );
		m_ListboxItems = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("txtlist_items") );
		m_Saldo = TextWidget.Cast(layoutRoot.FindAnyWidget("text_saldo") );
		m_SaldoValue = TextWidget.Cast(layoutRoot.FindAnyWidget("text_saldoValue") );
		m_TraderName = TextWidget.Cast(layoutRoot.FindAnyWidget("title_text") );
		m_XComboboxCategorys = XComboBoxWidget.Cast( layoutRoot.FindAnyWidget( "xcombobox_categorys" ) );
		
		m_Categorys = new array<string>;
		m_CategorysTraderKey = new array<int>;
		m_CategorysKey = new array<int>;
        m_ListboxItemsClassnames = new array<string>;
		m_ListboxItemsQuantity = new array<int>;
		m_ListboxItemsBuyValue = new array<int>;
		m_ListboxItemsSellValue = new array<int>;
		
		LoadFileValues();
		m_CategorysCurrentIndex = 0;
		
		updateItemListboxContent();		
		m_ListboxItems.SelectRow(0);

		updatePlayerCurrencyAmount();
		updateItemListboxColors();
		
        return layoutRoot;
    }
	
	override void Update(float timeslice)
	{
		super.Update(timeslice);
		
		if (m_UiUpdateTimer >= 0.05)
		{
			updatePlayerCurrencyAmount();				
			updateItemListboxColors();
			
			PlayerBase m_Player = GetGame().GetPlayer();
			
			m_UiUpdateTimer = 0;
		}
		else
		{
			m_UiUpdateTimer = m_UiUpdateTimer + timeslice;
		}
	}

    override void OnShow()
    {
        super.OnShow();
		
		/*PlayerBase player = g_Game.GetPlayer();
		if (player.m_Trader_RecievedAllData == false)
		{			
			player.MessageStatus("[Trader] MISSING TRADER DATA FROM SERVER!");
			player.MessageStatus("[Trader] TRYING TO GET TRADER DATA FROM SERVER..");
			
			Param1<PlayerBase> rp1 = new Param1<PlayerBase>( g_Game.GetPlayer() );
			GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_REQUEST_TRADER_DATA, rp1, true); // TO SERVER: requestTraderData();
			
			GetGame().GetUIManager().Back();
		}*/
    }

    override void OnHide()
    {
        super.OnHide();
		
		//PlayerBase m_Player = g_Game.GetPlayer();
		//m_Player.MessageStatus("+was hided!!!");
		
		Close();
    }

	override bool OnClick( Widget w, int x, int y, int button )
	{
		super.OnClick(w, x, y, button);

		PlayerBase m_Player = g_Game.GetPlayer();
		
		local int row_index = m_ListboxItems.GetSelectedRow();
		string itemType = m_ListboxItemsClassnames.Get(row_index);
		int itemQuantity = m_ListboxItemsQuantity.Get(row_index);
		
		if ( w == m_BtnBuy )
		{	
			//string itemCostsStr = "";
			//m_ListboxItems.GetItemText( row_index, 1, itemCostsStr );
			//int itemCosts = itemCostsStr.ToInt();
			int itemCosts = m_ListboxItemsBuyValue.Get(row_index);
			
			int playerCurrencyAmountBeforePurchase = m_Player_CurrencyAmount;
			
			if (m_Player_CurrencyAmount < itemCosts)
			{
				m_Player.MessageStatus("Trader: Sorry, but you can't afford that!");
				return true;
			}
			
			//EntityAI entity = g_Game.GetPlayer().SpawnEntityOnGroundPos(itemType, m_Player.GetPosition()); // ghostitem to check if there is space in inventory // nicht sichtbar fuer clients?
			
			//if (m_Player.GetHumanInventory().CanAddEntityToInventory(entity))
			if (canCreateItemInPlayerInventory(itemType))
			{
				m_Player.MessageStatus("Trader: " + getItemDisplayName(itemType) + " was added to your Inventory!");
				//entity.Delete();
				
				createItemInPlayerInventory(itemType, itemQuantity);
			}
			else
			{
				m_Player.MessageStatus("Trader: Your Inventory is full! " + getItemDisplayName(itemType) + " was placed on Ground!");
				
				vector playerPosition = m_Player.GetPosition();
				
				spawnItemOnGround(itemType, playerPosition, itemQuantity);
				
				GetGame().GetUIManager().Back();
			}
			
			deductPlayerCurrency(itemCosts);
			updatePlayerCurrencyAmount();
			updateItemListboxColors();
			//m_UiUpdateTimer = m_UiUpdateTimerInterval;
			
			//if (playerCurrencyAmountBeforePurchase == m_Player_CurrencyAmount) // only at the first purchase m_Player_CurrencyAmount stays like before, when UI keeps open.. WTF?! (EnumerateInventory() seems to be the reason)
			//{
				//m_Player.MessageStatus("FIXING BALANCE AND UI..");
			m_Player_CurrencyAmount = playerCurrencyAmountBeforePurchase - itemCosts;
			updatePlayerCurrencyAmount();				
			updateItemListboxColors();
			//}

			return true;
		}
		
		if ( w == m_BtnSell )
		{			
			if (!isInPlayerInventory(itemType, itemQuantity))
			{
				m_Player.MessageStatus("Trader: Sorry, but you can't sell that!");
				return true;
			}
			
			int playerCurrencyAmountBeforeSale = m_Player_CurrencyAmount;
			
			m_Player.MessageStatus("Trader: " + getItemDisplayName(itemType) + " was sold!");
			removeFromPlayerInventory(itemType, itemQuantity);
			
			int itemSellValue = m_ListboxItemsSellValue.Get(row_index);
			increasePlayerCurrency(itemSellValue);
			updatePlayerCurrencyAmount();
			updateItemListboxColors();
			//m_UiUpdateTimer = m_UiUpdateTimerInterval;
			
			//if (playerCurrencyAmountBeforeSale == m_Player_CurrencyAmount) // only at the first sale m_Player_CurrencyAmount stays like before, when UI keeps open.. WTF?! (EnumerateInventory() seems to be the reason)
			//{
			m_Player_CurrencyAmount = playerCurrencyAmountBeforeSale + itemSellValue;
			updatePlayerCurrencyAmount();			
			updateItemListboxColors();
			//}
			
			return true;
		}
		
		/*if ( w == m_BtnShow )
		{
			m_Player.MessageStatus("[Trader] Preview " + itemType);
			EntityAI entityToInspect = g_Game.GetPlayer().SpawnEntityOnGroundPos(itemType, m_Player.GetPosition());
			InspectItem(entityToInspect);
			entityToInspect.Delete();

			return true;
		}*/
		
		if ( w == m_BtnCancel )
		{
			GetGame().GetUIManager().Back();

			return true;
		}
		
		if (w == m_XComboboxCategorys)
		{
			if (updateListbox)
			{
				updateListbox = false;
				
				updateItemListboxContent();
				m_ListboxItems.SelectRow(0);

				updatePlayerCurrencyAmount();
				updateItemListboxColors();
			}
		}

		return false;
	}
	
	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		super.OnChange(w, x, y, finished);
		if (!finished) return false;
		
		m_CategorysCurrentIndex = m_XComboboxCategorys.GetCurrentItem();
		
		//PlayerBase m_Player = g_Game.GetPlayer();
		//m_Player.MessageStatus("ONCHANGE! " + m_CategorysCurrentIndex);
		
		//if(!GetGame().IsClient())
		//	return false;
		
		//updateItemListboxContent();
		updateListbox = true;

		/*PlayerBase m_Player = g_Game.GetPlayer();
		EntityAI entityInHands = m_Player.GetHumanInventory().GetEntityInHands();
		if (entityInHands)
		{
			m_Player.MessageStatus("SOMETHING IN HANDS!");
			bool testIA = IsAttachment(entityInHands, "AK_Suppressor");
			m_Player.MessageStatus("ISATTACHMENT = " + testIA);
		}
		else
			m_Player.MessageStatus("NOTHING IN HANDS!");*/
		
		return false;
	}
	
	/*override bool OnUpdate(Widget w)
	{
		if (updateListbox)
		{
			updateListbox = false;
			updateItemListboxContent();
		}
	}*/
	
	private void updateItemListboxContent()
	{		
		//------------------------------------------------------
		
		LoadItemsFromFile();
		
		//------------------------------------------------------
		
		m_ListboxItems.ClearItems();
		
		for ( int i = 0; i < m_ListboxItemsClassnames.Count(); i++ )
		{
			m_ListboxItems.AddItem(getItemDisplayName(m_ListboxItemsClassnames.Get(i)), NULL, 0 );	
			m_ListboxItems.SetItem( i, "" + m_ListboxItemsBuyValue.Get(i), 				NULL, 1 );
			m_ListboxItems.SetItem( i, "" + m_ListboxItemsSellValue.Get(i), 			NULL, 2 );
		}
	}
	
	private void updateItemListboxColors()
	{
		for (int i = 0; i < m_ListboxItems.GetNumItems(); i++)
		{
			int itemCosts = m_ListboxItemsBuyValue.Get(i);
			
			if (m_Player_CurrencyAmount >= itemCosts)
			{
				//m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 1, 1, 1) );
				m_ListboxItems.SetItemColor(i, 1, ARGBF(1, 1, 1, 1) );
			}
			else
			{
				//m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 1, 0, 0) );
				m_ListboxItems.SetItemColor(i, 1, ARGBF(1, 1, 0, 0) );
			}
			
			string itemClassname = m_ListboxItemsClassnames.Get(i);
			int itemQuantity = m_ListboxItemsQuantity.Get(i);
			
			if (isInPlayerInventory(itemClassname, itemQuantity))
			{
				m_ListboxItems.SetItemColor(i, 2, ARGBF(1, 0, 1, 0) );
			}
			else
			{
				m_ListboxItems.SetItemColor(i, 2, ARGBF(1, 1, 1, 1) );
			}

			//PlayerBase m_Player = g_Game.GetPlayer();
			EntityAI entityInHands = g_Game.GetPlayer().GetHumanInventory().GetEntityInHands();
			if (entityInHands)
			{
				if (IsAttached(entityInHands, itemClassname))
					m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 0.4, 0.4, 1) );
				else if (IsAttachment(entityInHands, itemClassname))
					m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 0.7, 0.7, 1) );
				else
					m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 1, 1, 1) );
			}
		}
	}
	
	private void updatePlayerCurrencyAmount()
	{
		PlayerBase m_Player = g_Game.GetPlayer();

		m_Player_CurrencyAmount = 0;
		m_Player_CurrencyAmount = getPlayerCurrencyAmount();
		m_SaldoValue.SetText(" " + m_Player_CurrencyAmount);
	}
	
	private bool canCreateItemInPlayerInventory(string itemType)
	{
		PlayerBase m_Player = g_Game.GetPlayer();

		InventoryLocation il = new InventoryLocation;
		
		if (g_Game.GetPlayer().GetInventory().FindFirstFreeLocationForNewEntity(itemType, FindInventoryLocationType.ANY, il))
		{
			return true;
		}

		EntityAI entityInHands = m_Player.GetHumanInventory().GetEntityInHands();
		if (!entityInHands)
			return true;

		return false;			
	}
	
	private void spawnItemOnGround(string itemType, vector position, int amount)
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		Param4<PlayerBase, string, vector, int> rp2 = new Param4<PlayerBase, string, vector, int>(m_Player, itemType, position, amount);
		GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_SPAWN_ITEM_ON_GROUND, rp2, true); // TO SERVER: spawnEntityOnGround(itemType, m_Player.GetPosition());
	}
	
	private void createItemInPlayerInventory(string itemType, int amount)
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		Param3<PlayerBase, string, int> rp1 = new Param3<PlayerBase, string, int>(m_Player, itemType, amount);
		GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_CREATE_ITEM_IN_INVENTORY, rp1, true); // TO SERVER: createInInventory(m_Player, itemType);
		
		//ref Param3<string, float, float> params = new Param3<string, float, float>(itemType, 100, amount);
		//m_Player.RPCSingleParam(ERPCs.DEV_RPC_SPAWN_ITEM_IN_INVENTORY, params, true);
		
		m_Player.UpdateInventoryMenu();
	}
	
	private int getPlayerCurrencyAmount()
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		int currencyAmount = 0;
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		/*EntityAI entityInHands = m_Player.GetHumanInventory().GetEntityInHands();
		if (entityInHands)
			itemsArray.Insert(entityInHands);*/

		ItemBase item;
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			if(item && item.GetType() == m_Player.m_Trader_CurrencyItemType)
			{
				//currencyAmount += QuantityConversions.GetItemQuantity(item);
				currencyAmount += getItemAmount(item);
			}
		}
		
		/*
		// remove double counted items because item was also in hand
		ItemBase entityInHands;
		Class.CastTo(entityInHands, m_Player.GetHumanInventory().GetEntityInHands());
		if(entityInHands && entityInHands.GetType() == m_Player.m_Trader_CurrencyItemType)
			currencyAmount -= getItemAmount(entityInHands);	
		//EntityAI entityInHands = m_Player.GetHumanInventory().GetEntityInHands();
		//if(item && item.GetType() == m_Player.m_Trader_CurrencyItemType)
			//currencyAmount -= QuantityConversions.GetItemQuantity(entityInHands);	
		
		//m_Player.MessageStatus("currencyAmount: " + currencyAmount);	
		*/	
		
		return currencyAmount;
	}
	
	private bool isInPlayerInventory(string itemClassname, int amount)
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		//int currencyAmount = 0;
		
		array<EntityAI> itemsArray = new array<EntityAI>;		
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		ItemBase item;
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			if(item && item.GetType() == itemClassname && getItemAmount(item) >= amount)
			{
				return true;
			}
		}
		
		return false;
	}
	
	private bool removeFromPlayerInventory(string itemClassname, int amount)
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		ItemBase item;
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			if(item && item.GetType() == itemClassname && getItemAmount(item) >= amount)
			{				
				int itemAmount = 0;
				itemAmount = getItemAmount(item);
				
				if (itemAmount <= amount)
				{
					ItemBase itemToDelete;
					Class.CastTo(itemToDelete, itemsArray.Get(i));
					
					Param1<ItemBase> rp1 = new Param1<ItemBase>(itemToDelete);
					GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_DELETE_ITEM, rp1, true); // TO SERVER: deleteItem(item);
					
					m_Player.UpdateInventoryMenu();
					return true;
				}
				
				setItemAmount(item, itemAmount - amount); // TO SERVER: deductItemAmount(item, amount);
				
				m_Player.UpdateInventoryMenu();
				return true;
			}
		}
		
		m_Player.UpdateInventoryMenu();
		return false;
	}
	
	private int getItemAmount(ItemBase item)
	{
		Magazine mgzn = Magazine.Cast(item);
				
		int itemAmount = 0;
		if( item.IsMagazine() ) // is a magazine
		{
			itemAmount = mgzn.GetAmmoCount();
		}
		else // is not a magazine
		{
			itemAmount = QuantityConversions.GetItemQuantity(item);
		}
		
		return itemAmount;
	}
	
	private void setItemAmount(ItemBase item, int amount)
	{
		Param2<ItemBase, int> rp1 = new Param2<ItemBase, int>(item, amount);
		GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_SET_ITEM_AMOUNT, rp1, true); // TO SERVER: setItemAmount(item, amount);
		
		g_Game.GetPlayer().UpdateInventoryMenu();
	}
	
	/*private int getItemMaxAmount(ItemBase item) // wird das noch iwo benutzt?
	{
		Magazine mgzn = Magazine.Cast(item);
				
		int itemMaxAmount = 0;
		if( item.IsMagazine() ) // is a magazine
		{
			itemMaxAmount = mgzn.GetAmmoMax();
		}
		else // is not a magazine
		{
			itemMaxAmount = item.GetQuantityMax();
		}
		
		return itemMaxAmount;
	}*/
	
	private void increasePlayerCurrency(int currencyAmount)
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		Param3<PlayerBase, string, int> rp1 = new Param3<PlayerBase, string, int>( m_Player, m_Player.m_Trader_CurrencyItemType, currencyAmount);
		GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_INCREASE_PLAYER_CURRENCY, rp1, true); // TO SERVER: increasePlayerCurrency(player, currencyType, currencyAmount);
		
		/*while (currencyAmount > 0)
		{
			EntityAI entity = m_Player.SpawnEntityOnGroundPos(m_Player.m_Trader_CurrencyItemType, m_Player.GetPosition());
			
			ItemBase item;
			Class.CastTo(item, entity);			
			int itemMaxAmount = getItemMaxAmount(item);
			m_Player.MessageStatus("AMOUNTMAX: " + itemMaxAmount + "!");
			
			if (m_Player.GetHumanInventory().CanAddEntityToInventory(entity))
			{
				createItemInPlayerInventory(m_Player.m_Trader_CurrencyItemType, currencyAmount)
			}
			else
			{
				m_Player.MessageStatus("Your Inventory is full! Your Currencys were placed on Ground!");
				
				spawnItemOnGround(m_Player.m_Trader_CurrencyItemType, m_Player.GetPosition(), currencyAmount);
				GetGame().GetUIManager().Back();
			}
			GetGame().ObjectDelete(entity);
			
			//--------------------------------------------
			ItemBase item;
			Class.CastTo(item, entity);
			
			int itemMaxAmount = getItemMaxAmount(item);
			
			if (currencyAmount > itemMaxAmount)
			{
				setItemAmount(item, itemMaxAmount);
				currencyAmount -= itemMaxAmount;
			}
			else
			{
				setItemAmount(item, currencyAmount);
				currencyAmount = 0;
			}
		}*/
	}
	
	private void deductPlayerCurrency(int currencyAmount)
	{		
		PlayerBase m_Player = g_Game.GetPlayer();
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		ItemBase item;
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			if(item && item.GetType() == m_Player.m_Trader_CurrencyItemType)
			{
				/*Magazine mgzn = Magazine.Cast(item);
				
				int itemCurrencyAmount = 0;
				if( item.IsMagazine() ) // is a magazine //////////// <- falsch herum? A tauschen mit B?
				{
					//itemCurrencyAmount = QuantityConversions.GetItemQuantity(item); // A
					itemCurrencyAmount = mgzn.GetAmmoCount(); // B
				}
				else // is not a magazine
				{
					//itemCurrencyAmount = mgzn.GetAmmoCount(); // B
					itemCurrencyAmount = QuantityConversions.GetItemQuantity(item); // A
				}*/
				int itemCurrencyAmount = getItemAmount(item);
				
				if(itemCurrencyAmount > currencyAmount)
				{
					/*if( item.IsMagazine() ) // is a magazine
					{
						m_Player.MessageStatus("Currency is Magazine!");	
						mgzn.ServerSetAmmoCount(itemCurrencyAmount - currencyAmount);
						return;
					}
					else // is not a magazine
					{
						m_Player.MessageStatus("Currency is NO Magazine!");
						item.SetQuantity(itemCurrencyAmount - currencyAmount);
						return;
					}*/
					setItemAmount(item, itemCurrencyAmount - currencyAmount);
					return;
				}
				else
				{
					//GetGame().ObjectDelete(itemsArray.Get(i));
					Param1<ItemBase> rp1 = new Param1<ItemBase>(itemsArray.Get(i));
					GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_DELETE_ITEM, rp1, true); // TO SERVER: deleteItem(item);
					
					m_Player.UpdateInventoryMenu();
					
					currencyAmount -= itemCurrencyAmount;
				}
			}
		}
	}
	
	private string getItemDisplayName(string itemClassname)
	{
		TStringArray itemInfos = new TStringArray;
		
		string cfg = "CfgVehicles " + itemClassname + " displayName";
		string displayName;
		GetGame().ConfigGetText(cfg, displayName);
	
		if (displayName == "")
		{
			//GetGame().ConfigGetFullPath("CfgAmmo " + itemClassname, itemInfos);
			cfg = "CfgAmmo " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			//GetGame().ConfigGetFullPath("CfgMagazines " + itemClassname, itemInfos);
			cfg = "CfgMagazines " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			//GetGame().ConfigGetFullPath("cfgWeapons " + itemClassname, itemInfos);
			cfg = "cfgWeapons " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
	
		if (displayName == "")
		{
			//GetGame().ConfigGetFullPath("CfgNonAIVehicles " + itemClassname, itemInfos);
			cfg = "CfgNonAIVehicles " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		
		
		
		if (displayName != "")
			return displayName;
		else
			return itemClassname;
	}

	private bool IsAttached(EntityAI parentEntity, string attachmentClassname)
	{
		for ( int i = 0; i < parentEntity.GetInventory().AttachmentCount(); i++ )
		{
			EntityAI attachment = parentEntity.GetInventory().GetAttachmentFromIndex ( i );
			if ( attachment.IsKindOf ( attachmentClassname ) )
				return true;
		}

		return false;
	}

	private bool IsAttachment(EntityAI parentEntity, string attachmentClassname)
	{
		//PlayerBase m_Player = g_Game.GetPlayer();

		// Check chamberable Ammunitions
		string type_name = parentEntity.GetType();
		TStringArray cfg_chamberables = new TStringArray;

		GetGame().ConfigGetTextArray(CFG_WEAPONSPATH + " " + type_name + " chamberableFrom", cfg_chamberables);

		for (int i = 0; i < cfg_chamberables.Count(); i++)
		{
			if (attachmentClassname == cfg_chamberables[i])
				return true;
		}


		// Check Magazines
		TStringArray cfg_magazines = new TStringArray;

		GetGame().ConfigGetTextArray(CFG_WEAPONSPATH + " " + type_name + " magazines", cfg_magazines);

		for (i = 0; i < cfg_magazines.Count(); i++)
		{
			if (attachmentClassname == cfg_magazines[i])
				return true;
		}


		// Check non-Ammo Attachments (TODO with "Cfg ... randomAttachments")
		/*
		string attachmentInventorySlot = GetItemInventorySlot(attachmentClassname);
		if (attachmentInventorySlot == string.Empty || attachmentInventorySlot == "weaponOptics")
			return false;

		array<string> attachments_slots = GetItemAttachmentSlots(parentEntity.GetType());

		for (i = 0; i < attachments_slots.Count(); i++)
		{
			if (attachments_slots.Get(i) == attachmentInventorySlot)
				return true;
		}*/


		return false;


		/*string attachmentInventorySlot;
		g_Game.ConfigGetText(CFG_VEHICLESPATH + " " + attachmentClassname + " inventorySlot", attachmentInventorySlot);
		m_Player.MessageStatus("ITEM: " + attachmentInventorySlot);*/


		/*TStringArray searching_in = new TStringArray;
		searching_in.Insert( CFG_VEHICLESPATH );
		searching_in.Insert( CFG_WEAPONSPATH );
		searching_in.Insert( CFG_MAGAZINESPATH );

		array<string> attachments_slots	= new array<string>;

		for ( int s = 0; s < searching_in.Count(); ++s )
		{
			string cfg_name = searching_in.Get( s );
			string path = cfg_name + " " + parentEntity.GetType();

			if ( GetGame().ConfigIsExisting( path ) )
			{
				g_Game.ConfigGetTextArray( path + " attachments", attachments_slots );
				if ( parentEntity.IsWeapon() )
				{
					attachments_slots.Insert( "magazine" );
				}
			}
		}
		if ( parentEntity.IsWeapon() )
		{
			attachments_slots.Insert( "magazine" );
		}

		for (int i = 0; i < attachments_slots.Count(); i++)
		{
			string slot_name = attachments_slots.Get ( i );

			string namePath = "CfgSlots" + " Slot_" + slot_name;
			GetGame().ConfigGetText( namePath + " name", slot_name );

			m_Player.MessageStatus("HANDS: " + slot_name);

			if (slot_name == attachmentInventorySlot)
				return true;

			
			//int cfg_count = GetGame().ConfigGetChildrenCount(namePath);
			//m_Player.MessageStatus("WOW: " + cfg_count);
			//for (int j = 0; j < cfg_count; j++)
			//{
			//	string childName;
			//	GetGame().ConfigGetChildName(namePath, j, childName);
			//	m_Player.MessageStatus("WOWW: " + childName);
			//}
		}

		return false;*/


		// Check with Ghostentity
		/*EntityAI entity = g_Game.GetPlayer().SpawnEntityOnGroundPos(attachmentClassname, vector.Zero); 
		m_Player.MessageStatus("DEBUG: Placed " + entity.GetDisplayName());

		if ( parentEntity.GetInventory() && parentEntity.GetInventory().CanAddAttachment( entity ) )
		{
			entity.Delete();
			return true;
		}

		entity.Delete();
		return false;*/


		/*
		//string			type_name = entity.GetType();
		string			type_name = parentEntity.GetType();
		TStringArray	cfg_attachments = new TStringArray;
		
		string cfg_path;
		
		if ( GetGame().ConfigIsExisting(CFG_VEHICLESPATH+" "+type_name) )
		{
			cfg_path = CFG_VEHICLESPATH+" "+type_name+" attachments";
		}
		else if ( GetGame().ConfigIsExisting(CFG_WEAPONSPATH+" "+type_name) )
		{
			cfg_path = CFG_WEAPONSPATH+" "+type_name+" attachments";
		}
		else if ( GetGame().ConfigIsExisting(CFG_MAGAZINESPATH+" "+type_name) )
		{
			cfg_path = CFG_MAGAZINESPATH+" "+type_name+" attachments";
		}
		
		GetGame().ConfigGetTextArray(cfg_path, cfg_attachments);

		//GetGame().ConfigGetTextArray("cfgVehicles " + type_name + " itemInfo", cfg_attachments);
		GetGame().ConfigGetTextArray(cfg_path, cfg_attachments);

		for (int i = 0; i < cfg_attachments.Count(); i++)
		{
			m_Player.MessageStatus(cfg_attachments[i]);

			if (attachmentClassname == cfg_attachments[i].GetType)
			return true;
		}

		return false;*/

		/*string tesstr;
		TStringArray cfg_attachments = new TStringArray;
		string type_name = parentEntity.GetType();
		string path = CFG_WEAPONSPATH + " " + type_name + " chamberableFrom";
		//int cfg_count = GetGame().ConfigGetChildrenCount(path);

		
		GetGame().ConfigGetTextArray(path, cfg_attachments);
		//tesstr = GetGame().ConfigGetTextOut(path);
		//cfg_attachments.Insert(tesstr);

		m_Player.MessageStatus("(" + cfg_attachments.Count() + ") CFGs: " + path + ":");
		Print("(" + cfg_attachments.Count() + ") CFGs: " + path + ":");

		for (int i = 0; i < cfg_attachments.Count(); i++)
		{
			m_Player.MessageStatus("x   " + cfg_attachments[i]);
			Print("x   " + cfg_attachments[i]);
		}

		return false;*/
	}

	private string GetItemInventorySlot(string itemClassname)
	{
		TStringArray searching_in = new TStringArray;
		searching_in.Insert( CFG_VEHICLESPATH );
		searching_in.Insert( CFG_WEAPONSPATH );
		searching_in.Insert( CFG_MAGAZINESPATH );

		string inventorySlot = string.Empty;

		for ( int s = 0; s < searching_in.Count(); ++s )
		{
			string cfg_name = searching_in.Get( s );
			string path = cfg_name + " " + itemClassname + " inventorySlot";

			if ( GetGame().ConfigIsExisting( path ) )
			{
				g_Game.ConfigGetText( path, inventorySlot );

				return inventorySlot;
			}
		}

		return inventorySlot;
	}

	private array<string> GetItemAttachmentSlots(string itemClassname)
	{
		TStringArray searching_in = new TStringArray;
		searching_in.Insert( CFG_VEHICLESPATH );
		searching_in.Insert( CFG_WEAPONSPATH );
		searching_in.Insert( CFG_MAGAZINESPATH );

		array<string> attachments_slots	= new array<string>;

		for ( int s = 0; s < searching_in.Count(); ++s )
		{
			string cfg_name = searching_in.Get( s );
			string path = cfg_name + " " + itemClassname;

			if ( GetGame().ConfigIsExisting( path ) )
			{
				g_Game.ConfigGetTextArray( path + " attachments", attachments_slots );
				/*if ( parentEntity.IsWeapon() )
				{
					attachments_slots.Insert( "magazine" );
				}*/
			}
		}
		/*if ( parentEntity.IsWeapon() )
		{
			attachments_slots.Insert( "magazine" );
		}*/

		//TEST();

		return attachments_slots;
	}

	/*private void TEST()
	{
		string classname = "CZ61";

		TStringArray searching_in = new TStringArray;
		searching_in.Insert( CFG_VEHICLESPATH );
		searching_in.Insert( CFG_WEAPONSPATH );
		searching_in.Insert( CFG_MAGAZINESPATH );

		ref array<array<string>> entrys = new array<array<string>>;

		for ( int s = 0; s < searching_in.Count(); ++s )
		{
			string cfg_name = searching_in.Get( s );
			string path = cfg_name + " " + classname;

			if ( GetGame().ConfigIsExisting( path ) )
			{
				g_Game.ConfigGetTextArray( path + " randomAttachments", entrys );

				for (int i = 0; i < entrys.Count(); i++)
				{
					Print("ENTRYS: " + entrys.Get(i));
				}
			}
		}
	}*/

	/*static void GetBaseConfigClasses( out TStringArray base_classes )
	{
		base_classes.Clear();
		base_classes.Insert(CFG_VEHICLESPATH);
		base_classes.Insert(CFG_WEAPONSPATH);
		base_classes.Insert(CFG_MAGAZINESPATH);
		base_classes.Insert(CFG_RECIPESPATH);
	}

	static void GetAllConfigClasses( string search_string, out TStringArray filtered_classes, bool only_public = false )
	{	
		TStringArray searching_in = new TStringArray;
		GetBaseConfigClasses( searching_in );
		
		filtered_classes.Clear();
		
		search_string.ToLower();
		
		for ( int s = 0; s < searching_in.Count(); ++s )
		{
			string config_path = searching_in.Get(s);
			
			int objects_count = GetGame().ConfigGetChildrenCount(config_path);
			for (int i = 0; i < objects_count; i++)
			{
				string childName;
				GetGame().ConfigGetChildName(config_path, i, childName);
	
				//if ( only_public )
				//{
				//	int scope = GetGame().ConfigGetInt( config_path + " " + childName + " scope" );
				//	if ( scope == 0 )
				//	{
				//		continue;
				//	}
				//}
				
				string nchName = childName;
				nchName.ToLower();
	
				//if ( nchName.Contains(search_string) != -1)
				//{
				filtered_classes.Insert(childName);
				Print("CGFs ChildName: " + childName);
				
				int objects_count2 = GetGame().ConfigGetChildrenCount(config_path + " " + childName);
				for (int i2 = 0; i2 < objects_count2; i2++)
				{
					string childName2;
					GetGame().ConfigGetChildName(config_path + " " + childName, i2, childName2);
					//GetGame().ConfigGetTextArray(cfg_path, cfg_attachments);

					filtered_classes.Insert(childName2);
					Print("---CGFs ChildName2: " + childName2);
				}
				//}
			}
		}
	}*/

	/*array<string> GetItemSlots(EntityAI e)
	{
		TStringArray searching_in = new TStringArray;
		searching_in.Insert(CFG_VEHICLESPATH);
		searching_in.Insert(CFG_WEAPONSPATH);
		searching_in.Insert(CFG_MAGAZINESPATH);

		array<string> attachments_slots	= new array<string>;
		
		for ( int s = 0; s < searching_in.Count(); ++s )
		{
			string cfg_name = searching_in.Get(s);
			string path = cfg_name + " " + e.GetType();   

			if ( GetGame().ConfigIsExisting( path ) )
			{
				g_Game.ConfigGetTextArray(path + " attachments", attachments_slots);
				if ( e.IsWeapon() )
				{
					attachments_slots.Insert("magazine");
				}
				return attachments_slots;
			}
		}
		if ( e.IsWeapon() )
		{
			attachments_slots.Insert("magazine");
		}
		return attachments_slots;
	}*/
	
	private bool LoadFileValues()
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		m_TraderName.SetText(m_Player.m_Trader_TraderNames.Get(m_TraderID));
		m_Saldo.SetText("(" + getItemDisplayName(m_Player.m_Trader_CurrencyItemType) + ") Balance: ");

		m_XComboboxCategorys.ClearAll();
		m_Categorys = new array<string>;
		m_CategorysTraderKey = new array<int>;
		m_CategorysKey = new array<int>;
		
		for ( int i = 0; i < m_Player.m_Trader_Categorys.Count(); i++ )
		{
			if (m_TraderID != m_Player.m_Trader_CategorysTraderKey.Get(i))
				continue;
			
			m_XComboboxCategorys.AddItem(m_Player.m_Trader_Categorys.Get(i));
			m_Categorys.Insert(m_Player.m_Trader_Categorys.Get(i));
			m_CategorysTraderKey.Insert(m_Player.m_Trader_CategorysTraderKey.Get(i));
			m_CategorysKey.Insert(i);
		}
		
		//----------------------------------------------------------------------
		
		/*PlayerBase m_Player = g_Game.GetPlayer();	
		
		m_FileContent.Clear();
		//string filePath = "com/trader/scripts/5_Mission/mission/TraderConfig.txt";
		FileHandle file_index = OpenFile(filePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			m_Player.MessageStatus("FOUND NO TRADERCONFIG FILE!");
			return false;
		}
		
		//m_Player.MessageStatus("FOUND FILE!");
		
		string line_content = "";
		
		
		line_content = SearchForNextTermInFile(file_index, "<Currency>", "");
		line_content.Replace("<Currency>", "");
		line_content = TrimComment(line_content);
		m_CurrencyItemType = line_content;
				
		
		line_content = SearchForNextTermInFile(file_index, "<Trader>", "");
		line_content.Replace("<Trader>", "");
		line_content = TrimComment(line_content);
		m_TraderName.SetText(line_content);
		
		
		m_XComboboxCategorys.ClearAll();
		m_Categorys = new array<string>;		
		int categoryCounter = 0;
		line_content = TrimComment(SearchForNextTermInFile(file_index, "<Category>", "<TraderEnd>"));
		while (categoryCounter <= 500 && line_content != "<TraderEnd>")
		{
			line_content.Replace("<Category>", "");
			m_XComboboxCategorys.AddItem(TrimComment(line_content));
			m_Categorys.Insert(TrimComment(line_content));
			
			line_content = TrimComment(SearchForNextTermInFile(file_index, "<Category>", "<TraderEnd>"));
			categoryCounter++;
		}
		
		
		CloseFile(file_index);*/
		
		return true;
	}
	
	private bool LoadItemsFromFile()
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		m_ListboxItemsClassnames = new array<string>;
		m_ListboxItemsQuantity = new array<int>;
		m_ListboxItemsBuyValue = new array<int>;
		m_ListboxItemsSellValue = new array<int>;
			
		for ( int i = 0; i < m_Player.m_Trader_ItemsClassnames.Count(); i++ )
		{
			//if ( m_Player.m_Trader_ItemsTraderId.Get(i) != m_CategorysTraderKey.Get(m_CategorysCurrentIndex))
			//	continue;
			
			if ( m_Player.m_Trader_ItemsCategoryId.Get(i) == m_CategorysKey.Get(m_CategorysCurrentIndex) )
			{
				m_ListboxItemsClassnames.Insert(m_Player.m_Trader_ItemsClassnames.Get(i));
				m_ListboxItemsQuantity.Insert(m_Player.m_Trader_ItemsQuantity.Get(i));
				m_ListboxItemsBuyValue.Insert(m_Player.m_Trader_ItemsBuyValue.Get(i));
				m_ListboxItemsSellValue.Insert(m_Player.m_Trader_ItemsSellValue.Get(i));
			}
		}
		
		/*int lineCount = 1;		
		
		m_FileContent.Clear();
		//string filePath = "com/trader/scripts/5_Mission/mission/TraderConfig.txt";
		
		FileHandle file_index = OpenFile(filePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			//m_Player.MessageStatus("FOUND NO FILE!");
			return false;
		}
		//m_Player.MessageStatus("FOUND FILE!");
		
		
		//m_Player.MessageStatus("CATEGORY = " + m_Categorys.Get(m_CategorysCurrentIndex));
		bool categoryFound = false;
		int itemCounter = 0;
		while (itemCounter <= 5000 && !categoryFound)
		{
			string line_content = TrimComment(SearchForNextTermInFile(file_index, "<Category>", ""));
			line_content.Replace("<Category>", "");
			line_content = TrimComment(line_content);
			
			if (line_content != m_Categorys.Get(m_CategorysCurrentIndex))
			{
				//m_Player.MessageStatus("CONTINUED CATEGORY " + line_content);
				continue;
			}
			
			//m_Player.MessageStatus("FOUND CATEGORY " + line_content);
			categoryFound = true;
			
			
			
			m_ListboxItemsClassnames = new array<string>;
			m_ListboxItemsBuyValue = new array<int>;
			m_ListboxItemsSellValue = new array<int>;
			
			int char_count = 0;
			while ( itemCounter <= 5000 && char_count != -1 )
			{			
				char_count = FGets( file_index,  line_content );
				
				if (line_content.Contains("<Category>"))
					return true;
				
				line_content = TrimComment(line_content);
				
				if (!line_content.Contains(","))
					continue;
			
				TStringArray strs = new TStringArray;
				line_content.Split( ",", strs );
				
				string item = strs.Get(0);
				item = TrimSpaces(item);
				
				string buy = strs.Get(1);
				buy = TrimSpaces(buy);
				
				string sell = strs.Get(2);
				sell = TrimSpaces(sell);
				
				//m_Player.MessageStatus("ITEM \"" + item + "\"");
				//m_Player.MessageStatus("BUY \"" + buy + "\"");
				//m_Player.MessageStatus("SELL \"" + sell + "\"");
				m_ListboxItemsClassnames.Insert(item);
				m_ListboxItemsBuyValue.Insert(buy.ToInt());
				m_ListboxItemsSellValue.Insert(sell.ToInt());
				
				itemCounter++;
			}		
			
			itemCounter++;
		}
		
		
		CloseFile(file_index);*/
		
		return true;
	}
	
	/*private string SearchForNextTermInFile(FileHandle file_index, string searchTerm, string abortTerm)
	{
		int char_count = 0;
		while ( char_count != -1 )
		{			
			string line_content = "";
			char_count = FGets( file_index,  line_content );
			
			if (line_content.Contains(searchTerm) || (line_content.Contains(abortTerm) && abortTerm != ""))
			{
				return line_content;
			}
		}
		
		return "";
	}
	
	private string TrimComment(string line)
	{
		int to_substring_end = line.Length();
		
		for (int i = 0; i < to_substring_end; i++)
		{
			string sign = line.Get(i);
			if ( sign == "/" && i + 1 < to_substring_end)
			{
				if (line.Get(i + 1) == "/")
				{
					to_substring_end = i;
					break;
				}
			}
		}
		
		string lineOut = line.Substring(0, to_substring_end);
		
		return TrimSpaces(lineOut);
	}
	
	private string TrimSpaces(string line)
	{
		line.Replace("	", ""); // Replace Tabs("\t" or "	") with nothing.
		
		bool hasSpaces = true;		
		while(hasSpaces)
		{
			line = line.Trim();
			
			if (line.Length() > 0)
				hasSpaces = line.Get(0) == " " || line.Get(line.Length() - 1) == " ";
			else
				hasSpaces = false;
		}
		
		return line;
	}*/
	
	private void InspectItem(EntityAI itemToInspect)
	{
		//InventoryItem item = InventoryItem.Cast( itemToInspect );
		//ItemBase item = ItemBase.Cast(itemToInspect);
		
		//InventoryMenuNew m_inventory_menu = InventoryMenuNew.Cast( GetGame().GetUIManager().FindMenu(MENU_INVENTORY) );
		/*InspectMenuNew inspect_menu = InspectMenuNew.Cast( menu.EnterScriptedMenu(MENU_INSPECT) );
		if ( inspect_menu )
		{
			inspect_menu.SetItem(item);
		}*/
		
		//GetGame().GetUIManager().ShowUICursor(true);
		
		/* klappt so halb.. */
		/*ItemBase item = ItemBase.Cast(itemToInspect);
		menu = new InspectMenuNew;
		menu.Init();
		menu.SetItem(item);
		GetGame().GetUIManager().ShowScriptedMenu( menu, this );*/
		
		/*if ( menu )
		{
			menu.SetItem(item);
		}*/
		
		/*if (m_inventory_menu == NULL)
			{
				m_inventory_menu = InventoryMenu.Cast( GetUIManager().EnterScriptedMenu(MENU_INVENTORY, NULL) );
			}
			else if ( GetUIManager().FindMenu(MENU_INVENTORY) == NULL )
			{
				GetUIManager().ShowScriptedMenu(m_inventory_menu, NULL);
			}
			init = true;*/
		
		
		
		
		//InspectMenuNew inspect = InspectMenuNew.Cast( GetUIManager().FindMenu(MENU_INSPECT) );
		//InspectMenu inspect_menu = InspectMenu.Cast( this.EnterScriptedMenu(MENU_INSPECT) );
		//InspectMenuNew inspect_menu = InspectMenuNew.Cast( this.EnterScriptedMenu(MENU_INSPECT) );
		//if (inspect_menu)
		//if (inspect)
		/*{
			inspect_menu.SetItem(item);
			inspect.SetItem(item);
		}*/
		
		//InspectMenu inspect_menu = InspectMenu.Cast( this.EnterScriptedMenu(MENU_INSPECT) );
		//GetGame().GetUIManager().EnterScriptedMenu(MENU_INSPECT, NULL);
		//GetGame().GetUIManager().ShowScriptedMenu( inspect_menu, NULL );
	}
}