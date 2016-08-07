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




#pragma once

// STL
//
#include <iostream>
#include <string>
#include <memory>

// LOCAL
//
#include "common.h"
#include "stat.h"

// molib
//
#include "mo/mo_props.h"
#include "mo/mo_string.h"

namespace Attribute
{

class StatManager
{
public:
	typedef std::shared_ptr<StatManager>	Pointer;

	StatManager();

	molib::mo_name_t initId  () const	{ return f_initId;   }
	molib::mo_name_t hpId    () const	{ return f_hpId;     }
	molib::mo_name_t strId   () const	{ return f_strId;    }
	molib::mo_name_t dexId   () const	{ return f_dexId;    }
	molib::mo_name_t conId   () const	{ return f_conId;    }
	molib::mo_name_t intId   () const	{ return f_intId;    }
	molib::mo_name_t wisId   () const	{ return f_wisId;    }
	molib::mo_name_t chaId   () const	{ return f_chaId;    }
	molib::mo_name_t spotId  () const	{ return f_spotId;   }
	molib::mo_name_t listenId() const	{ return f_listenId; }
	molib::mo_name_t willId  () const	{ return f_willId;   }
	molib::mo_name_t levelId () const	{ return f_levelId;  }

	void			AddStat( const molib::moWCString& name, const molib::mo_name_t id );
	void			AddStat( Stat::Pointer stat );
	Stat::Pointer	GetStat( const molib::mo_name_t id );
	void			DeleteStat( const molib::mo_name_t id );

	Stat::Pointer	Strength();
	Stat::Pointer	Dexterity();
	Stat::Pointer	Constitution();
	Stat::Pointer	Intelligence();
	Stat::Pointer	Wisdom();
	Stat::Pointer	Charisma();

	typedef std::map<molib::mo_name_t, Stat::Pointer>	 	StatMap;
	const StatMap&	GetStats();

	Stat::Pointer	GetLegacyStat( const int id, const int type );

	typedef sigc::signal<void>		VoidSignal;
	VoidSignal	signal_changed()	{ return f_statsChanged; }	// One or more stats added/deleted/changed

	void		AddDefaultColumns();

	void		Load( molib::moPropBagRef& propBag );
	void		Save( molib::moPropBagRef& propBag );

private:
	StatMap				f_statMap;

	// Fixed ids, because they are special
	//
	molib::moName		f_initId;
	molib::moName		f_hpId;
	//
	// Ability IDs
	//
	molib::moName		f_strId;
	molib::moName		f_dexId;
	molib::moName		f_conId;
	molib::moName		f_intId;
	molib::moName		f_wisId;
	molib::moName		f_chaId;
	//
	// Legacy IDs
	//
	molib::moName		f_spotId;
	molib::moName		f_listenId;
	molib::moName		f_willId;
	//
	// New IDs
	//
	molib::moName		f_levelId;

	// Array in which to keep all stats
	//
	molib::moName		f_arrayName;

	// A signal to notify interested objects for changes
	// TODO: This may need to be redesigned
	//
	VoidSignal			f_statsChanged;

	// Forbidden due to singleton pattern
	//
	StatManager( const StatManager& );
	void operator =( StatManager& );

	// Create default stats if none existed before
	//
	void CreateDefaultStats();

	// Private methods
	//
	void AddDefaultStat(	const molib::moWCString& name,
			const molib::mo_name_t id,
			const int order,
			const bool on_UI = true,
			const bool internal_stat = false,
			const bool ability_stat = false,
			const char* accel = NULL,
			const molib::mo_name_t ability_id = molib::moName("NONE") );

	void OnStatChanged();	// For when an "owned" Stat changed
};

}
// namespace Attribute


// vim: ts=4 sw=4 syntax=cpp.doxygen

