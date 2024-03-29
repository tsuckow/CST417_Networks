// Driver handling hardware access to the MAC. Provides a standardized
// interface for interacting with ethernet.
//
// == Now some legal ==
//
// The low level initialization code is from:
// *****                                                        *****
// *****  Name: cs8900.c                                        *****
// *****  Ver.: 1.0                                             *****
// *****  Date: 07/05/2001                                      *****
// *****  Auth: Andreas Dannenberg                              *****
// *****        HTWK Leipzig                                    *****
// *****        university of applied sciences                  *****
// *****        Germany                                         *****
// *****  Func: ethernet packet-driver for use with LAN-        *****
// *****        controller CS8900 from Crystal/Cirrus Logic     *****
// *****                                                        *****
// *****  Keil: Module modified for use with Philips            *****
// *****        LPC2378 EMAC Ethernet controller                *****
// *****                                                        *****
//
// The C++ wrapper and all new interface to the hardware is by:
// Date:   2011-11-6 +-
// Author: Thomas Suckow
//         Oregon Institute of Technology
//         United States of America

#pragma once

#include "ethernet_driver.hpp"

class Ethernet_Driver_LPC23xx : public Ethernet_Driver
{
	protected:
		unsigned short *rptr;
		EthernetAddress myAddress;
		
		// Keil: function added to write PHY
		void write_PHY (int PhyReg, int Value)
		{
		  unsigned int tout;
		
		  MAC_MADR = DP83848C_DEF_ADR | PhyReg;
		  MAC_MWTD = Value;
		
		  /* Wait utill operation completed */
		  tout = 0;
		  for (tout = 0; tout < MII_WR_TOUT; tout++) {
		    if ((MAC_MIND & MIND_BUSY) == 0) {
		      break;
		    }
		  }
		}
		
		
		// Keil: function added to read PHY
		unsigned short read_PHY (unsigned char PhyReg) 
		{
		  unsigned int tout;
		
		  MAC_MADR = DP83848C_DEF_ADR | PhyReg;
		  MAC_MCMD = MCMD_READ;
		
		  /* Wait until operation completed */
		  tout = 0;
		  for (tout = 0; tout < MII_RD_TOUT; tout++) {
		    if ((MAC_MIND & MIND_BUSY) == 0) {
		      break;
		    }
		  }
		  MAC_MCMD = 0;
		  return (MAC_MRDD);
		}
		
		
		// Keil: function added to initialize Rx Descriptors
		void rx_descr_init (void)
		{
		  unsigned int i;
		
		  for (i = 0; i < NUM_RX_FRAG; i++) {
		    RX_DESC_PACKET(i)  = RX_BUF(i);
		    RX_DESC_CTRL(i)    = RCTRL_INT | (ETH_FRAG_SIZE-1);
		    RX_STAT_INFO(i)    = 0;
		    RX_STAT_HASHCRC(i) = 0;
		  }
		
		  /* Set EMAC Receive Descriptor Registers. */
		  MAC_RXDESCRIPTOR    = RX_DESC_BASE;
		  MAC_RXSTATUS        = RX_STAT_BASE;
		  MAC_RXDESCRIPTORNUM = NUM_RX_FRAG-1;//Minus 1 Encoding
		
		  /* Rx Descriptors Point to 0 */
		  MAC_RXCONSUMEINDEX  = 0;
		}
		
		
		// Keil: function added to initialize Tx Descriptors
		void tx_descr_init (void) {
		  unsigned int i;
		
		  for (i = 0; i < NUM_TX_FRAG; i++) {
		    TX_DESC_PACKET(i) = TX_BUF(i);
		    TX_DESC_CTRL(i)   = 0;
		    TX_STAT_INFO(i)   = 0;
		  }
		
		  /* Set EMAC Transmit Descriptor Registers. */
		  MAC_TXDESCRIPTOR    = TX_DESC_BASE;
		  MAC_TXSTATUS        = TX_STAT_BASE;
		  MAC_TXDESCRIPTORNUM = NUM_TX_FRAG-1;//Minus 1 Encoding
		
		  /* Tx Descriptors Point to 0 */
		  MAC_TXPRODUCEINDEX  = 0;
		}
		
		
		// configure port-pins for use with LAN-controller,
		// reset it and send the configuration-sequence
		
		void Init_EMAC(void)
		{
		// Keil: function modified to access the EMAC
		// Initializes the EMAC ethernet controller
		  unsigned int regv,tout,id1,id2;
		
		   /* Power Up the EMAC controller. */
		   PCONP |= 0x40000000;
		
		  /* Enable P1 Ethernet Pins. */
		  if (MAC_MODULEID == OLD_EMAC_MODULE_ID) { 
		    /* For the first silicon rev.'-' ID P1.6 should be set. */
		    PINSEL2 = 0x50151105;
		  }
		  else {
		    /* on rev. 'A' and later, P1.6 should NOT be set. */
		    PINSEL2 = 0x50150105;
		  }
		  PINSEL3 = (PINSEL3 & ~0x0000000F) | 0x00000005;
		
		  /* Reset all EMAC internal modules. */
		  MAC_MAC1 = MAC1_RES_TX | MAC1_RES_MCS_TX | MAC1_RES_RX | MAC1_RES_MCS_RX |
		             MAC1_SIM_RES | MAC1_SOFT_RES;
		  MAC_COMMAND = CR_REG_RES | CR_TX_RES | CR_RX_RES;
		
		  /* A short delay after reset. */
		  for (tout = 100; tout; tout--);
		
		  /* Initialize MAC control registers. */
		  MAC_MAC1 = MAC1_PASS_ALL;
		  MAC_MAC2 = MAC2_CRC_EN | MAC2_PAD_EN;
		  MAC_MAXF = ETH_MAX_FLEN;
		  MAC_CLRT = CLRT_DEF;
		  MAC_IPGR = IPGR_DEF;
		
		  /* Enable Reduced MII interface. */
		  MAC_COMMAND = CR_RMII | CR_PASS_RUNT_FRM;
		
		  /* Reset Reduced MII Logic. */
		  MAC_SUPP = SUPP_RES_RMII;
		  for (tout = 100; tout; tout--);
		  MAC_SUPP = 0;
		
		  /* Put the DP83848C in reset mode */
		  write_PHY (PHY_REG_BMCR, 0x8000);
        
        //Allow NIC to power up. (Could get bullshit otherwise)
        //Parameter T2.1.1 http://www.national.com/ds/DP/DP83848C.pdf
        sleep(200);
		
		  /* Wait for hardware reset to end. */
		  for (tout = 0; tout < 0x100000; tout++) {
		    regv = read_PHY (PHY_REG_BMCR);
		    if (!(regv & 0x8000)) {
		      /* Reset complete */
		      break;
		    }
		  }
		
		  /* Check if this is a DP83848C PHY. */
		  id1 = read_PHY (PHY_REG_IDR1);
		  id2 = read_PHY (PHY_REG_IDR2);
		  if (((id1 << 16) | (id2 & 0xFFF0)) == DP83848C_ID) {
		    /* Configure the PHY device */
		
		    /* Use autonegotiation about the link speed. */
		    write_PHY (PHY_REG_BMCR, PHY_AUTO_NEG);
		    /* Wait to complete Auto_Negotiation. */
		    for (tout = 0; tout < 0x100000; tout++) {
		      regv = read_PHY (PHY_REG_BMSR);
		      if (regv & 0x0020) {
		        /* Autonegotiation Complete. */
		        break;
		      }
		    }
		  }
		
		  /* Check the link status. */
		  for (tout = 0; tout < 0x10000; tout++) {
		    regv = read_PHY (PHY_REG_STS);
		    if (regv & 0x0001) {
		      /* Link is on. */
		      break;
		    }
		  }
		
		  /* Configure Full/Half Duplex mode. */
		  if (regv & 0x0004) {
		    /* Full duplex is enabled. */
		    MAC_MAC2    |= MAC2_FULL_DUP;
		    MAC_COMMAND |= CR_FULL_DUP;
		    MAC_IPGT     = IPGT_FULL_DUP;
		  }
		  else {
		    /* Half duplex mode. */
		    MAC_IPGT = IPGT_HALF_DUP;
		  }
		
		  /* Configure 100MBit/10MBit mode. */
		  if (regv & 0x0002) {
		    /* 10MBit mode. */
		    MAC_SUPP = 0;
		  }
		  else {
		    /* 100MBit mode. */
		    MAC_SUPP = SUPP_SPEED;
		  }
		
		  /* Set the Ethernet MAC Address registers */
        uint8_t * addr = myAddress.getRaw();
		  MAC_SA0 = (addr[5] << 8) | addr[4];
		  MAC_SA1 = (addr[3] << 8) | addr[2];
		  MAC_SA2 = (addr[1] << 8) | addr[0];
		
		  /* Initialize Tx and Rx DMA Descriptors */
		  rx_descr_init ();
		  tx_descr_init ();
		
		  /* Receive Broadcast and Perfect Match Packets */
		  MAC_RXFILTERCTRL = RFC_BCAST_EN | RFC_MCAST_EN | RFC_PERFECT_EN;
		
		  /* Enable EMAC interrupts. */
		  MAC_INTENABLE = 0xFF;//INT_RX_DONE | INT_TX_DONE | INT_RX_OVERRUN;
		
		  /* Reset all interrupts */
		  MAC_INTCLEAR  = 0xFFFF;
		
		  /* Enable receive and transmit mode of MAC Ethernet core */
		  MAC_COMMAND  |= (CR_RX_EN | CR_TX_EN);
		  MAC_MAC1     |= MAC1_REC_EN;
		}

	public:
		Ethernet_Driver_LPC23xx( EthernetAddress eaddr ) : myAddress( eaddr )
		{
		}
		
		virtual ~Ethernet_Driver_LPC23xx()
		{
		}
	
		virtual bool init()
		{
			Init_EMAC();
			return true;
		}
	
		virtual void install( void (*handler)(void) )
		{
			//Interrupt
			VICVectAddr21  = (unsigned long)handler;
			VICVectPriority21  = 15;
			VICIntEnable  = (1  << 21);
		}
		
		virtual void irq_handler()
		{
			if( MAC_INTSTATUS )
			{
				if( MAC_INTSTATUS & INT_RX_OVERRUN )
				{
					MAC_INTCLEAR = INT_RX_OVERRUN;
					printf( "ETH ERR: RX Overrun\n" );
				}
				
				if( MAC_INTSTATUS & INT_RX_ERR )
				{
					MAC_INTCLEAR = INT_RX_ERR;
					//This is normal because of packets with type field
					//printf( "ETH ERR: RX Error\n" );
				}
				
				if( MAC_INTSTATUS & INT_RX_FIN ) //RX Finished Process Descriptors
				{
					MAC_INTCLEAR = INT_RX_FIN;
					//Printing doesn't help the cause.
					//printf( "ETH: RX FIN, About to drop packet.\n" );
				}
				
				if( MAC_INTSTATUS & INT_RX_DONE )
				{
					MAC_INTCLEAR = INT_RX_DONE;
					
					//Inform the thread to wake.
					sem_recv.signal();
				}
				
				if( MAC_INTSTATUS & INT_TX_UNDERRUN )
				{
					MAC_INTCLEAR = INT_TX_UNDERRUN;
					printf( "ETH ERR: TX Underrun\n" );
				}
				
				if( MAC_INTSTATUS & INT_TX_ERR )
				{
					MAC_INTCLEAR = INT_TX_ERR;
					printf( "ETH ERR: TX Error\n" );
				}
				
				//TX Finished Process Descriptors
				if( MAC_INTSTATUS & INT_TX_FIN )
				{
					MAC_INTCLEAR = INT_TX_FIN;
					//printf( "ETH: TX FIN\n" );
				}
				
				if( MAC_INTSTATUS & INT_TX_DONE )
				{
					MAC_INTCLEAR = INT_TX_DONE;
               sem_tx.signal();
					//printf( "ETH: TX DONE\n" );
				}
				
				if( MAC_INTSTATUS & INT_SOFT_INT )
				{
					MAC_INTCLEAR = INT_SOFT_INT;
					printf( "ETH: SOFT INT\n" );
				}
				
				if( MAC_INTSTATUS & INT_WAKEUP )
				{
					MAC_INTCLEAR = INT_WAKEUP;
					printf( "ETH: WAKE\n" );
				}
			}
		}
		
		virtual bool isFrameAvailable()
		{
			//The docs are pretty sketchy about this, they say the pointers can be equal and it is full but
			//that appears to be false (which is more useful).
			return (MAC_RXPRODUCEINDEX != MAC_RXCONSUMEINDEX);     // more packets received ?
		}
		
		// Reads the length of the received ethernet frame and checks if the 
		// destination address is a broadcast message or not
		// returns the frame length
		virtual unsigned short beginReadFrame(void)
		{
		  unsigned short RxLen;
		  unsigned int idx;
		
		  idx = MAC_RXCONSUMEINDEX;
		  RxLen = (RX_STAT_INFO(idx) & RINFO_SIZE) + 1;
		  rptr = (unsigned short *)RX_DESC_PACKET(idx);
		  return(RxLen);
		}
		
		virtual uint8_t * readFramePointer()
		{
			return reinterpret_cast<uint8_t *>( rptr ); 
		}
		
		virtual void endReadFrame(void)
		{
		  unsigned int idx;
		
		  /* DMA free packet. */
		  idx = MAC_RXCONSUMEINDEX;
		  if (++idx == NUM_RX_FRAG) idx = 0;
		  MAC_RXCONSUMEINDEX = idx;
		}
		
		virtual bool isTXFull()
		{
			//So tempting to use %. Bad, bad tom. Division slow.
			return ((MAC_TXPRODUCEINDEX + 1) % NUM_TX_FRAG) == MAC_TXCONSUMEINDEX;
		}
		
		//! \warning: Not Reentrant
		virtual void sendFrame( uint8_t const * frame, uint_fast16_t size )
		{
			//TODO: Could be optimized to fill last buffer then wait to increment produce index.
			while( isTXFull() )
			{
				sem_tx.wait();
			}
         
			{
				unsigned int idx;
				idx = MAC_TXPRODUCEINDEX;
		  		TX_DESC_CTRL(idx) = (size-1/*-1 Encoded*/-4/*CRC*/) | TCTRL_CRC | TCTRL_LAST | TCTRL_INT;
			
				memcpy((void*)TX_DESC_PACKET(idx), frame, size);
				
			  	if (++idx == NUM_TX_FRAG) idx = 0;
			  	MAC_TXPRODUCEINDEX = idx;
			}
		}
      
      //! \brief Returns the ethernet address of this NIC
      virtual EthernetAddress getAddress()
		{
         return myAddress;
		}
};
