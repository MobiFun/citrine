/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : TDC
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 

#ifdef TDC_H
#error "TDC.H already included, TDC.H must come after all primitives and messages"
#endif

#ifndef TDC_MSG_H
#define TDC_MSG_H

#if DOT_COMPLETE_DEFINES

//----------------------------------------------------------------------------
// macros to repeat stuff that is similar in many interface classes 
//----------------------------------------------------------------------------

#define M_TDC_INTERFACE_PRIM_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_PSTRUCT_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_PUNION_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_SDU_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_PENUM_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_PINT_ADDITIONAL(INT_NAME, SHORT_NAME)

//-----

#define M_TDC_INTERFACE_MSG_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_MSTRUCT_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_MUNION_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_MENUM_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_MINT_ADDITIONAL(INT_NAME, SHORT_NAME)

//-----

#define M_TDC_INTERFACE_INT_ADDITIONAL(SHORT_NAME)

#define M_TDC_INTERFACE_XXX_PRIMITIVE_UNION_ADDITIONAL(SAP)

#define M_TDC_INTERFACE_XXX_MESSAGE_UNION_ADDITIONAL(MSG)

#define M_TDC_CREATE_DEFAULT_PRIMITIVE_INTERFACE_CLASS(SAP)

#define M_TDC_INTERFACE_PRIMITIVE_ADDITIONAL()

#define M_TDC_INTERFACE_PRIMITIVE_ELEMENT_ADDITIONAL(SAP)

#define M_TDC_CREATE_DEFAULT_MESSAGE_INTERFACE_CLASS(MSG)

#define M_TDC_INTERFACE_MESSAGE_ADDITIONAL()

#define M_TDC_INTERFACE_MESSAGE_ELEMENT_ADDITIONAL(MSG)

#else
#endif

#include "tdc_base.h"

#ifndef TDC_DESCRIPTOR
#include "message.h"
#include "primitive.h"
#else
#include "message_dsc.h"
#include "primitive_dsc.h"
#endif

struct T_TDC_INTERFACE_U8;

//TODO: eliminate need for these types used by M_RR (this is a bug work around)
typedef T_TDC_HANDLE_U8 T_TDC_HANDLE_AVG_W;
typedef T_TDC_HANDLE_U8 T_TDC_HANDLE_AVG_T;
typedef T_TDC_INTERFACE_U8 T_TDC_INTERFACE_AVG_W;
typedef T_TDC_INTERFACE_U8 T_TDC_INTERFACE_AVG_T;
//TODO: eliminate need for these types used by M_GRR (this is a bug work around)
typedef T_TDC_HANDLE_U8 T_TDC_HANDLE_RESEL;
typedef T_TDC_INTERFACE_U8 T_TDC_INTERFACE_RESEL;

typedef struct T_TDC_HANDLE_TEST_var4 T_TDC_HANDLE_TEST_var4T;
typedef struct T_TDC_HANDLE_TEST_enum4 T_TDC_HANDLE_TEST_enum4T;

M_TDC_FORWARD_SDU (sdu)
struct T_TDC_DESCRIPTOR_sdu;
struct T_TDC_HANDLE_sdu;
struct T_TDC_INTERFACE_sdu;
struct T_sdu;
typedef T_TDC_HANDLE_sdu T_TDC_HANDLE_SDU;
typedef T_TDC_DESCRIPTOR_sdu T_TDC_DESCRIPTOR_SDU;
typedef T_TDC_INTERFACE_sdu T_TDC_INTERFACE_SDU;
typedef T_sdu T_TDC_INSTANCE_SDU;



///\todo make some sencible stuff here
struct T_TDC_HANDLE_desc_list
{
  int dummy_to_keep_doxygen_happy;
};

///\todo make some sencible stuff here
struct T_TDC_HANDLE_desc
{
  int dummy_to_keep_doxygen_happy;
};

///\todo make some sencible stuff here
struct T_TDC_INTERFACE_desc_list
{
  int dummy_to_keep_doxygen_happy;
};

///\todo make some sencible stuff here
struct T_TDC_INTERFACE_desc
{
  int dummy_to_keep_doxygen_happy;
};



#ifdef TDC_DESCRIPTOR

/*M_TDC_FORWARD_PSTRUCT(raw)
M_TDC_FORWARD_PSTRUCT(aim)
M_TDC_FORWARD_PSTRUCT(sdu)*/

struct T_TDC_DESCRIPTOR_aim:T_TDC_DESCRIPTOR_PSTRUCT_BASE
{
	M_TDC_DESCRIPTOR_PSTRUCT_ADDITIONAL (aim)
	T_TDC_HANDLE_U8 ti; 
	T_TDC_HANDLE_U8 tie;
  T_TDC_HANDLE_U8 nsd;
	T_TDC_HANDLE_MESSAGE_UNION entity;
};

struct T_TDC_DESCRIPTOR_sdu:T_TDC_DESCRIPTOR_SDU_BASE
{
	M_TDC_DESCRIPTOR_SDU_ADDITIONAL (sdu)
	T_TDC_HANDLE_raw raw; //manual sdu
	T_TDC_HANDLE_aim aim; //coded sdu
};

#endif

#if !defined TDC_DESCRIPTOR || defined TDC_PRECOMPILE 

struct T_aim:T_TDC_INSTANCE_PSTRUCT_BASE
{
  M_TDC_INSTANCE_PSTRUCT_ADDITIONAL (aim,aim)
  T_TDC_INTERFACE_aim* operator-> ();
};
struct T_TDC_INTERFACE_aim:T_TDC_INTERFACE_PSTRUCT_BASE
{
  M_TDC_INTERFACE_PSTRUCT_ADDITIONAL (aim,aim)
  T_TDC_INTERFACE_U8 ti;
  T_TDC_INTERFACE_U8 tie;
  T_TDC_INTERFACE_U8 nsd;
  T_TDC_INTERFACE_MESSAGE_UNION entity;
#ifdef TDC_TYPE_NAME_COMPLETE
  struct { char T_aim, ___ti___tie___nsd___entiy; } _type_name ();
#endif
};

struct T_sdu:T_TDC_INSTANCE_SDU_BASE
{
  M_TDC_INSTANCE_SDU_ADDITIONAL (sdu)
  T_sdu(const T_MESSAGE_UNION& msg);
  T_sdu(const T_TDC_INSTANCE_MSG_BASE& msg);
  T_TDC_INTERFACE_sdu* operator-> ();
};
struct T_TDC_INTERFACE_sdu:T_TDC_INTERFACE_SDU_BASE
{
  M_TDC_INTERFACE_PSTRUCT_ADDITIONAL (sdu,sdu)
  T_TDC_INTERFACE_raw raw; //manual sdu
  T_TDC_INTERFACE_aim aim; //coded sdu
#ifdef TDC_TYPE_NAME_COMPLETE
  struct { char T_sdu, ___raw___aim; } _type_name ();
#endif
};
inline T_sdu::T_sdu(const T_MESSAGE_UNION& msg)
{
  construct();
  (*this)->aim.entity=msg;
}
inline T_sdu::T_sdu(const T_TDC_INSTANCE_MSG_BASE& msg)
{
  construct();
  (*this)->aim.entity=msg;
}


#endif //!defined TDC_DESCRIPTOR || defined TDC_PRECOMPILE 

//============================================================================
template <class T>
T_sdu tds_array_to_sdu(const T& U8array)
//This function is a temporary solution to help the convertion from TDS U8 arrays to TDC SDU's.
{
	T_sdu sdu; 
	sdu->raw.l_buf = U8array[0] + U8array[1] * 256;
	sdu->raw.o_buf = U8array[2] + U8array[3] * 256;
	if (sizeof(U8array)>4)
	{
		for (int i = 4; i < sizeof(U8array);i++)
		{
			sdu->raw.buf[i-4] = U8array[i];
		}
	}
	else
	{
		sdu->raw.buf = T_ARRAY<U8>();
	}
	return sdu;
}
//============================================================================
template <class T>
T_sdu BIN(const T& U8array)
{
	T_sdu sdu;
	int i=0,bit=0,l_buf=0;
	U8 temp=0;

	while(U8array[i] != '\0')
	{
		if(U8array[i] == 0x20)				//(0x20 is space) 
		{
			if(bit != 0)
				tdc_user_error("BIN(): There is not 8 bits between spaces!");	
		}	
		else
		{
			switch(U8array[i])
			{
				case 0x31:  //(0x31 ascii code for 1)
					temp += 1<<(7-bit);
					break;	

				case 0x30:	//(0x30 ascii code for 0)
					break;	

				default:
					tdc_user_error("BIN(): string character neither '0' or '1': %c",U8array[i]);
					break;
			}
			bit ++;
			if(bit == 8)
			{
				sdu->raw.buf[l_buf/8] = temp;
				temp = 0;
				bit=0;
			}
			l_buf++;
		}
		i++;
	}
	if(bit != 0)		//if the number of bits is not a multiplum of 8 the last few bits will be assigned here
	{
		sdu->raw.buf[l_buf/8] = temp;
	}

	sdu->raw.l_buf=l_buf;
	sdu->raw.o_buf=0;
	return sdu;
}

template <class T>
T_sdu HEX(const T& U8array)
{
  T_sdu sdu;
  int i = 0, nibble = 0, l_buf = 0;
  U8 temp = 0;
  
  while(U8array[i] != '\0')
    {
      if(U8array[i] == 0x20)				//(0x20 is space) 
	{
	  if(nibble != 0)
	    tdc_user_error("HEX(): There is not 2 nibbles between spaces!");	
	}	
      else
	{
	  if(('0' <= U8array[i]) &&  (U8array[i] <= '9'))
	    {
	      temp += (U8array[i] - 0x30) << (4*(1-nibble));
	    }
	  else if(('A' <= U8array[i]) &&  (U8array[i] <= 'F'))
	    {
	      temp += (U8array[i] - 0x37) << (4*(1-nibble));
	    }
	  else if(('a' <= U8array[i]) && (U8array[i] <= 'f'))
	    {
	      temp += (U8array[i] - 0x57) << (4*(1-nibble));
	    }
	  else
	    {
	      tdc_user_error("HEX(): String character not a HEX number: %c",U8array[i]);
	    }
	  nibble ++;
	  if(nibble == 2)
	    {
	      sdu->raw.buf[l_buf/8] = temp;
	      temp = 0;
	      nibble = 0;
	    }
	  l_buf += 4;
	}
      i++;
    }	
  
  if(nibble != 0) //checks whether the number of nibbles are odd. IF THIS SHOULD BE MADE LEGAL 
    {				//remove the '//' in front of the line above and comment out the error function call!			
      tdc_user_error("HEX(): Odd number of nibbles (hex numbers)");
      //sdu->raw.buf[l_buf/8]=temp;
    }
  sdu->raw.l_buf = l_buf;
  sdu->raw.o_buf = 0;
  return sdu;
}

template <class T>
T_sdu CHR(const T& U8array)
{
	T_sdu sdu;
	int i=0;
	while(U8array[i] != '\0')	
	{
		sdu->raw.buf[i]= (U8)U8array[i];
		i++;
	}
	sdu->raw.l_buf=8*i;
	sdu->raw.o_buf=0;
	return sdu;
}

#endif //TDC_MSG_H
