//Internet Protocol Structures

#pragma once

static uint8_t const IP_UNCONFIGURED[4] = {0x00,0x00,0x00,0x00};

class IPAddress
{
protected:

	static size_t const SIZE = 4;
	uint8_t addr[SIZE];

public:
	IPAddress()
	{
	}

	IPAddress( uint8_t const * addr_ )
	{
		memcpy( addr, addr_, SIZE );
	}

	IPAddress( IPAddress const & addr_ )
	{
		memcpy( addr, addr_.addr, SIZE );
	}

	bool operator==( IPAddress const & rhs ) const
	{
		return
			addr[0] == rhs.addr[0] &&
			addr[1] == rhs.addr[1] &&
			addr[2] == rhs.addr[2] &&
			addr[3] == rhs.addr[3]
			;
	}

	void print()
	{
		printf("%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
	}

   void store( uint8_t * buffer )
   {
      memcpy( buffer, addr, SIZE );
   }
};
