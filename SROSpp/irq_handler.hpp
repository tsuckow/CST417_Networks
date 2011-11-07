#pragma once

#include "ll.hpp"
#include "driver.hpp"

class irq_handler
{
	protected:
		typedef ll<Driver *> dlist;
		dlist drivers;
	
	public:
		void process()
		{
      
			dlist::iterator begin = drivers.begin();
			dlist::iterator end   = drivers.end();
			
			for(; begin != end; ++begin)
			{
				(*begin)->irq_handler();
			}
		}
		
		void add( Driver * driver )
		{
			drivers.push_back( driver );
		}
};
