modded class BarrelHoles_ColorBase
{
    bool IsTraderFireBarrel = false;

    override bool CanPutIntoHands( EntityAI parent )
	{
        if (IsTraderFireBarrel)
            return false;

		return super.CanPutIntoHands( parent );
	}
    
    override void DeferredInit()
	{
		super.DeferredInit();
		if(GetGame().IsServer() && IsTraderFireBarrel)
		{
			SetLifetime(3600 * 24 * 100);
			Synchronize();
		}
	}

    override bool CanReleaseAttachment( EntityAI attachment )
	{
		ItemBase item = ItemBase.Cast( attachment );
		if (item.IsKindOf("FBF_Pot") || item.IsKindOf("FBF_FryingPan"))
		{
			return false;
		}
        return super.CanReleaseAttachment(attachment);
    }	
    
    override void CalcAndSetQuantity()
	{
		if (GetGame().IsServer() && IsTraderFireBarrel) 
		{			
			SetQuantity(10000);
            return;
		}
        super.CalcAndSetQuantity();
	}

    override void SpendFireConsumable(float amount)
	{
		if (GetGame().IsServer() && IsTraderFireBarrel) 
		{			
		    CalcAndSetQuantity();
        }
        else
        {
            super.SpendFireConsumable(amount);
        }
	}

    override void BurnItemsInFireplace()
	{
        if (GetGame().IsServer() && !IsTraderFireBarrel) 
		{	
            super.BurnItemsInFireplace();
            return;
        }

		//! cargo
		CargoBase cargo = GetInventory().GetCargo();
		for (int i = 0; i < cargo.GetItemCount(); i++)
		{
			ItemBase item = ItemBase.Cast(cargo.GetItem(i));
			
			//set damage
			AddDamageToItemByFireEx(item, false, false);

			if (item.GetHealth("", "Health") <= 0 && !item.IsKindOf("Grenade_Base"))
			{
				item.Delete();
			}
			
			//add temperature
			AddTemperatureToItemByFire(item);
			
			//remove wetness
			AddWetnessToItem(item, -PARAM_WET_HEATING_DECREASE_COEF);
		}	
		
		//! attachments
		for (int j = 0; j < GetInventory().AttachmentCount(); ++j)
		{
			ItemBase attachment = ItemBase.Cast(GetInventory().GetAttachmentFromIndex(j));

			//add temperature
			AddTemperatureToItemByFire(attachment);
		
			//remove wetness
			AddWetnessToItem(attachment, -PARAM_WET_HEATING_DECREASE_COEF);
		}
	}
	
	//add wetness on fireplace
	override void AddWetnessToFireplace(float amount)
	{
        if (GetGame().IsServer() && !IsTraderFireBarrel) 
		{	
            super.AddWetnessToFireplace(amount);
            return;
        }
	}
}