class TM_InventoryTransactions
{
    static EntityAI CreateItem(string classname, notnull EntityAI target, out InventoryLocationType locationType, bool createOnGroundOnFailure = true)
    {
        #ifdef SERVER
        EntityAI newItemLocal = EntityAI.Cast(GetGame().CreateObjectEx(classname,  target.GetPosition(), ECE_PLACE_ON_SURFACE|ECE_LOCAL));
        if(!newItemLocal)
        {
            locationType = InventoryLocationType.UNKNOWN;
            return null;
        }
        InventoryLocation sourceInvLoc = new InventoryLocation();
        newItemLocal.GetInventory().GetCurrentInventoryLocation(sourceInvLoc);
        InventoryLocation targetInvLoc = new InventoryLocation();
		if (target.GetInventory().FindFreeLocationFor(newItemLocal, FindInventoryLocationType.CARGO | FindInventoryLocationType.ATTACHMENT, targetInvLoc))
		{
            if(GameInventory.LocationCanMoveEntity(sourceInvLoc, targetInvLoc))
            {
                switch (targetInvLoc.GetType())
                {
                    case InventoryLocationType.ATTACHMENT:
                        if(targetInvLoc.GetParent().CanReceiveAttachment(newItemLocal, targetInvLoc.GetSlot()))
                        {
                            if(target.GetInventory().TakeToDst(InventoryMode.LOCAL, sourceInvLoc, targetInvLoc))
                            {
                                locationType = InventoryLocationType.ATTACHMENT;
                                GetGame().RemoteObjectTreeCreate(newItemLocal);
                                return newItemLocal;
                            }
                        }
                        break;
                    case InventoryLocationType.CARGO:
                        if(targetInvLoc.GetParent().CanReceiveItemIntoCargo(newItemLocal))
                        {                            
                            if(target.GetInventory().TakeToDst(InventoryMode.LOCAL, sourceInvLoc, targetInvLoc))
                            {
                                GetGame().RemoteObjectTreeCreate(newItemLocal);
                                locationType = InventoryLocationType.CARGO;
                                return newItemLocal;
                            }
                        }
                        break;
                    case InventoryLocationType.HANDS:
                        break;
                    default:
                        Error("CreateInInventory: unknown location for item");
                        break;
                }
            }
		}
        if(createOnGroundOnFailure)
        {
            locationType = InventoryLocationType.GROUND;
            GetGame().RemoteObjectTreeCreate(newItemLocal);
            return newItemLocal;
        }

        locationType = InventoryLocationType.UNKNOWN;
        GetGame().ObjectDelete(newItemLocal);
        #endif

		return null;
    }


    //1. check item can be created on local
    //2. Try create in inventory of player directly
    //3. Try create in attachments 
    //4. Create on ground if allowed to
    static EntityAI CreateItemInPlayerInventory(string classname, notnull PlayerBase target, out InventoryLocationType locationType, bool createOnGroundOnFailure = true)
    {        
        #ifdef SERVER
        //initial check to make sure we can create this class name
        EntityAI newItemLocal = EntityAI.Cast(GetGame().CreateObjectEx(classname,  target.GetPosition(), ECE_PLACE_ON_SURFACE|ECE_LOCAL));
        if(!newItemLocal)
        {
            locationType = InventoryLocationType.UNKNOWN;
            return null;
        }
        EntityAI newItem = EntityAI.Cast(TM_InventoryTransactions.CreateItem(classname, target, locationType, false));
        if (!newItem)
        {
            for (int i = 0; i < target.GetInventory().GetAttachmentSlotsCount(); i++)
		    {
                int slot = target.GetInventory().GetAttachmentSlotId(i);
                EntityAI attachment = target.GetInventory().FindAttachment(slot);
                if (attachment)
                {
                    if(!attachment.GetInventory().GetCargo() && attachment.GetInventory().GetAttachmentSlotsCount() <= 0)
                    {
                        continue;
                    }
                    newItem = EntityAI.Cast(TM_InventoryTransactions.CreateItem(classname, attachment, locationType, false));
                    if (newItem)
                    {
                        break;
                    }
                }
            }
        }
        if(newItem)
        {
            GetGame().ObjectDelete(newItemLocal);
            return newItem;
        }

        if(createOnGroundOnFailure && !newItem)
        {
            locationType = InventoryLocationType.GROUND;
            GetGame().RemoteObjectTreeCreate(newItemLocal);
            return newItemLocal;
        }

        GetGame().ObjectDelete(newItemLocal);
        locationType = InventoryLocationType.UNKNOWN;
        #endif

        return null;
    }
};