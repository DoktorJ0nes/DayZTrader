modded class WeaponManager
{
	override bool CanFire(Weapon_Base wpn)
	{
		if ( m_player.IsInSafeZone() )
			return false;
		
		return super.CanFire(wpn);
	}
}