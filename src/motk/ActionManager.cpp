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

// LOCAL
#include "Common.h"
#include "ActionManager.h"

// STL
#include <string>

// MOLIB
#include "mo/mo_application.h"

// GTK/GDK MM
#include <gtkmm/accelkey.h>
#include <gdkmm/types.h>

using namespace molib;

namespace motk
{

ActionManager::ActionManager( Gtk::BuiltinIconSize defaultIconSize ) :
	f_defaultIconSize(defaultIconSize)
{
	CreateActionGroup();
	f_iconFactory	 = Gtk::IconFactory::create();
	f_iconFactory->add_default();
}


ActionManager::~ActionManager()
{
	f_iconFactory->remove_default();
}


void ActionManager::SetDefaultIconSize( Gtk::BuiltinIconSize defaultIconSize )
{
	f_defaultIconSize = defaultIconSize;
}


Glib::ustring	ActionManager::GetImagePath( const moWCString& imageName )
{
	molib::moApplicationSPtr app = molib::moApplication::Instance();
	const molib::moWCString app_path = app->GetApplicationPath();
	molib::moWCString fullPath = app_path.FilenameChild("images").FilenameChild(imageName.c_str());
	const char* s = fullPath.MBData();
	Glib::ustring fp(s);
	delete s;
	return fp;
}


PixbufPtr ActionManager::GetPixbufImage( const char* const* xpm_data )
{
	PixbufPtr pixbuf = Gdk::Pixbuf::create_from_xpm_data( xpm_data );
	return pixbuf;
}

PixbufPtr ActionManager::GetPixbufImage( const moWCString& imageName )
{
	Glib::ustring path = GetImagePath( imageName );
	PixbufPtr pixbuf = Gdk::Pixbuf::create_from_file( path );
	return pixbuf;
}


Gtk::IconSource	ActionManager::CreateIconSource( const ButtonImage* image )
{
	Gtk::IconSource	icon_source;
	try
	{
		switch( image->f_type )
		{
			case ButtonImage::XPM:
				icon_source.set_pixbuf( GetPixbufImage( image->f_xpmData ) );
				break;

			case ButtonImage::FILENAME:
				icon_source.set_pixbuf( GetPixbufImage( image->f_imageName ) );
				break;

			case ButtonImage::NONE:
				// Do nothing
				break;
		}
	}
	catch( const Glib::Exception& x )
	{
#ifdef DEBUG
		std::cerr << x.what() << std::endl;
#endif
		throw;
	}
	return icon_source;
}

void ActionManager::AddIconSource( const Gtk::StockID& stock_id, Gtk::IconSource icon_source )
{
	icon_source.set_size( f_defaultIconSize );
	icon_source.set_size_wildcarded();
	Gtk::IconSet icon_set;
	icon_set.add_source( icon_source );
	f_iconFactory->add( stock_id, icon_set );
}


void ActionManager::AddStockItem( const Glib::ustring& id, const Glib::ustring& label, const ButtonImage* image )
{
	const Gtk::StockID stock_id( id );
	Gtk::StockItem	item( stock_id, label );
	//
	if( !Gtk::Stock::lookup( stock_id, item ) )
	{
		if( image )
		{
			AddIconSource( stock_id, CreateIconSource( image ) );
		}

		Gtk::Stock::add( item );
	}
}


void ActionManager::AddActionToGroup( ActionPtr action, const Glib::ustring& event, const Glib::ustring& accel_key )
{
	if( accel_key == "" )
	{
		f_refActionGroup->add( action );
	}
	else if( accel_key == "space" )
	{
		f_refActionGroup->add( action, Gtk::AccelKey( ' ', Gdk::RELEASE_MASK ) );
	}
	else if( accel_key == "up" )
	{
		f_refActionGroup->add( action, Gtk::AccelKey( GDK_Up, Gdk::RELEASE_MASK ) );
	}
	else if( accel_key == "down" )
	{
		f_refActionGroup->add( action, Gtk::AccelKey( GDK_Down, Gdk::RELEASE_MASK ) );
	}
	else
	{
		f_refActionGroup->add( action, Gtk::AccelKey( accel_key ) );
	}

	f_actionSignals[event].f_action = action;
		//action->signal_activate().connect( sigc::bind( sigc::mem_fun(*this, &ActionManager::OnButtonClicked), action, event ));
	ActivateSignal( event, true );
}


void ActionManager::ActivateSignal( const Glib::ustring& event, const bool activate )
{
	ActionSignal& as = f_actionSignals[event];
	if( activate )
	{
		as.f_connection = as.f_action->signal_activate().connect(
				sigc::bind( sigc::mem_fun(*this, &ActionManager::OnButtonClicked), as.f_action, event ) );
	}
	else
	{
		as.f_connection.disconnect();
	}
}

void ActionManager::AddRadioAction(
		const Glib::ustring& name,
		const Glib::ustring& event,
		const ButtonImage* image,
		const Glib::ustring& accel_key )
{
	if( !f_actionMap[event] )
	{
		ActionPtr newAction;
		
		if( image == 0 )
		{
			newAction = Gtk::RadioAction::create(
				f_radioGroup,
				event,
				name );
		}
		else
		{
			newAction = Gtk::RadioAction::create(
				f_radioGroup,
				event,
				Gtk::StockID(event),
				name );
	
			AddStockItem( event, name, image );
		}

		AddActionToGroup( newAction, event, accel_key );

		// Flag as entered
		f_actionMap[event] = true;
	}
}


void ActionManager::AddToggleAction(
		const Glib::ustring& name,
		const Glib::ustring& event,
		const ButtonImage* image,
		const Glib::ustring& accel_key )
{
	if( !f_actionMap[event] )
	{
		ActionPtr newAction;

		if( image == 0 )
		{
			newAction = Gtk::ToggleAction::create(
					event,
			name );
		}
		else
		{
			newAction = Gtk::ToggleAction::create(
					event,
			Gtk::StockID(event),
			name );
	
			AddStockItem( event, name, image );
		}

		AddActionToGroup( newAction, event, accel_key );

		// Flag as entered
		f_actionMap[event] = true;
	}
}


namespace
{
	Glib::ustring MakeTooltip( const Glib::ustring& name, const Glib::ustring& accel_key )
	{
		Glib::ustring retstr1 = name + " (" + accel_key + ")";
		Glib::ustring retstr2;
		const int len = retstr1.size();
		for( int idx = 0; idx < len; ++idx )
		{
			if( retstr1[idx] == '_' )
			{
				continue;
			}
			retstr2 += retstr1[idx];
		}

		return retstr2;
	}
}


void ActionManager::AddAction(
		const Glib::ustring& name,
		const Glib::ustring& event,
		const ButtonImage* image,
		const Glib::ustring& accel_key )
{
	if( !f_actionMap[event] )
	{
		ActionPtr newAction;

		if( image == 0 )
		{
			newAction = Gtk::Action::create(
				event,
				name );
		}
		else
		{
			newAction = Gtk::Action::create(
				event,
				Gtk::StockID(event),
				name );
	
			AddStockItem( event, name, image );
		}

		// Add tooltip to action
		//
		newAction->set_tooltip( MakeTooltip( name, accel_key ) );

		// Add to group
		//
		AddActionToGroup( newAction, event, accel_key );

		// Flag as entered
		f_actionMap[event] = true;
	}
}

void ActionManager::AddAction(
		const Glib::ustring& name,
		const Glib::ustring& event,
		const Gtk::StockID& stock_id,
		const Glib::ustring& accel_key )
{
	if( !f_actionMap[event] )
	{
		ActionPtr newAction = Gtk::Action::create(
					event,
					stock_id,
					name );

		newAction->set_tooltip( MakeTooltip( name, accel_key ) );
		AddActionToGroup( newAction, event, accel_key );

		// Flag as entered
		f_actionMap[event] = true;
	}
}


void ActionManager::AddMenu(
		const Glib::ustring& name,
		const Glib::ustring& event )
{
	if( !f_actionMap[event] )
	{
		ActionPtr newAction = Gtk::Action::create(
					event,
					name );

		f_refActionGroup->add( newAction );

		// Flag as entered
		f_actionSignals[event].f_action = newAction;
		f_actionMap[event] = true;
	}
}


motk::ActionPtr ActionManager::GetAction( const molib::moWCString& name )
{
	return f_refActionGroup->get_action( name.c_str() );
}


/// \brief Remove and action by event name
//
// Search through the list of actions and remove it, including its events.
//
/// \sa RemoveAction
//
void ActionManager::RemoveAction( const Glib::ustring& event )
{
	ActionSignal& as = f_actionSignals[event];
	as.f_connection.disconnect();
	f_refActionGroup->remove( as.f_action );
	as.f_action.clear();
	f_actionMap[event] = false;
	f_actionSignals.erase( f_actionSignals.find( event ) );
}


/// \brief Remove an action by pointer
//
// Removes the action from the local list and the GTKMM action list
//
/// \sa RemoveAction
//
void ActionManager::RemoveAction( ActionPtr action )
{
	RemoveAction( action->get_name() );
}


void ActionManager::CreateActionGroup()
{
	f_refActionGroup = Gtk::ActionGroup::create();
}

/// \brief Remove all actions from the action group
//
/// \sa RemoveAction
//
void ActionManager::RemoveActions()
{
#if 0
	String2Signal::iterator			iter = f_actionSignals.begin();
	const String2Signal::iterator	end  = f_actionSignals.end();
	//
	for( ; iter != end; ++iter )
	{
		RemoveAction( iter->second.f_action );
	}
#endif

	f_actionMap.clear();
	f_actionSignals.clear();
	f_refActionGroup.clear();
}


}
// namespace motk


// vim: ts=4 sw=4 syntax=cpp.doxygen

