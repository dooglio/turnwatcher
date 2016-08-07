//===============================================================================
// Copyright (c) 2005-2013 by Made to Order Software Corporation
// 
// All Rights Reserved.
// 
// The source code in this file ("Source Code") is provided by Made to Order Software Corporation
// to you under the terms of the GNU General Public License, version 2.0
// ("GPL").  Terms of the GPL can be found in doc/GPL-license.txt in this distribution.
// 
// By copying, modifying or distributing this software, you acknowledge
// that you have read and understood your obligations described above,
// and agree to abide by those obligations.
// 
// ALL SOURCE CODE IN THIS DISTRIBUTION IS PROVIDED "AS IS." THE AUTHOR MAKES NO
// WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
// COMPLETENESS OR PERFORMANCE.
//===============================================================================




#include "transactions/RollTransaction.h"

namespace Transactions
{

RollTransaction::RollTransaction( const molib::mo_name_t id )
	: f_rollId(id)
{
#ifdef DEBUG
printf( "RollTransaction(): id=%s\n", static_cast<molib::moWCString>(static_cast<molib::moName>(id)).c_str() );
#endif
	RollSave();
}


RollTransaction::Roll::Roll( const Combatant::Character::Pointer ch, const int prev_roll, const int curr_roll )
	: f_prevRoll(prev_roll)
	, f_currRoll(curr_roll)
	, f_char(ch)
{
	assert(f_char);
	f_prevPosition    = f_char->getPosition();
	f_prevSubPosition = f_char->getSubPosition();
	f_prevManualPos	  = f_char->getManualPos();
}


void RollTransaction::RollSave()
{
	auto charMgr( GetCharacterMgr().lock() );
	assert(charMgr);

	for( auto ch : charMgr->GetCharacters() )
	{
		assert(ch);
		const int prev_roll = ch->getRoll ( f_rollId );
		const int new_roll  = ch->makeRoll( f_rollId );
		f_rolls.push_back( Roll( ch, prev_roll, new_roll ) );
	}
}


void RollTransaction::doit()
{
	auto statMgr( GetStatMgr().lock() );
	assert(statMgr);

	const molib::mo_name_t initId( statMgr->initId() );
	//
	for( auto& roll : f_rolls )
	{
		Combatant::Character::Pointer ch( roll.f_char );
		assert(ch);
		ch->setRoll( f_rollId, roll.f_currRoll );

		if( f_rollId == initId )
		{
			ch->setPosition    ( 0 );
			ch->setSubPosition ( 0 );
			ch->setManualPos   ( 0 );
		}
		//
		ch->signal_changed().emit();
	}

    GetAppSettings().lock()->Modified( true );
}


void RollTransaction::undo()
{
	auto statMgr( GetStatMgr().lock() );
	assert(statMgr);

	const molib::mo_name_t initId( statMgr->initId() );
	//
	for( auto& roll : f_rolls )
	{
		Combatant::Character::Pointer ch( roll.f_char );
		assert(ch);
		ch->setRoll( f_rollId, roll.f_prevRoll );

		if( f_rollId == initId  )
		{
            ch->setPosition    ( roll.f_prevPosition    );
            ch->setSubPosition ( roll.f_prevSubPosition );
            ch->setManualPos   ( roll.f_prevManualPos   );
		}
		//
		ch->signal_changed().emit();
	}

    GetAppSettings().lock()->Modified( true );
}

}
// namespace Transactions

// vim: ts=8 sw=8

