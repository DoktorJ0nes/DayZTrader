//All rights reserved
class TM_InventoryTransactions
{
    //1. Create item local on server
    //2. Check it can be created. No point continuing through loop if not
    //3. CreateInInventory of player
    //4. Try CreateInInventory in attachments of player if #3 fails
    //5. Create on ground if allowed to and #4 failed
    static EntityAI CreateItemInPlayerInventory(string classname, notnull PlayerBase target, out InventoryLocationType locationType, bool createOnGroundOnFailure = true)
    {        
        #ifdef SERVER
        //create item and
        //initial check to make sure we can create this class name
        EntityAI newItemLocal = EntityAI.Cast(GetGame().CreateObjectEx(classname,  target.GetPosition(), ECE_PLACE_ON_SURFACE|ECE_LOCAL));
        if(!newItemLocal)
        {
            locationType = InventoryLocationType.UNKNOWN;
            return null;
        }
        else
        {
            GetGame().ObjectDelete(newItemLocal);
        }

        InventoryLocation newItemInvLoc = new InventoryLocation;
        EntityAI newItem = EntityAI.Cast(target.GetInventory().CreateInInventory(classname));
        if (newItem)
        {
            newItem.GetInventory().GetCurrentInventoryLocation(newItemInvLoc);
            locationType = newItemInvLoc.GetType();
            return newItem;
        }
        else
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
                    newItem = EntityAI.Cast(attachment.GetInventory().CreateInInventory(classname));
                    if (newItem)
                    {
			            newItem.GetInventory().GetCurrentInventoryLocation(newItemInvLoc);
                        locationType = newItemInvLoc.GetType();
                        return newItem;
                    }
                }
            }
        }

        if(createOnGroundOnFailure && !newItem)
        {
            newItem = EntityAI.Cast(GetGame().CreateObjectEx(classname,  target.GetPosition(), ECE_PLACE_ON_SURFACE));
            locationType = InventoryLocationType.GROUND;
            return newItem;
        }

        
        locationType = InventoryLocationType.UNKNOWN;
        #endif

        return null;
    }
};