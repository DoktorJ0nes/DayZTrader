modded class Ammunition_Base: Magazine_Base
{
    override int AddQuantityTR(float amount)
	{
		if(IsAmmoPile())
		{
			int this_free_space = GetAmmoMax() - GetAmmoCount();
            if(this_free_space == 0)
                return amount;
            if( amount >= this_free_space )
            {
                ServerAddAmmoCount(this_free_space);
			    SetSynchDirty();
                return amount - this_free_space;
            }
            else
            {
                ServerAddAmmoCount(amount);
			    SetSynchDirty();
                return 0;
            }			
        }
        
        return -1;
    }

    override int SetQuantityTR(float amount)
	{
		if(IsAmmoPile())
		{
			int maxAmount = GetAmmoMax();
            if( amount >= maxAmount )
            {
                ServerSetAmmoCount(maxAmount);
			    SetSynchDirty();
                return amount - maxAmount;
            }
            else
            {
                ServerSetAmmoCount(amount);
			    SetSynchDirty();
                return 0;
            }			
        }
        
        return -1;
    }
};