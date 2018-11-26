class TraderMenu extends UIScriptedMenu
{
    MultilineTextWidget m_InfoBox;
    ButtonWidget m_BtnBuy;
	ButtonWidget m_BtnSell;
	ButtonWidget m_BtnCancel;
	TextListboxWidget m_ListboxItems;
	TextWidget m_Saldo;
	TextWidget m_SaldoValue;
	TextWidget m_TraderName;
	XComboBoxWidget m_XComboboxCategorys;
	ItemPreviewWidget m_ItemPreviewWidget;
	protected EntityAI previewItem;
	MultilineTextWidget m_ItemDescription;
	TextWidget m_ItemWeight;
	float m_UiUpdateTimer = 0;
	
	static const string filePath = "DZ/Trader/scripts/5_Mission/mission/TraderConfig.txt";
	
	int m_TraderID = -1;
	vector m_TraderVehicleSpawn = "0 0 0";
	vector m_TraderVehicleSpawnOrientation = "0 0 0";
	
	int m_Player_CurrencyAmount;
	int m_ColorBuyable;
	int m_ColorTooExpensive;
	int m_CategorysCurrentIndex;

	int m_LastRowIndex = -1;
	int m_LastCategoryCurrentIndex = -1;
	
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
		PlayerBase player = g_Game.GetPlayer();
		player.GetInputController().SetDisabled(false);

		if ( previewItem ) 
		{
			GetGame().ObjectDelete( previewItem );
		}
	}

    override Widget Init()
    {
		layoutRoot = GetGame().GetWorkspace().CreateWidgets( "DZ/Trader/scripts/5_Mission/mission/TraderMenu.layout" );

        m_BtnBuy = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_buy" ) );
		m_BtnSell = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_sell" ) );
		m_BtnCancel = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_cancel" ) );
		m_ListboxItems = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("txtlist_items") );
		m_Saldo = TextWidget.Cast(layoutRoot.FindAnyWidget("text_saldo") );
		m_SaldoValue = TextWidget.Cast(layoutRoot.FindAnyWidget("text_saldoValue") );
		m_TraderName = TextWidget.Cast(layoutRoot.FindAnyWidget("title_text") );
		m_XComboboxCategorys = XComboBoxWidget.Cast( layoutRoot.FindAnyWidget( "xcombobox_categorys" ) );
		m_ItemDescription = MultilineTextWidget.Cast( layoutRoot.FindAnyWidget( "ItemDescWidget" ) );
		m_ItemWeight = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemWeight") );
		
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

			local int row_index = m_ListboxItems.GetSelectedRow();
			if ((m_LastRowIndex != row_index) || (m_LastCategoryCurrentIndex != m_CategorysCurrentIndex))
			{
				m_LastRowIndex = row_index;
				m_LastCategoryCurrentIndex = m_CategorysCurrentIndex;

				string itemType = m_ListboxItemsClassnames.Get(row_index);
				updateItemPreview(itemType);
			}

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

		PPEffects.SetBlurMenu(0.5);

		PlayerBase player = g_Game.GetPlayer();
		player.GetInputController().SetDisabled(true);

		SetFocus( layoutRoot );
	}

	override void OnHide()
	{
		super.OnHide();

		PPEffects.SetBlurMenu(0);

		PlayerBase player = g_Game.GetPlayer();
		player.GetInputController().SetDisabled(false);

		if ( previewItem ) 
		{
			GetGame().ObjectDelete( previewItem );
		}

		Close();
	}

    /*override void OnHide()
    {
        super.OnHide();
		
		Close();
    }*/

	override bool OnClick( Widget w, int x, int y, int button )
	{
		super.OnClick(w, x, y, button);

		PlayerBase m_Player = g_Game.GetPlayer();
		
		local int row_index = m_ListboxItems.GetSelectedRow();
		string itemType = m_ListboxItemsClassnames.Get(row_index);
		int itemQuantity = m_ListboxItemsQuantity.Get(row_index);
		
		if ( w == m_BtnBuy )
		{
			int itemCosts = m_ListboxItemsBuyValue.Get(row_index);
			
			int playerCurrencyAmountBeforePurchase = m_Player_CurrencyAmount;
			
			if (m_Player_CurrencyAmount < itemCosts)
			{
				m_Player.MessageStatus("Trader: Sorry, but you can't afford that!");
				return true;
			}

			if (itemQuantity == -2) // Is a Vehicle
			{
				if (!IsVehicleSpawnFree())
				{
					m_Player.MessageStatus("Trader: Something is blocking the Way!");
					return true;
				}

				m_Player.MessageStatus("Trader: " + getItemDisplayName(itemType) + " was parked next to you!");

				GetGame().RPCSingleParam(m_Player, TRPCs.RPC_SPAWN_VEHICLE, new Param3<vector, vector, string>( m_TraderVehicleSpawn, m_TraderVehicleSpawnOrientation ,itemType), true);

				GetGame().GetUIManager().Back();
			}
			else // Is not a Vehicle
			{			
				if (canCreateItemInPlayerInventory(itemType))
				{
					m_Player.MessageStatus("Trader: " + getItemDisplayName(itemType) + " was added to your Inventory!");
					
					createItemInPlayerInventory(itemType, itemQuantity);
				}
				else
				{
					m_Player.MessageStatus("Trader: Your Inventory is full! " + getItemDisplayName(itemType) + " was placed on Ground!");
					
					vector playerPosition = m_Player.GetPosition();
					
					spawnItemOnGround(itemType, playerPosition, itemQuantity);
					
					GetGame().GetUIManager().Back();
				}
			}
			
			deductPlayerCurrency(itemCosts);
			updatePlayerCurrencyAmount();
			updateItemListboxColors();
			
			m_Player_CurrencyAmount = playerCurrencyAmountBeforePurchase - itemCosts;
			updatePlayerCurrencyAmount();				
			updateItemListboxColors();

			return true;
		}
		
		if ( w == m_BtnSell )
		{		
			Object vehicleToSell = GetVehicleToSell(itemType);
			bool isValidVehicle = (itemQuantity == -2 && vehicleToSell);

			if (!isInPlayerInventory(itemType, itemQuantity) && !isValidVehicle)
			{
				m_Player.MessageStatus("Trader: Sorry, but you can't sell that!");
				return true;
			}
			
			int playerCurrencyAmountBeforeSale = m_Player_CurrencyAmount;
			
			m_Player.MessageStatus("Trader: " + getItemDisplayName(itemType) + " was sold!");

			if (isValidVehicle)
				sellVehicle(vehicleToSell);
			else
				removeFromPlayerInventory(itemType, itemQuantity);
			
			int itemSellValue = m_ListboxItemsSellValue.Get(row_index);
			increasePlayerCurrency(itemSellValue);
			updatePlayerCurrencyAmount();
			updateItemListboxColors();

			m_Player_CurrencyAmount = playerCurrencyAmountBeforeSale + itemSellValue;
			updatePlayerCurrencyAmount();			
			updateItemListboxColors();
			
			return true;
		}
		
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
		updateListbox = true;
		
		return false;
	}
	
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
				m_ListboxItems.SetItemColor(i, 1, ARGBF(1, 1, 1, 1) );
			}
			else
			{
				m_ListboxItems.SetItemColor(i, 1, ARGBF(1, 1, 0, 0) );
			}
			
			string itemClassname = m_ListboxItemsClassnames.Get(i);
			int itemQuantity = m_ListboxItemsQuantity.Get(i);
			
			if (isInPlayerInventory(itemClassname, itemQuantity) || (itemQuantity == -2 && GetVehicleToSell(itemClassname)))
			{
				m_ListboxItems.SetItemColor(i, 2, ARGBF(1, 0, 1, 0) );
			}
			else
			{
				m_ListboxItems.SetItemColor(i, 2, ARGBF(1, 1, 1, 1) );
			}

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

	private void updateItemPreview(string itemType)
	{
		if ( !m_ItemPreviewWidget )
			{
				Widget preview_frame = layoutRoot.FindAnyWidget("ItemFrameWidget");

				if ( preview_frame ) 
				{
					float width;
					float height;
					preview_frame.GetSize(width, height);

					m_ItemPreviewWidget = ItemPreviewWidget.Cast( GetGame().GetWorkspace().CreateWidget(ItemPreviewWidgetTypeID, 0, 0, 1, 1, WidgetFlags.VISIBLE, ARGB(255, 255, 255, 255), 10, preview_frame) );
				}
			}

			if ( previewItem )
				GetGame().ObjectDelete( previewItem );

			previewItem = GetGame().CreateObject( itemType, "0 0 0", true, false, true );

			m_ItemPreviewWidget.SetItem( previewItem );
			m_ItemPreviewWidget.SetModelPosition( Vector(0,0,0.5) );

			float itemx, itemy;		
			m_ItemPreviewWidget.GetPos(itemx, itemy);

			m_ItemPreviewWidget.SetSize( 1.5, 1.5 );

			// align to center 
			m_ItemPreviewWidget.SetPos( -0.225, -0.225 );

			// update Item Description:
			InventoryItem iItem = InventoryItem.Cast( previewItem );

			if (iItem)
			{
				m_ItemWeight.SetText(GetItemWeightText());
				m_ItemDescription.SetText(TrimUntPrefix(iItem.GetTooltip()));
			}
			else
			{
				m_ItemWeight.SetText("");
				m_ItemDescription.SetText("...");
			}
	}

	private string GetItemWeightText()
	{
		ItemBase item_IB = ItemBase.Cast( previewItem );

		int weight = item_IB.GetItemWeight();
		//string weightStr = "";
		
		if (weight >= 1000)
		{
			int kilos = Math.Round(weight / 1000.0);
			return  "#inv_inspect_about" + " " + kilos.ToString() + " " + "#inv_inspect_kg";
		}
		else if (weight >= 500)
		{
			return "#inv_inspect_under_1";
		} 
		else if (weight >= 250)
		{
			return "#inv_inspect_under_05";
		}
		else 
		{
			return "#inv_inspect_under_025";
		}

		return "ERROR";
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
		GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_SPAWN_ITEM_ON_GROUND, rp2, true);
	}
	
	private void createItemInPlayerInventory(string itemType, int amount)
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		Param3<PlayerBase, string, int> rp1 = new Param3<PlayerBase, string, int>(m_Player, itemType, amount);
		GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_CREATE_ITEM_IN_INVENTORY, rp1, true);
		
		m_Player.UpdateInventoryMenu();
	}
	
	private int getPlayerCurrencyAmount()
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		int currencyAmount = 0;
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase item;
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			if(item && item.GetType() == m_Player.m_Trader_CurrencyItemType)
			{
				currencyAmount += getItemAmount(item);
			}
		}
		
		return currencyAmount;
	}
	
	private bool isInPlayerInventory(string itemClassname, int amount)
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
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
					GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_DELETE_ITEM, rp1, true);
					
					m_Player.UpdateInventoryMenu();
					return true;
				}
				
				setItemAmount(item, itemAmount - amount);
				
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
		if( item.IsMagazine() )
		{
			itemAmount = mgzn.GetAmmoCount();
		}
		else
		{
			itemAmount = QuantityConversions.GetItemQuantity(item);
		}
		
		return itemAmount;
	}
	
	private void setItemAmount(ItemBase item, int amount)
	{
		Param2<ItemBase, int> rp1 = new Param2<ItemBase, int>(item, amount);
		GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_SET_ITEM_AMOUNT, rp1, true);
		
		g_Game.GetPlayer().UpdateInventoryMenu();
	}
	
	private void increasePlayerCurrency(int currencyAmount)
	{
		PlayerBase m_Player = g_Game.GetPlayer();
		
		Param3<PlayerBase, string, int> rp1 = new Param3<PlayerBase, string, int>( m_Player, m_Player.m_Trader_CurrencyItemType, currencyAmount);
		GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_INCREASE_PLAYER_CURRENCY, rp1, true);
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
				int itemCurrencyAmount = getItemAmount(item);
				
				if(itemCurrencyAmount > currencyAmount)
				{
					setItemAmount(item, itemCurrencyAmount - currencyAmount);
					return;
				}
				else
				{
					Param1<ItemBase> rp1 = new Param1<ItemBase>(itemsArray.Get(i));
					GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_DELETE_ITEM, rp1, true);
					
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
			cfg = "CfgAmmo " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			cfg = "CfgMagazines " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			cfg = "cfgWeapons " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
	
		if (displayName == "")
		{
			cfg = "CfgNonAIVehicles " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		
		if (displayName != "")
			return TrimUntPrefix(displayName);
		else
			return itemClassname;
	}

	private bool IsVehicleSpawnFree()
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		return !(GetGame().IsBoxColliding( m_TraderVehicleSpawn, m_TraderVehicleSpawnOrientation, size, excluded_objects, nearby_objects));
	}

	private Object GetVehicleToSell(string vehicleClassname)
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		if (GetGame().IsBoxColliding( m_TraderVehicleSpawn, m_TraderVehicleSpawnOrientation, size, excluded_objects, nearby_objects))
		{
			for (int i = 0; i < nearby_objects.Count(); i++)
			{
				if (nearby_objects.Get(i).GetType() == vehicleClassname)
				{
					// Check if there is any Player in the Vehicle:
					bool vehicleIsEmpty = true;

					Transport transport;
					Class.CastTo(transport, nearby_objects.Get(i))
					if (transport)
					{
						int crewSize = transport.CrewSize();
						for (int c = 0; c < crewSize; c++)
						{
							if (transport.CrewMember(c))
								vehicleIsEmpty = false;
						}
					}
					else
					{
						continue;
					}

					if (!vehicleIsEmpty)
						continue;

					// Check if Engine is running:
					Car car;
					Class.CastTo(car, nearby_objects.Get(i));
					if (car && vehicleIsEmpty)
					{
						if (car.EngineIsOn())
							return nearby_objects.Get(i);
					}					
				}					
			}
		}

		return NULL;
	}

	private void sellVehicle(Object vehicleToSell)
	{
		GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_DELETE_VEHICLE, new Param1<Object>(vehicleToSell), true);
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

	/*private string GetItemInventorySlot(string itemClassname)
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
				//if ( parentEntity.IsWeapon() )
				//{
				//	attachments_slots.Insert( "magazine" );
				//}
			}
		}
		//if ( parentEntity.IsWeapon() )
		//{
		//	attachments_slots.Insert( "magazine" );
		//}

		//TEST();

		return attachments_slots;
	}*/

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
			if ( m_Player.m_Trader_ItemsCategoryId.Get(i) == m_CategorysKey.Get(m_CategorysCurrentIndex) )
			{
				m_ListboxItemsClassnames.Insert(m_Player.m_Trader_ItemsClassnames.Get(i));
				m_ListboxItemsQuantity.Insert(m_Player.m_Trader_ItemsQuantity.Get(i));
				m_ListboxItemsBuyValue.Insert(m_Player.m_Trader_ItemsBuyValue.Get(i));
				m_ListboxItemsSellValue.Insert(m_Player.m_Trader_ItemsSellValue.Get(i));
			}
		}
		
		return true;
	}

	private string TrimUntPrefix(string str)
	{
		str.Replace("$UNT$", "");
		return str;
	}
}