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

#if defined TDC_PRECOMPILE && !defined TDC_BASE_TDC_H
#pragma message("TDC_BASE.H ENTERING PRECOMPILE MODE")
/*
  This code is added to make a prettier and easier to navigate .i-file
  Purpose:
    A)  maintain info of which macro codes come from
    B)  maintain line breaks from inside macros

 	The process include the following steps:

  1:
    Command:
      perl tdc_h.pl ..\..\..\..\INC\tdc_base.h >tdc_base.tdc_h
      (add TDC_BEGIN_MACRO TDC_NL TDC_MACRO_END TDC_NL macros)

    Example transformation:
      Input: (tdc_base.h) 
*/
        #define MULTI_LINE_MACRO(A) ... \
          ... \
          ...
        #define SINGLE_LINE_MACRO(A) ...
/*
      Output: (tdc_base.tdc_h)
        #define MULTI_LINE_MACRO(A) TDC_BEGIN_MACRO(MULTI_LINE_MACRO(A)) ... TDC_NL\
          ... TDC_NL\
          ... TDC_MACRO_END()
        #define SINGLE_LINE_MACRO(A) TDC_MACRO(SINGLE_LINE_MACRO(A)) ...
  2:
    Command:
      EDI preprocess .cpp-file with TDC and TDC_PRECOMPILE defined
      (as a consequence tdc_base.tdc_h is used instead of tdc_base.h)

    Example transformation:
      Input: (use_some_macro.cpp)
        MULTI_LINE_MACRO(B)
      Output: (use_some_macro.i)
        TDC_BEGIN_MACRO("MULTI_LINE_MACRO(B)") ... TDC_NL ... TDC_NL ... TDC_MACRO_END()

  3:
    Command:
      perl tdc_i.pl .\tdc.i >tdc.tdc_i
      (replace TDC_NL with real newline, make pretty indention

    Example transformation:
      Input: (use_some_macro.i)
        TDC_BEGIN_MACRO("MULTI_LINE_MACRO(B)") ... TDC_NL ... TDC_NL ... TDC_MACRO_END
    Output: (use_some_macro.tdc_i)
      TDC_BEGIN_MACRO("MULTI_LINE_MACRO(B)") ... 
        ... 
        ... TDC_MACRO_END()

 */
#define TDC_BASE_TDC_H
#include "tdc_base.tdc_h"
#endif //defined TDC_PRECOMPILE && !defined TDC_BASE_TDC_H

#ifndef TDC_BASE_H // stadard h-file guard
#define TDC_BASE_H

//============================================================================
/// \defgroup Configuration
//\{

///memory leak detection
#define noTDC_USE_ALLOC_DEBUG_COUNTER 

#define TDC_DEBUG

#define TDC_NO_PROFILING

///include T_PORT
#define TDC_PORT

#define noM_TDC_MESSAGE_DEMO

///typeinfo not needed, everything we need is in cccdgen tables
#define noTDC_TYPEINFO 

///never defined
#define noDOT_COMPLETE 

//\}

//============================================================================
// include files
//============================================================================

//tdc_test_header_begin
//extern int dummy_to_make_namespace_non_empty;
//} //temporare leave namespace tdc 
#ifndef TDC_TEST_I_CPP
#include <setjmp.h>
#include <stddef.h>
#include <typeinfo.h>
#include <malloc.h>
#include <string.h>
#endif
//namespace tdc { // reenter namespace tdc 

//============================================================================
// controling warning levels from compilers
//============================================================================

#pragma warning(disable:4786)
//tdc_test_header_end

/*
  general info members are constructed etc by generic functions so these are normal ok when
  arriving inside M_TDC_... :
    Warning 1539: member 'T_raw::handle' (line 3242, file M:\gpf\inc\tdc_base.h, module M:\gpf\util\tap\tdc\src\tdc_lib_main.cpp) not assigned by assignment operator
    Warning 1401: member 'T_raw::handle' (line 3242, file M:\gpf\inc\tdc_base.h, module M:\gpf\util\tap\tdc\src\tdc_lib_main.cpp) not initialized by constructor
 */

/*lint -e1717*/ //Info 1717: empty prototype for function declaration, assumed '(void)'
/*lint -e1505*/ //TODO: add access specifier for all base classes
/*lint -DLINT*/
/*lint -esym(18,BUF_*)*/ //CCDGEN generates surplus typedefs for _dsc.h-files
/*#lint -libdir(M:\gpf\util\teststack\inc\tdcinc)*/
/*#lint -libh(M:\gpf\util\teststack\inc\tdcinc\p_xx_tdc_1.h)*/

/// this lint option is used inside a macro, VA can not handle comments in macros correctly
#define TDC_LINT_UNUSED_MEMBER /*lint -e{754}*/
#define TDC_LINT_UNASSIGNED_MEMBER /*lint -e{1539}*/
#define TDC_LINT_UNCOPIED_BASE /*lint -e{1538}*/
#define TDC_LINT_NO_SELFASSIGN_TEST /*lint -e{1529}*/
#define TDC_LINT_UNINITIALIZED_MEMBER /*lint -e{1401}*/
#define TDC_LINT_POSSIBLE_UNHANDLE_POINTER_MEMBER /*lint -e{1740}*/

//============================================================================
/// \defgroup Constants
//============================================================================
//\{

#define FALSE 0
#define TRUE  1

/// TDSConstant comming from macros.h -> used in cc.doc
#define NOT_PRESENT_8BIT 0xff

/// TDSConstant comming from macros.h -> used in cc.doc
#define NOT_PRESENT_16BIT 0xffff

/// TDSConstant comming from macros.h -> used in cc.doc
#define NOT_PRESENT_32BIT 0xffffffffL

//\}

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_0
{
  int i0;
};

struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1
{
  int i;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_0 operator->(){}
};

void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1A()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
  s-> 
    i0;
}
#else
#endif

//============================================================================
// partial compiling tdcinc\*.cpp 
//============================================================================

/** VC6 can not handle more than 30000 types per compilation unit, but this 
    has not so much to do with this, here we are trying to minimize the overall
    memory usage during compilation, this is a problem for the umts asn1 file
    the big isue is that determin this number is not an excat sicense since
    VC give NO hint on whic entry is coursing this problem if it ocour during
    pass2 of the compilation 

    to overcome this problem CCDGEN generate additional gaurds in tdcinc\*.cpp files

    enum form

      #if TDC_ENUM_PART
      #ifndef TDC_SKIP_FORWARD_MENUM_prefix_enumtypename
      M_TDC_POST_MENUM (prefix_enumtypename)
      #endif
      #endif // TDC_ENUM_PART

    other form (example is for MSG but PRIM, STRUCT, UNION etc. use the same layout)

      #if N / TDC_SIZE == TDC_PART
      #ifndef TDC_SKIP_FORWARD_msgname
      M_TDC_POST_MSG (msgname)
      #endif
      #endif // N / TDC_SIZE == TDC_PART 

    where N is a sequence number running from 0 and onwards, incremented by 1 for
    each struct in the file

    by defining TDC_SIZE and TDC_PART the source files can be partial compiled
    the default behaviour is to compile the hole file at once

*/

#ifndef TDC_PART
#define TDC_SIZE 99999L // number of structs per compilation 
#define TDC_PART 0L // which struct part to compile in this session
#define TDC_ENUM_PART 1 // if enum part is to be compiled in this session
#elif TDC_PART == -1
#define TDC_ENUM_PART 1 // if enum part is to be compiled in this session
#else
#define TDC_ENUM_PART 0 // if enum part is to be compiled in this session
#endif

//============================================================================
// configuration side effects
//============================================================================

#ifndef TDC_DESCRIPTOR

#define NO_TDC_DESCRIPTOR

#else //TDC_DESCRIPTOR

#define __T_TDC_INTERFACE_MESSAGE__
#define __T_TDC_INTERFACE_PRIMITIVE__

#endif //TDC_DESCRIPTOR

//============================================================================
/// \defgroup dot_complete helper macros
//============================================================================
//\{

#if DOT_COMPLETE
#define TDC_DOT_COMPLETE_HIDE(code) \
  /* nothing */
struct T_TDC_EMPTY {};
#else//DOT_COMPLETE
#define TDC_DOT_COMPLETE_HIDE(code) \
  code
#endif//DOT_COMPLETE

//\}

//============================================================================
// 
//============================================================================

///tdc_test_header_begin
#ifndef M_TDC_DOC
#if DOT_COMPLETE

	void TDC_NOTHING() {} // generate code for breakpoint 
	size_t TDC_LENGTHOF(array) {}
  void TDC_DYNAMIC_DEAD_CODE() {} // program execution is not suposed to reach this point

#else //DOT_COMPLETE
  
  void tdc_nothing();
  /** generate code for breakpoint */
  #define TDC_NOTHING() \
    tdc_nothing()

  #define TDC_DYNAMIC_DEAD_CODE() tdc_dynamic_dead_code() // program execution is not suposed to reach this point


  #define TDC_LENGTHOF(array) (size_t(sizeof array / sizeof *array))

  #define M_TDC_STRING1(text) #text 
  /** expand macros before stringify */
  #define M_TDC_STRING(text) M_TDC_STRING1(text) 

#endif //DOT_COMPLETE
#endif //M_TDC_DOC

// macro to show pragma message with file name and line number when compiling (see M_TDC_MESSAGE_DEMO below)
//#define M_TDC_MESSAGE(text) message(__FILE__ "(" M_TDC_STRING(__LINE__) ") : message: \'" text "\'") 
#define M_TDC_MESSAGE(text) message(__FILE__ "(" M_TDC_STRING(__LINE__) ") : message: \'" M_TDC_STRING(text) "\'") 
#ifdef M_TDC_MESSAGE_DEMO
// e.g. show how a macro is expanded
#define MY(X) MY_##X
#pragma M_TDC_MESSAGE(M_TDC_STRING(MY(Y))) 
// compiler output: \gpf\inc\tdc_base.h(130) : message: 'MY_Y'
#endif
  
/// \defgroup IN_CLASS values for IN_CLASS macro argument
//\{
#define M_TDC_IN_CLASS(code) code
#define M_TDC_NOT_IN_CLASS(code) 
#define M_TDC_NOT_M_TDC_IN_CLASS(code) 
#define M_TDC_NOT_M_TDC_NOT_IN_CLASS(code) code
//\}

/// \defgroup WITH_BODY values for WITH_BODY macro argument 
//\{
#define M_TDC_WITH_BODY(code) code
#define M_TDC_WITHOUT_BODY(code) ;
//\}

/// temporary removed multiline comments in multiline macros 
/// <pre>(some compilers have problems with constructions like:
///   #define MY_MACRO() /* \\
///     my comment */ my_code
/// )</pre>
#define M_TDC_COMMENT(code) ;

  
/// \defgroup MacroTesting
/** for testing macros
 *  these macros is only used when tdc.dsw has the active project set to "tdc_lib - Win32 Precompile"
 */
//\{
#define TDC_MACRO_BEGIN_(name,line)
#define TDC_MACRO_(name,line) 
#define TDC_MACRO_END_()
//\}

// /tdc_test_header_end

#ifndef USE_TDC_TEST_I_II// during testing we see this part in the preprocessed file tdc_test_i.ii

struct T_TDC_BASE_DOT_COMPLETE_TEST
{
	int i; //HINT
};

#define TDC_TBD() tdc_tbd(__FILE__, __LINE__)

#define TDC_ASSERT(condition) tdc_assert(condition,#condition,__FILE__,__LINE__)

#ifndef TDC_DEBUG
	#define IF_TDC_DEBUG(code)
#else
	#define IF_TDC_DEBUG(code) code
#endif

#ifndef TDC_TEST_PROFILE

	#define TDC_PURE_BODY(code)\
		= 0;

#else

	#define TDC_PURE_BODY(code)\
		{\
			tdc_pure_body();\
			code\
		}\

#endif

#if TDC_DEBUG_DOT_COMPLETE
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1C()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
}
#else
#endif

void tdc_cursor_trace (char* file, int line, char* format, ...);
void tdc_trace (char* format, ...);
void __declspec(noreturn) tdc_trace_fail (char* format, ...); /*lint -sem(tdc_trace_fail,r_no)*/
extern "C" void tdc_usi_trace (char* format, ...);
void __declspec(noreturn) tdc_internal_error (char* format, ...); /*lint -sem(tdc_internal_error,r_no)*/
void __declspec(noreturn) tdc_user_error (char* format, ...); /*lint -sem(tdc_user_error,r_no)*/
void tdc_check_array_assignment(const void* that, const void* address);

// some error reporting functions, which have no argument becourse they have to be called from a lot of places
void __declspec(noreturn) tdc_tbd(char* file, int line); /*lint -sem(tdc_tbd,r_no)*/// this facility is yet to be defined i.e. implemented
void __declspec(noreturn) tdc_tbd(char* message); /*lint -sem(tdc_tbd,r_no)*/// this facility is yet to be defined i.e. implemented

void __declspec(noreturn) tdc_tbd_pointer_assignment_error();
void __declspec(noreturn) tdc_tbd_array_assignment_error();
void __declspec(noreturn) tdc_tbd_array_assignment_error_T_ARRAY();
void __declspec(noreturn) tdc_tbd_xxx_constructor_call(char* message);
void __declspec(noreturn) tdc_tbd_constructor_call(char* message);
void __declspec(noreturn) tdc_tbd_primitive_union_constructor_call();
void __declspec(noreturn) tdc_tbd_message_union_constructor_call();

void tdc_pure_body();
void tdc_dynamic_dead_code(); // program execution is not suposed to reach this point
void tdc_missing_h_file(); // use of AIM/SAP not included
void tdc_assert(int condition, char* message, char* file, int line);

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1D()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
}
#else
#endif

//----------------------------------------------------------------------------

//tdc_test_header_begin

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
#ifdef M_TDC_DOC
/*
  The definitions in this sections is only present to be able to generate call graphs to be used 
  in documentation (call graphs are generated with SourceInsight 3.5)
*/
/// \defgroup NamingScheme Naming convention template tree 
//\{
/// \callgraph
#define BASE() MAIN() PART() 
#define MAIN() all_unions() PRIM_and_MSG() XXX()
#define all_unions() PRMITIVE_UNION() MESSAGE_UNION() 
#define XXX() XXX_PRMITIVE_UNION() XXX_MESSAGE_UNION()
#define PRIM_and_MSG() PRIM() MSG()
#define PART() COMP() VAR()
#define COMP() STRUCT() UNION()
#define VAR() INT() ENUM()
#define ENUM() PENUM() MENM()
#define PRMITIVE_UNION() 
#define MESSAGE_UNION()
#define XXX_PRMITIVE_UNION() 
#define XXX_MESSAGE_UNION()
#define PRIM() 
#define MSG() 
#define STRUCT() PSTRUCT() MSTRUCT() 
#define UNION() PUNION() PUNION()
#define PSTRUCT() 
#define MSTRUCT() 
#define PUNION()
#define PUNION()
#define PENUM() 
#define MENM()
#define INT()
//\}
// base entries
/*
#define M_TDC_BASE\
  M_TDC_FORWARD_BASE(SHORT_NAME, T_HANDLE)\
  M_TDC_INSTANCE_ADDITIONAL_BASE(IN_CLASS, WITH_BODY, T_INTERFACE_, T_INSTANCE, T_HANDLE_)\
  M_TDC_INTERFACE_ADDITIONAL_BASE(IN_CLASS, WITH_BODY, T_INTERFACE, T_INSTANCE_, T_HANDLE)\
  M_TDC_DESCRIPTOR_ADDITIONAL_BASE(SHORT_NAME)\
  M_TDC_FORWARD_BASE(SHORT_NAME, T_HANDLE)\
 //*/
//TDC keywords
void TDC_keywords() 
{
  FAIL();
  PASS();
  SEND(primitive);
  AWAIT(primitive);
  START_TIMEOUT(timeout);
  WAIT_TIMEOUT();
  MUTE(timeout);
  COMMAND(command);
  TIMEOUT(timeout);
  PARKING(enable);
  ALT() //dummy "()"to force call graph 
  {
    ON(await);
    OTHERWISE();
  }
  TRAP() //dummy "()"to force call graph
  {
  }
  ONFAIL() //dummy "()"to force call graph
  {
  }
}
#endif
//----------------------------------------------------------------------------

#if DOT_COMPLETE_DEFINES

#define CCC(COMMENT)

//----------------------------------------------------------------------------
// macros to generate dummy functions body
//----------------------------------------------------------------------------

#define M_TDC_TYPE_NAME\
	//for(;;); /* no error for missing return */

#define T_TDC_TYPE_NAME

//----------------------------------------------------------------------------
// macros to repeat stuff that is similar in many descriptor classes
//----------------------------------------------------------------------------

#define M_TDC_FORWARD_PRIM(SAP,SHORT_NAME)
#define M_TDC_FORWARD_PSTRUCT(SHORT_NAME)
#define M_TDC_FORWARD_PUNION(SHORT_NAME)
#define M_TDC_FORWARD_SDU(SHORT_NAME)
#define M_TDC_FORWARD_PENUM(SHORT_NAME)
//-----
#define M_TDC_FORWARD_MSG(SHORT_NAME)
#define M_TDC_FORWARD_MSTRUCT(SHORT_NAME)
#define M_TDC_FORWARD_MUNION(SHORT_NAME)
#define M_TDC_FORWARD_MENUM(SHORT_NAME)
//-----
#define M_TDC_FORWARD_INT(SHORT_NAME)
#define M_TDC_FORWARD_XXX_PRIMITIVE_UNION(SAP)
#define M_TDC_FORWARD_XXX_MESSAGE_UNION(MSG)
#define M_TDC_FORWARD_PRIMITIVE()
#define M_TDC_FORWARD_MESSAGE()

//----------------------------------------------------------------------------
// macros to repeat stuff that is similar in many descriptor classes
//----------------------------------------------------------------------------

#define M_TDC_DESCRIPTOR_PRIM_ADDITIONAL(SHORT_NAME)
#define M_TDC_DESCRIPTOR_PSTRUCT_ADDITIONAL(SHORT_NAME)
#define M_TDC_DESCRIPTOR_PUNION_ADDITIONAL(SHORT_NAME)
#define M_TDC_DESCRIPTOR_SDU_ADDITIONAL(SHORT_NAME)
#define M_TDC_DESCRIPTOR_PENUM_ADDITIONAL(SHORT_NAME)
//-----
#define M_TDC_DESCRIPTOR_MSG_ADDITIONAL(SHORT_NAME)
#define M_TDC_DESCRIPTOR_MSTRUCT_ADDITIONAL(SHORT_NAME)
#define M_TDC_DESCRIPTOR_MUNION_ADDITIONAL(SHORT_NAME)
#define M_TDC_DESCRIPTOR_MENUM_ADDITIONAL(SHORT_NAME)
//-----
#define M_TDC_DESCRIPTOR_INT_ADDITIONAL(SHORT_NAME)
#define M_TDC_DESCRIPTOR_XXX_PRIMITIVE_UNION_ADDITIONAL(SAP)
#define M_TDC_DESCRIPTOR_XXX_MESSAGE_UNION_ADDITIONAL(MSG)
#define M_TDC_CREATE_DEFAULT_PRIMITIVE_DESCRIPTOR_CLASS(SAP)
#define M_TDC_DESCRIPTOR_PRIMITIVE_ADDITIONAL()
#define M_TDC_CREATE_DEFAULT_MESSAGE_DESCRIPTOR_CLASS(MSG)
#define M_TDC_DESCRIPTOR_MESSAGE_ADDITIONAL()

//----------------------------------------------------------------------------
// macros to repeat stuff that is similar in many interface classes 
//----------------------------------------------------------------------------

#ifndef TDC_T_TDC_INTERFACE_DOT_COMPLETE_TEST 
#define TDC_T_TDC_INTERFACE_DOT_COMPLETE_TEST
struct T_TDC_INTERFACE_DOT_COMPLETE_TEST
{
	typedef char T_HANDLE; 
};
#endif //TDC_T_TDC_INTERFACE_DOT_COMPLETE_TEST 

//-----
#define M_TDC_INTERFACE_PRIM_ADDITIONAL(SHORT_NAME) /*nothing*/
#define M_TDC_INTERFACE_PSTRUCT_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INTERFACE_PUNION_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INTERFACE_SDU_ADDITIONAL(SHORT_NAME) /*nothing*/
#define M_TDC_INTERFACE_PENUM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
//-----
#define M_TDC_INTERFACE_MSG_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INTERFACE_MSTRUCT_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INTERFACE_MUNION_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INTERFACE_MENUM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
//-----
#define M_TDC_INTERFACE_INT_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INTERFACE_XXX_PRIMITIVE_UNION_ADDITIONAL(SAP,PREFIXED_SAP) /*nothing*/
#define M_TDC_INTERFACE_XXX_MESSAGE_UNION_ADDITIONAL(MSG,PREFIXED_MSG) /*nothing*/
#define M_TDC_CREATE_DEFAULT_PRIMITIVE_INTERFACE_CLASS(SAP) /*nothing*/
#define M_TDC_INTERFACE_PRIMITIVE_ADDITIONAL() /*nothing*/
#define M_TDC_CREATE_DEFAULT_MESSAGE_INTERFACE_CLASS(MSG) /*nothing*/
#define M_TDC_INTERFACE_MESSAGE_ADDITIONAL() /*nothing*/

//----------------------------------------------------------------------------
// macros to repeat stuff that is similar in many instance classes 
//----------------------------------------------------------------------------

#define M_TDC_INSTANCE_PRIM_ADDITIONAL(SAP,SHORT_NAME) /*nothing*/
#define M_TDC_INSTANCE_PSTRUCT_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INSTANCE_PUNION_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INSTANCE_SDU_ADDITIONAL(SHORT_NAME) /*nothing*/
#define M_TDC_INSTANCE_PENUM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
//-----
#define M_TDC_INSTANCE_MSG_ADDITIONAL(MSG,SHORT_NAME) /*nothing*/
#define M_TDC_INSTANCE_MSTRUCT_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INSTANCE_MUNION_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
#define M_TDC_INSTANCE_MENUM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME) /*nothing*/
//-----
#define M_TDC_INSTANCE_INT_ADDITIONAL(SHORT_NAME) /*nothing*/
#define M_TDC_INSTANCE_XXX_PRIMITIVE_ADDITIONAL(SAP) /*nothing*/
#define M_TDC_INSTANCE_XXX_MESSAGE_ADDITIONAL(MSG) /*nothing*/
#define M_TDC_INSTANCE_PRIMITIVE_ADDITIONAL() /*nothing*/
#define M_TDC_INSTANCE_MESSAGE_ADDITIONAL() /*nothing*/

#else
#endif

//tdc_test_header_end

#if DOT_COMPLETE_DEFINES
#else //DOT_COMPLETE_DEFINES

#ifndef TDC_KEEP_CCC
#define CCC(COMMENT)
#endif //TDC_KEEP_CCC

//----------------------------------------------------------------------------
// macros to control namespace
//----------------------------------------------------------------------------

#define M_TDC_ENTER() //TODO namespace TDC {
#define M_TDC_LEAVE() //TODO }

//----------------------------------------------------------------------------
// macros to generate dummy functions body
//----------------------------------------------------------------------------

#define M_TDC_TYPE_NAME\
	return T_TDC_TYPE_NAME();

//#define T_TDC_TYPE_NAME M_TDC_HIDE(T_TDC_TYPE_NAME)

//----------------------------------------------------------------------------
// macros to hide templates from dot-completion 
//----------------------------------------------------------------------------

typedef char T_TDC_LEVEL;
extern T_TDC_LEVEL tdc_level;

//----------------------------------------------------------------------------
// macros to repeat stuff that is similar in many descriptor classes
//----------------------------------------------------------------------------

#ifndef TDC_DESCRIPTOR


#define M_TDC_FORWARD_BASE(SHORT_NAME,T_HANDLE)\
  struct T_TDC_INTERFACE_##SHORT_NAME;\
  T_TDC_INTERFACE_BASE* new_T_TDC_INTERFACE_##SHORT_NAME();\
  struct T_HANDLE;\
  T_TDC_HANDLE_BASE* new_##T_HANDLE();\

#define M_TDC_FORWARD_COMP(SHORT_NAME,TDC_IS)\
	M_TDC_FORWARD_BASE(SHORT_NAME, T_TDC_HANDLE_##SHORT_NAME)\

#define M_TDC_FORWARD_VAR(SHORT_NAME)\
	M_TDC_FORWARD_BASE(SHORT_NAME, T_TDC_DESCRIPTOR_##SHORT_NAME)\


#define M_TDC_POST_BASE(SHORT_NAME,T_HANDLE)\
  M_TDC_INTERFACE_ADDITIONAL_BASE (M_TDC_NOT_IN_CLASS, M_TDC_WITH_BODY, T_TDC_INTERFACE_##SHORT_NAME, T_##SHORT_NAME, T_HANDLE)\
  M_TDC_INSTANCE_ADDITIONAL_BASE (M_TDC_NOT_IN_CLASS, M_TDC_WITH_BODY, T_TDC_INTERFACE_##SHORT_NAME, T_##SHORT_NAME, T_HANDLE)\

#define M_TDC_POST_XXX(SHORT_NAME,TDC_IS)\
  M_TDC_POST_BASE(SHORT_NAME, T_TDC_HANDLE_##SHORT_NAME)\

#define M_TDC_POST_COMP(SHORT_NAME,TDC_IS)\
  M_TDC_POST_BASE(SHORT_NAME, T_TDC_HANDLE_##SHORT_NAME)\

#define M_TDC_POST_VAR(SHORT_NAME)\
  M_TDC_POST_BASE(SHORT_NAME, T_TDC_DESCRIPTOR_##SHORT_NAME)\


#else //TDC_DESCRIPTOR


#define M_TDC_FORWARD_BASE(SHORT_NAME,T_HANDLE)\

#define M_TDC_FORWARD_COMP(SHORT_NAME,TDC_IS)\
  M_TDC_FORWARD_DESCRIPTOR_COMP(SHORT_NAME,TDC_IS)\

// TODO:eliminate forwards: T_##SHORT_NAME and T_TDC_INTERFACE_##SHORT_NAME
#define M_TDC_FORWARD_VAR(SHORT_NAME)\
  M_TDC_FORWARD_BASE(SHORT_NAME,T_TDC_HANDLE_##SHORT_NAME)\
  struct T_TDC_DESCRIPTOR_##SHORT_NAME;\
  struct T_##SHORT_NAME;\
  struct T_TDC_INTERFACE_##SHORT_NAME;\
  typedef T_TDC_DESCRIPTOR_##SHORT_NAME T_TDC_HANDLE_##SHORT_NAME;\
  T_TDC_HANDLE_BASE* new_T_TDC_DESCRIPTOR_##SHORT_NAME();\

#define M_TDC_POST_DESCRIPTOR_BASE(SHORT_NAME,T_HANDLE)\
  T_TDC_HANDLE_BASE* new_##T_HANDLE()\
  {\
    return new T_HANDLE;\
  }\

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_6
{
  int i6;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};

inline void dot_complete_test(){
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_6 tdc_debug_dot_complete__tdc_h;
  tdc_debug_dot_complete__tdc_h. i6;
  tdc_debug_dot_complete__tdc_h-> i;
}
#else
#endif

#define M_TDC_POST_XXX(SHORT_NAME,TDC_IS)\
  M_TDC_POST_DESCRIPTOR_BASE(SHORT_NAME,T_TDC_HANDLE_##SHORT_NAME)\
  T_TDC_DESCRIPTOR_BASE* new_T_TDC_DESCRIPTOR_##SHORT_NAME ()\
  {\
    return new T_TDC_DESCRIPTOR_##SHORT_NAME;\
  }\

#define M_TDC_POST_COMP(SHORT_NAME, TDC_IS)\
  M_TDC_POST_DESCRIPTOR_COMP(SHORT_NAME, TDC_IS)\

#define M_TDC_POST_VAR(SHORT_NAME)\
  M_TDC_POST_DESCRIPTOR_BASE(SHORT_NAME, T_TDC_DESCRIPTOR_##SHORT_NAME)\
  
  
#endif //TDC_DESCRIPTOR


#define M_TDC_FORWARD_DESCRIPTOR_COMP(SHORT_NAME,TDC_IS)\
  M_TDC_HANDLE(M_TDC_IN_CLASS,M_TDC_WITHOUT_BODY,SHORT_NAME,TDC_IS)\
  
#define M_TDC_POST_DESCRIPTOR_COMP(SHORT_NAME, TDC_IS)\
  M_TDC_POST_DESCRIPTOR_BASE(SHORT_NAME,T_TDC_HANDLE_##SHORT_NAME)\
  T_TDC_DESCRIPTOR_BASE* T_TDC_HANDLE_##SHORT_NAME::implement_new_descriptor () const\
  {\
    return new T_TDC_DESCRIPTOR_##SHORT_NAME;\
  }\
  M_TDC_HANDLE_ADDITIONAL(M_TDC_NOT_IN_CLASS,M_TDC_WITH_BODY,T_TDC_HANDLE_##SHORT_NAME,SHORT_NAME,TDC_IS)\

  
/*lint -emacro(1706,M_TDC_HANDLE_ADDITIONAL)*/ //we don't want M_TDC_NOT(IN_CLASS(...)) for all the scope operators
#define M_TDC_HANDLE_ADDITIONAL(IN_CLASS,WITH_BODY,T_HANDLE_,SHORT_NAME,TDC_IS)\
    IN_CLASS(virtual) char* T_HANDLE_::get_name () const\
    WITH_BODY({\
      return #SHORT_NAME;\
    })\
    IN_CLASS(virtual) long T_HANDLE_::get_sizeof ()\
    WITH_BODY({\
      return sizeof *this;\
    })\
    IN_CLASS(virtual) T_TDC_IS_ENUM T_HANDLE_::is ()\
    WITH_BODY({\
      return T_TDC_IS_ENUM (TDC_IS_COMP | TDC_IS);\
    })\
    IN_CLASS(static) T_TDC_HANDLE_BASE* T_HANDLE_::implement_new_handle ()\
    WITH_BODY({\
      return new T_HANDLE_();\
    })\

#define M_TDC_HANDLE(IN_CLASS,WITH_BODY,SHORT_NAME,TDC_IS)\
  struct T_TDC_DESCRIPTOR_##SHORT_NAME;\
  struct T_TDC_HANDLE_##SHORT_NAME:T_TDC_HANDLE_BASE\
  {\
    typedef T_TDC_DESCRIPTOR_##SHORT_NAME T_DESCRIPTOR;\
    M_TDC_HANDLE_ADDITIONAL(IN_CLASS,WITH_BODY,T_TDC_HANDLE_##SHORT_NAME,SHORT_NAME,TDC_IS)\
    M_TDC_DESCRIPTOR_HANDLE_ADDITIONAL_PART (T_TDC_DESCRIPTOR_##SHORT_NAME)\
  protected:\
    virtual T_TDC_DESCRIPTOR_BASE* implement_new_descriptor () const;\
  };\

#if TDC_DEBUG_DOT_COMPLETE
inline void dot_complete_test(){
T_TDC_DEBUG_DOT_COMPLETE__TDC_H_6 tdc_debug_dot_complete__tdc_h;
tdc_debug_dot_complete__tdc_h. i6;
tdc_debug_dot_complete__tdc_h-> i;
}
#else
#endif

//------

#define M_TDC_FORWARD_PRIM(SHORT_NAME)\
	M_TDC_FORWARD_COMP (SHORT_NAME,TDC_IS_PRIM)

#define M_TDC_FORWARD_PSTRUCT(SHORT_NAME)\
	M_TDC_FORWARD_COMP (SHORT_NAME,TDC_IS_STRUCT)

#define M_TDC_FORWARD_PUNION(SHORT_NAME)\
	M_TDC_FORWARD_COMP (SHORT_NAME,TDC_IS_UNION)

#define M_TDC_FORWARD_SDU(SHORT_NAME)\
	M_TDC_FORWARD_COMP (SHORT_NAME,TDC_IS_SDU)

#define M_TDC_FORWARD_TYPEDEF_SDU(BUF)\
	typedef T_TDC_HANDLE_SDU T_TDC_HANDLE_##BUF;\
	typedef T_TDC_DESCRIPTOR_SDU T_TDC_DESCRIPTOR_##BUF;\

/**TODO: eliminate this macro*/
#define M_FORWARD_TYPEDEF_SDU(BUF)\
	M_TDC_FORWARD_TYPEDEF_SDU(BUF)\

#define M_TDC_FORWARD_TYPEDEF(BASE,SHORT_NAME)\
	typedef T_TDC_HANDLE_##BASE T_TDC_HANDLE_##SHORT_NAME;\
	typedef T_TDC_DESCRIPTOR_##BASE T_TDC_DESCRIPTOR_##SHORT_NAME;\

#define M_TDC_FORWARD_PENUM(SHORT_NAME)\
	M_TDC_FORWARD_VAR (SHORT_NAME)

#if TDC_DEBUG_DOT_COMPLETE
inline void dot_complete_test(){
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_6 tdc_debug_dot_complete__tdc_h;
  tdc_debug_dot_complete__tdc_h. i6;
  tdc_debug_dot_complete__tdc_h-> i;
}
#else
#endif

//------

#define M_TDC_FORWARD_MSG(SHORT_NAME)\
	M_TDC_FORWARD_COMP (SHORT_NAME,TDC_IS_MSG)

#define M_TDC_FORWARD_MSTRUCT(SHORT_NAME)\
	M_TDC_FORWARD_COMP (SHORT_NAME,TDC_IS_STRUCT)

#define M_TDC_FORWARD_MUNION(SHORT_NAME)\
	M_TDC_FORWARD_COMP (SHORT_NAME,TDC_IS_UNION)

#define M_TDC_FORWARD_MENUM(SHORT_NAME)\
	M_TDC_FORWARD_VAR (SHORT_NAME)

//------

#define M_TDC_FORWARD_INT(SHORT_NAME)\
	M_TDC_FORWARD_BASE(SHORT_NAME, T_TDC_DESCRIPTOR_##SHORT_NAME)\
  struct T_TDC_DESCRIPTOR_##SHORT_NAME;\
	typedef T_TDC_DESCRIPTOR_##SHORT_NAME T_TDC_HANDLE_##SHORT_NAME;\
	typedef T_TDC_INT_##SHORT_NAME SHORT_NAME;\
	T_TDC_HANDLE_BASE* new_T_TDC_DESCRIPTOR_##SHORT_NAME();\

#if TDC_DEBUG_DOT_COMPLETE
inline void dot_complete_test(){
T_TDC_DEBUG_DOT_COMPLETE__TDC_H_6 tdc_debug_dot_complete__tdc_h;
tdc_debug_dot_complete__tdc_h. i6;
tdc_debug_dot_complete__tdc_h-> i;
}
#else
#endif

#define M_TDC_FORWARD_XXX_PRIMITIVE_UNION(SAP)\
	M_TDC_FORWARD_BASE (SAP, T_TDC_HANDLE_##SAP)\

#define M_TDC_FORWARD_XXX_MESSAGE_UNION(MSG)\
	M_TDC_FORWARD_BASE (MSG,T_TDC_HANDLE_##MSG)\

#if TDC_DEBUG_DOT_COMPLETE
inline void dot_complete_test(){
T_TDC_DEBUG_DOT_COMPLETE__TDC_H_6 tdc_debug_dot_complete__tdc_h;
tdc_debug_dot_complete__tdc_h. i6;
tdc_debug_dot_complete__tdc_h-> i;
}
#else
#endif

//----------------------------------------------------------------------------
// macros to repeat stuff that is similar in many descriptor classes
//----------------------------------------------------------------------------

#define M_TDC_POST_PRIM(SHORT_NAME)\
	M_TDC_POST_COMP (SHORT_NAME, TDC_IS_PRIM)

#define M_TDC_POST_PSTRUCT(SHORT_NAME)\
	M_TDC_POST_COMP (SHORT_NAME, TDC_IS_STRUCT)

#define M_TDC_POST_PUNION(SHORT_NAME)\
	M_TDC_POST_COMP (SHORT_NAME, TDC_IS_UNION)

#define M_TDC_POST_SDU(SHORT_NAME)

#define M_TDC_POST_PENUM(SHORT_NAME)\
	M_TDC_POST_VAR (SHORT_NAME)

#define M_TDC_POST_MSG(SHORT_NAME)\
	M_TDC_POST_COMP (SHORT_NAME, TDC_IS_MSG)

#define M_TDC_POST_MSTRUCT(SHORT_NAME)\
	M_TDC_POST_COMP (SHORT_NAME, TDC_IS_STRUCT)

#define M_TDC_POST_MUNION(SHORT_NAME)\
	M_TDC_POST_COMP (SHORT_NAME, TDC_IS_UNION)

#define M_TDC_POST_MENUM(SHORT_NAME)\
	M_TDC_POST_VAR (SHORT_NAME)

#define M_TDC_POST_INT(SHORT_NAME)

#define M_TDC_POST_XXX_PRIMITIVE_UNION(SAP)\
	M_TDC_POST_XXX(SAP,TDC_IS_XXX_PRIMITIE_UNION)

#define M_TDC_POST_XXX_MESSAGE_UNION(MSG)\
	M_TDC_POST_XXX(MSG,TDC_IS_XXX_MESSAGE_UNION)

#define M_TDC_POST_PRIMITIVE()

#define M_TDC_POST_MESSAGE()

#if TDC_DEBUG_DOT_COMPLETE
inline void dot_complete_test(){
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_6 tdc_debug_dot_complete__tdc_h;
  tdc_debug_dot_complete__tdc_h. i6;
  tdc_debug_dot_complete__tdc_h-> i;
}

struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_5
{
  int i5;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};

inline void dot_complete_test(){
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_5 tdc_debug_dot_complete__tdc_h;
  tdc_debug_dot_complete__tdc_h. i4;
  tdc_debug_dot_complete__tdc_h-> i;
}
#else
#endif

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1F
{
  int i1F;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1F()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1F s1F;
  s1F->
    i0;
}
#else
#endif

//----------------------------------------------------------------------------
/// \defgroup M_TDC_DESCRIPTOR M_TDC_DESCRIPTOR
/// macros to repeat stuff that is similar in many descriptor classes
//----------------------------------------------------------------------------
//\{

/** declare action and description elements that are accessed through typecast to stencil */
#define M_TDC_DESCRIPTOR_HANDLE_ADDITIONAL_PART(T_DESCRIPTOR)\
	  T_TDC_ACTION_ENUM action TDC_LINT_UNUSED_MEMBER;\
	  T_DESCRIPTOR* descriptor TDC_LINT_UNUSED_MEMBER;\

#define	M_TDC_DESCRIPTOR_ADDITIONAL_BASE(SHORT_NAME)\
	private:\
		virtual long get_sizeof ()\
		{\
			return sizeof *this;\
		}\

#define	M_TDC_DESCRIPTOR_ADDITIONAL_MAIN_BASE(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_BASE(SHORT_NAME)\
	public:\
		int ctrl;\
		virtual int read_ctrl () const\
		{\
			return ctrl;\
		}\
		T_TDC_HANDLE_BASE* get_element (unsigned index)\
		{\
			return implement_get_union_element (index, ctrl);\
		}\
		virtual char* get_name () const\
		{\
			return #SHORT_NAME;\
		}\
		T_TDC_DESCRIPTOR_##SHORT_NAME ():\
			ctrl (0)\
		{\
		}\


#define	M_TDC_DESCRIPTOR_ADDITIONAL_STRUCT_BASE(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_BASE(SHORT_NAME)\
	public:\
		typedef  T_TDC_HANDLE_##SHORT_NAME T_HANDLE;\
		virtual char* get_name () const\
		{\
      extern char* TDC_NAME_##SHORT_NAME;\
			return TDC_NAME_##SHORT_NAME;\
		}\
		friend T_TDC_DESCRIPTOR_BASE* new_T_TDC_DESCRIPTOR_##SHORT_NAME ()\
		{\
			return new T_TDC_DESCRIPTOR_##SHORT_NAME;\
		}\
		T_TDC_DESCRIPTOR_##SHORT_NAME ()\
		{\
		}\

#define	M_TDC_DESCRIPTOR_ADDITIONAL_UNION_BASE(SHORT_NAME,TABLE_KIND)\
	M_TDC_DESCRIPTOR_ADDITIONAL_BASE(SHORT_NAME)\
	public:\
		typedef  T_TDC_HANDLE_##SHORT_NAME T_HANDLE;\
		int ctrl;\
		virtual int read_ctrl () const\
		{\
			return ctrl;\
		}\
		T_TDC_HANDLE_BASE* get_element (unsigned index)\
		{\
			return implement_get_union_element (index, ctrl);\
		}\
		virtual char* get_name () const\
		{\
      extern char* TDC_NAME_##SHORT_NAME;\
			return TDC_NAME_##SHORT_NAME;\
		}\
		T_TDC_DESCRIPTOR_##SHORT_NAME ():\
			ctrl (0)\
		{\
		}\

/**
 * \todo move get_tap_handle() to var base class 
 */
#define M_TDC_DESCRIPTOR_ADDITIONAL_VAR(T_DESCRIPTOR, T_VAR, SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_BASE(SHORT_NAME)\
	public:\
	typedef T_DESCRIPTOR T_HANDLE;\
	T_TDC_ACTION_ENUM action;\
	union \
	{\
		T_VAR value;\
		T_TDC_BASIC_TYPES value_as_basic_types;\
	};\
	virtual char* get_name () const\
	{\
    extern char* TDC_NAME_##SHORT_NAME;\
    return TDC_NAME_##SHORT_NAME;\
	}\
	T_DESCRIPTOR ():\
		action (TDC_ACTION_DEFAULT)\
	{\
	}\
	virtual unsigned get_sizeof_target ()\
	{\
		return sizeof T_VAR;\
	}\
	static T_TDC_HANDLE_BASE* implement_new_handle ()\
	{\
		return  new_##T_DESCRIPTOR();\
	}\

//-----

#define M_TDC_DESCRIPTOR_INT_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_VAR (T_TDC_DESCRIPTOR_##SHORT_NAME, T_TDC_INT_##SHORT_NAME,SHORT_NAME)\
	virtual T_TDC_IS_ENUM is ()\
	{\
		return TDC_IS_VAR;\
	}\

//-----

#define M_TDC_DESCRIPTOR_PRIM_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_STRUCT_BASE (SHORT_NAME)\
		virtual int get_id () const\
		{\
			return SHORT_NAME;\
		}\

#define M_TDC_DESCRIPTOR_PSTRUCT_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_STRUCT_BASE (SHORT_NAME)\

#define M_TDC_DESCRIPTOR_PUNION_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_UNION_BASE (SHORT_NAME,TDC_TABLE_KIND_PRIM)\

#define M_TDC_DESCRIPTOR_SDU_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_UNION_BASE (SHORT_NAME,TDC_TABLE_KIND_PRIM)\

#define M_TDC_DESCRIPTOR_PENUM_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_VAR (T_TDC_DESCRIPTOR_##SHORT_NAME, T_TDC_ENUM_##SHORT_NAME,SHORT_NAME)\
	friend T_TDC_HANDLE_BASE* new_T_TDC_DESCRIPTOR_##SHORT_NAME ();\

//-----

#define M_TDC_DESCRIPTOR_MSG_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_STRUCT_BASE (SHORT_NAME)\
		virtual int get_id () const\
		{\
			return SHORT_NAME;\
		}\

#define M_TDC_DESCRIPTOR_MSTRUCT_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_STRUCT_BASE (SHORT_NAME)\

#define M_TDC_DESCRIPTOR_MUNION_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_UNION_BASE (SHORT_NAME,TDC_TABLE_KIND_MSG)\

#define M_TDC_DESCRIPTOR_MENUM_ADDITIONAL(SHORT_NAME)\
	M_TDC_DESCRIPTOR_ADDITIONAL_VAR (T_TDC_DESCRIPTOR_##SHORT_NAME, T_TDC_ENUM_##SHORT_NAME,SHORT_NAME)\
	friend T_TDC_HANDLE_BASE* new_T_TDC_DESCRIPTOR_##SHORT_NAME ();\

//-----

#define M_TDC_DESCRIPTOR_XXX_PRIMITIVE_UNION_ADDITIONAL(SAP)\
	M_TDC_DESCRIPTOR_ADDITIONAL_MAIN_BASE (SAP)\

#define M_TDC_DESCRIPTOR_XXX_MESSAGE_UNION_ADDITIONAL(MSG)\
	M_TDC_DESCRIPTOR_ADDITIONAL_MAIN_BASE (MSG)\
		virtual int get_id () const\
		{\
			return CCDENT_##MSG;\
		}\

#ifdef TDC_LIB_MAIN_DSC
#define IF_TDC_LIB_MAIN_DSC(code) code
#else
#define IF_TDC_LIB_MAIN_DSC(code) 
#endif

#define M_TDC_CREATE_DEFAULT_DESCRIPTOR_CLASS(SHORT_NAME, TDC_IS)\
  struct T_TDC_DESCRIPTOR_##SHORT_NAME;\
  struct T_TDC_HANDLE_##SHORT_NAME:T_TDC_HANDLE_BASE\
  {\
    typedef T_TDC_DESCRIPTOR_##SHORT_NAME T_DESCRIPTOR;\
    M_TDC_HANDLE_ADDITIONAL(M_TDC_IN_CLASS,M_TDC_WITH_BODY,T_TDC_HANDLE_##SHORT_NAME,SHORT_NAME,TDC_IS)\
    M_TDC_DESCRIPTOR_HANDLE_ADDITIONAL_PART (T_TDC_DESCRIPTOR_##SHORT_NAME)\
  protected:\
    static T_TDC_NEW_DESCRIPTOR new_descriptor;\
    virtual T_TDC_DESCRIPTOR_BASE* implement_new_descriptor () const\
    {\
      return call_implement_new_descriptor(new_descriptor);\
    }\
    friend char set_new_descriptor_T_TDC_HANDLE_##SHORT_NAME()\
    {\
      extern T_TDC_DESCRIPTOR_BASE* new_T_TDC_DESCRIPTOR_##SHORT_NAME ();\
      new_descriptor = new_T_TDC_DESCRIPTOR_##SHORT_NAME;\
      return 1;\
    }\
  };\
	T_TDC_HANDLE_BASE* new_T_TDC_HANDLE_##SHORT_NAME();\
  M_TDC_XXX_PRIMITIVE_UNION_ADDITIONAL_INLINE_1(SHORT_NAME,SHORT_NAME)\
  IF_TDC_LIB_MAIN_DSC(T_TDC_NEW_DESCRIPTOR T_TDC_HANDLE_##SHORT_NAME::new_descriptor;)\

#define M_TDC_CREATE_DEFAULT_PRIMITIVE_DESCRIPTOR_CLASS(SAP)\
	M_TDC_CREATE_DEFAULT_DESCRIPTOR_CLASS(SAP##_PRIMITIVE_UNION,TDC_IS_XXX_PRIMITIVE_UNION)\

#define M_TDC_CREATE_DEFAULT__T_TDC_HANDLE_XXX_PRIMITIVE_UNION(SAP,CCDSAP)\
  M_TDC_CREATE_DEFAULT_DESCRIPTOR_CLASS(SAP##_PRIMITIVE_UNION,TDC_IS_XXX_PRIMITIVE_UNION)\

#define M_TDC_DESCRIPTOR_PRIMITIVE_ADDITIONAL()\
	M_TDC_DESCRIPTOR_ADDITIONAL_MAIN_BASE (PRIMITIVE_UNION)\

#define M_TDC_CREATE_DEFAULT_MESSAGE_DESCRIPTOR_CLASS(MSG)\
	M_TDC_CREATE_DEFAULT_DESCRIPTOR_CLASS(MSG##_MESSAGE_UNION,TDC_IS_XXX_MESSAGE_UNION)\
	enum { CCDENT_##MSG##_MESSAGE_UNION	= CCDENT_##MSG };\

#define M_TDC_CREATE_DEFAULT__T_TDC_HANDLE_XXX_MESSAGE_UNION(MSG,CCDENT)\
  M_TDC_CREATE_DEFAULT_MESSAGE_DESCRIPTOR_CLASS(MSG)

#define M_TDC_DESCRIPTOR_MESSAGE_ADDITIONAL()\
	M_TDC_DESCRIPTOR_ADDITIONAL_MAIN_BASE (MESSAGE_UNION)\

//\}
//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1H
{
  int i1H;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1H()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1H s1H;
  s1H->
    i0;
}
#else
#endif

//----------------------------------------------------------------------------
/// \defgroup M_TDC_INTERFACE
// macros to repeat stuff that is similar in many inteface classes 
//----------------------------------------------------------------------------
//\{

/**
 * should these functions return T_RETURN / *this instead?	
 */
#define M_TDC_ADDITIONAL_CALL_FUNCTION_BASE(IN_CLASS, WITH_BODY, T_SCOPE,T_RETURN,T_ARG)\
    void T_SCOPE::operator = (T_ARG (*f) ())\
		WITH_BODY({\
			T_ARG value_ = f ();\
			set_value (value_);\
      TDC_LINT_UNASSIGNED_MEMBER\
		})\
    void T_SCOPE::operator = (void (*f) (T_ARG))\
		WITH_BODY({\
			T_ARG value_;\
			f (value_);\
			set_value (value_);\
      TDC_LINT_UNASSIGNED_MEMBER\
		})\

#define M_TDC_INTERFACE_ADDITIONAL_BASE(IN_CLASS, WITH_BODY, T_INTERFACE, T_INSTANCE_, T_HANDLE)\
		IN_CLASS(typedef T_INSTANCE_ T_INSTANCE;)\
		IN_CLASS(friend struct T_INSTANCE_;)\
	IN_CLASS(private:)\
		void T_INTERFACE::set_value (const T_INSTANCE_& value_)\
		WITH_BODY({\
			copy_instance ((const T_TDC_INSTANCE_BASE*)&value_);\
		})\
	IN_CLASS(explicit) T_INTERFACE::T_INTERFACE (const T_INTERFACE& value_)\
		WITH_BODY({\
			copy_interface (&value_);\
      TDC_LINT_UNCOPIED_BASE\
		})\
	IN_CLASS(public:)\
    void T_INTERFACE::operator = (const T_TDC_ACTION& action_)\
		WITH_BODY({\
			set_action (action_);\
      TDC_LINT_UNASSIGNED_MEMBER\
		})\
		void T_INTERFACE::operator = (const T_INTERFACE& value_)\
		WITH_BODY({\
			copy_interface (&value_);\
      TDC_LINT_NO_SELFASSIGN_TEST\
      TDC_LINT_UNASSIGNED_MEMBER\
		})\
		T_INSTANCE_ T_INTERFACE::operator = (const T_INSTANCE_& value_)\
		WITH_BODY({\
			copy_instance (&value_);\
			return value_;\
      TDC_LINT_UNASSIGNED_MEMBER\
		})\
		IN_CLASS(static) T_TDC_HANDLE_BASE* T_INTERFACE::implement_new_handle ()\
		WITH_BODY({\
			return  new_##T_HANDLE();\
		})\
	  IN_CLASS(friend) T_TDC_INTERFACE_BASE* new_##T_INTERFACE()\
	  WITH_BODY({\
		  return new T_INTERFACE;\
	  })\
		M_TDC_ADDITIONAL_CALL_FUNCTION_BASE(IN_CLASS, WITH_BODY, T_INTERFACE,T_INSTANCE,T_INSTANCE)\

#define M_TDC_INTERFACE_ADDITIONAL_MAIN(IN_CLASS, WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, BASE)\
  public:\
    void operator = (const T_TDC_INTERFACE_##BASE##_BASE& value_)\
    {\
      set_main_value (&value_);\
    }\
    void operator = (const T_TDC_INSTANCE_##BASE##_BASE& value_)\
    {\
      set_main_value (&value_);\
    }\
    T_TDC_INTERFACE_##PREFIXED_SHORT_NAME ()\
    {\
      tdc_level--;\
    }\
    M_TDC_INTERFACE_ADDITIONAL_BASE (IN_CLASS, WITH_BODY, T_TDC_INTERFACE_##PREFIXED_SHORT_NAME, T_##SHORT_NAME, T_TDC_HANDLE_##PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_ADDITIONAL_COMP(IN_CLASS, WITH_BODY, SHORT_NAME, PREFIXED_SHORT_NAME)\
  public:\
    T_TDC_INTERFACE_##PREFIXED_SHORT_NAME ()\
    {\
      tdc_level--;\
    }\
    M_TDC_INTERFACE_ADDITIONAL_BASE (IN_CLASS, WITH_BODY, T_TDC_INTERFACE_##PREFIXED_SHORT_NAME, T_##SHORT_NAME, T_TDC_HANDLE_##PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_ADDITIONAL_VAR(IN_CLASS, WITH_BODY, SHORT_NAME, PREFIXED_SHORT_NAME)\
  public:\
    T_TDC_INTERFACE_##PREFIXED_SHORT_NAME ()\
    {\
    }\
    M_TDC_INTERFACE_ADDITIONAL_BASE (IN_CLASS, WITH_BODY, T_TDC_INTERFACE_##PREFIXED_SHORT_NAME, T_##SHORT_NAME, T_TDC_HANDLE_##PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_INT_ADDITIONAL(SHORT_NAME)\
  public:\
    void operator = (int value_)\
    {\
      set_descriptor_value (value_);\
    }\
    M_TDC_INTERFACE_ADDITIONAL_VAR (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME, SHORT_NAME)\

#define M_TDC_INTERFACE_ADDITIONAL_ENUM(IN_CLASS, WITH_BODY, SHORT_NAME, PREFIXED_SHORT_NAME)\
  public:\
    void set_value (int value_)\
    {\
      set_descriptor_value (value_);\
    }\
    void operator = (int value_)\
    {\
      set_descriptor_value (value_);\
    }\
    void set_value (const T_TDC_INSTANCE_INT_BASE &value_)\
    {\
      copy_instance (&value_);\
    }\
    void operator = (const T_TDC_INSTANCE_INT_BASE &value_)\
    {\
      copy_instance (&value_);\
    }\
    M_TDC_INTERFACE_ADDITIONAL_VAR (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME, PREFIXED_SHORT_NAME)\

#define M_TDC_INTERFACE_ADDITIONAL_MAIN_CALL(IN_CLASS, WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME,T_ARG)\
	T_##SHORT_NAME operator = (const T_ARG& value_)\
  {\
		set_value (value_);\
		return value_;\
    TDC_LINT_UNASSIGNED_MEMBER\
	}\

//-----

#define M_TDC_INTERFACE_PRIM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, PRIMITIVE)\
  M_TDC_INTERFACE_ADDITIONAL_MAIN_CALL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, T_PRIMITIVE_UNION)\

#define M_TDC_INTERFACE_PSTRUCT_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INTERFACE_ADDITIONAL_COMP (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_PUNION_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INTERFACE_ADDITIONAL_COMP (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_PENUM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INTERFACE_ADDITIONAL_ENUM (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

//-----

#define M_TDC_INTERFACE_MSG_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, MESSAGE)\
  M_TDC_INTERFACE_ADDITIONAL_MAIN_CALL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, T_MESSAGE_UNION)\

#define M_TDC_INTERFACE_MSTRUCT_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INTERFACE_ADDITIONAL_COMP (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_MUNION_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INTERFACE_ADDITIONAL_COMP (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_MENUM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INTERFACE_ADDITIONAL_ENUM (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

//-----

#define M_TDC_INTERFACE_PRIM_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, PRIMITIVE)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN_CALL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, T_PRIMITIVE_UNION)\

#define M_TDC_INTERFACE_PSTRUCT_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
	M_TDC_INTERFACE_ADDITIONAL_COMP (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_PUNION_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
	M_TDC_INTERFACE_ADDITIONAL_COMP (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_SDU_ADDITIONAL_INLINE(SHORT_NAME)\
	M_TDC_INTERFACE_ADDITIONAL_COMP (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,SHORT_NAME)

#define M_TDC_INTERFACE_PENUM_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
	M_TDC_INTERFACE_ADDITIONAL_ENUM (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

//-----

#define M_TDC_INTERFACE_MSG_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, MESSAGE)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN_CALL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, T_MESSAGE_UNION)\

#define M_TDC_INTERFACE_MSTRUCT_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
	M_TDC_INTERFACE_ADDITIONAL_COMP (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_MUNION_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
	M_TDC_INTERFACE_ADDITIONAL_COMP (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INTERFACE_MENUM_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
	M_TDC_INTERFACE_ADDITIONAL_ENUM (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

//-----

#define M_TDC_INTERFACE_ADDITIONAL_MAIN_XXX(M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY,XXX,PREFIXED_XXX)\
  private:\
		T_TDC_INTERFACE_##XXX (const T_TDC_INTERFACE_PRIMITIVE_BASE& primitive)\
    {\
    tdc_tbd_xxx_constructor_call(#XXX);\
    }\

#define M_TDC_INTERFACE_XXX_PRIMITIVE_UNION_ADDITIONAL(SAP,PREFIXED_SAP)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SAP,PREFIXED_SAP, PRIMITIVE)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN_CALL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SAP,PREFIXED_SAP, T_PRIMITIVE_UNION)\
  M_TDC_INTERFACE_ADDITIONAL_MAIN_XXX (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SAP,PREFIXED_SAP)\
	public:\

#define M_TDC_INTERFACE_XXX_PRIMITIVE_UNION_ADDITIONAL_INLINE(SAP,PREFIXED_SAP)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SAP,PREFIXED_SAP, PRIMITIVE)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN_CALL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SAP,PREFIXED_SAP, T_PRIMITIVE_UNION)\
  M_TDC_INTERFACE_ADDITIONAL_MAIN_XXX (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SAP,PREFIXED_SAP)\
	public:\

#define M_TDC_INTERFACE_XXX_MESSAGE_UNION_ADDITIONAL(MSG,PREFIXED_MSG)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, MSG, PREFIXED_MSG, MESSAGE)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN_CALL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, MSG, PREFIXED_MSG, T_MESSAGE_UNION)\
  M_TDC_INTERFACE_ADDITIONAL_MAIN_XXX (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, MSG,PREFIXED_MSG)\
	public:\

#define M_TDC_INTERFACE_XXX_MESSAGE_UNION_ADDITIONAL_INLINE(MSG,PREFIXED_MSG)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITH_BODY, MSG, PREFIXED_MSG, MESSAGE)\
	M_TDC_INTERFACE_ADDITIONAL_MAIN_CALL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, MSG, PREFIXED_MSG, T_MESSAGE_UNION)\
  M_TDC_INTERFACE_ADDITIONAL_MAIN_XXX (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, MSG,PREFIXED_MSG)\
	public:\

#define M_TDC_CREATE_DEFAULT_INTERFACE_CLASS(SHORT_NAME)\
	struct T_TDC_INTERFACE_##SHORT_NAME;\
	struct T_TDC_HANDLE_##SHORT_NAME;\

#define M_TDC_CREATE_DEFAULT_PRIMITIVE_INTERFACE_CLASS(SAP)\
  M_TDC_CREATE_DEFAULT_INTERFACE_CLASS(SAP##_PRIMITIVE_UNION)\

#define M_TDC_CREATE_DEFAULT__T_TDC_INTERFACE_XXX_PRIMITIVE_UNION(SAP,CCDSAP)\
  enum { CCDSAP_##SAP = CCDSAP };\
  M_TDC_CREATE_DEFAULT_INTERFACE_CLASS(SAP##_PRIMITIVE_UNION)\

#define M_TDC_INTERFACE_PRIMITIVE_ADDITIONAL()\
	M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, PRIMITIVE_UNION, PRIMITIVE_UNION, PRIMITIVE)\
	private:\
		T_TDC_INTERFACE_PRIMITIVE_UNION (const T_TDC_INTERFACE_PRIMITIVE_BASE& primitive)\
		{\
			tdc_tbd_primitive_union_constructor_call();\
		}\
	public:\

#define M_TDC_INTERFACE_PRIMITIVE_ELEMENT_ADDITIONAL(SAP)\
	/*nothing*/

#define M_TDC_CREATE_DEFAULT_MESSAGE_INTERFACE_CLASS(MSG)\
  M_TDC_CREATE_DEFAULT_INTERFACE_CLASS(MSG##_MESSAGE_UNION)\

#define M_TDC_CREATE_DEFAULT__T_TDC_INTERFACE_XXX_MESSAGE_UNION(MSG,CCDENT)\
  enum { CCDENT_##MSG = CCDENT };\
  M_TDC_CREATE_DEFAULT_MESSAGE_INTERFACE_CLASS(MSG)

#define M_TDC_INTERFACE_MESSAGE_ADDITIONAL()\
	M_TDC_INTERFACE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, MESSAGE_UNION,MESSAGE_UNION, MESSAGE)\
	private:\
		T_TDC_INTERFACE_MESSAGE_UNION (const T_TDC_INTERFACE_MESSAGE_BASE& primitive)\
		{\
			tdc_tbd_message_union_constructor_call();\
		}\
	public:\

#define M_TDC_INTERFACE_MESSAGE_ELEMENT_ADDITIONAL(MSG)\
	/*nothing*/

//\}

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1I()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
  s->
    i0;
}
#else
#endif

//----------------------------------------------------------------------------
/// \defgroup M_TDC_INSTANCE M_TDC_INSTANCE
/// macros to repeat stuff that is similar in many instance classes
//----------------------------------------------------------------------------
//\{

/*lint -emacro(1706,M_TDC_INSTANCE_ADDITIONAL_BASE)*/ //we don't want M_TDC_NOT(IN_CLASS(...)) for all the scope operators
#define M_TDC_INSTANCE_ADDITIONAL_BASE(IN_CLASS, WITH_BODY, T_INTERFACE_, T_INSTANCE, T_HANDLE_)\
    IN_CLASS(typedef T_INTERFACE_ T_INTERFACE;)\
    IN_CLASS(typedef T_HANDLE_ T_HANDLE;)\
    IN_CLASS(T_HANDLE* handle;)\
    IN_CLASS(static) T_TDC_HANDLE_BASE* T_INSTANCE::implement_new_handle ()\
    WITH_BODY({\
      return new_##T_HANDLE_();\
    })\
    IN_CLASS(virtual) T_TDC_INTERFACE_BASE* T_INSTANCE::new_interface() const\
    WITH_BODY({\
      return new_##T_INTERFACE_();\
    })\
    void T_INSTANCE::construct()\
    WITH_BODY({\
      construct_handle(new_##T_HANDLE_);\
    })\
    T_INSTANCE::T_INSTANCE ()\
    WITH_BODY({\
      construct();\
      TDC_LINT_UNINITIALIZED_MEMBER\
    })\
    T_INSTANCE::~T_INSTANCE ()\
    WITH_BODY({\
      destroy_handle ();\
      TDC_LINT_POSSIBLE_UNHANDLE_POINTER_MEMBER\
    })\
    T_INSTANCE::T_INSTANCE (const T_INTERFACE_& value_)\
    WITH_BODY({\
      construct_from_interface ((T_TDC_INTERFACE_BASE*)&value_, new_##T_HANDLE_); M_TDC_COMMENT(T_INTERFACE not defined yet; means need cast here to the base type pointer)\
      TDC_LINT_UNINITIALIZED_MEMBER\
    })\
    IN_CLASS(explicit) T_INSTANCE::T_INSTANCE (const T_TDC_HANDLE_BASE* value_)\
    WITH_BODY({\
      tdc_tbd_constructor_call(#T_INSTANCE);\
      TDC_LINT_UNINITIALIZED_MEMBER\
    })\
    void T_INSTANCE::operator = (const T_INSTANCE& value_)\
    WITH_BODY({\
      set_value (value_);\
      TDC_LINT_UNASSIGNED_MEMBER\
      TDC_LINT_NO_SELFASSIGN_TEST\
    })\
    M_TDC_NOT_##IN_CLASS(T_INTERFACE_* T_INSTANCE::operator -> ()\
    {\
      return (T_INTERFACE*)get_navigation();\
    })\
    T_INSTANCE::T_INSTANCE (const T_INSTANCE& value_)\
    WITH_BODY({\
      construct_from_instance (value_, new_##T_HANDLE_);\
      TDC_LINT_UNCOPIED_BASE\
      TDC_LINT_UNINITIALIZED_MEMBER\
    })\
    T_INSTANCE::T_INSTANCE (T_INSTANCE (*f) ())\
    WITH_BODY({\
      T_INSTANCE value_ = f ();\
      construct_from_instance (value_, new_##T_HANDLE_);\
      TDC_LINT_UNINITIALIZED_MEMBER\
    })\
    M_TDC_ADDITIONAL_CALL_FUNCTION_BASE(IN_CLASS, WITH_BODY, T_INSTANCE, T_INSTANCE, T_INSTANCE)

#define M_TDC_INSTANCE_ADDITIONAL_MAIN(IN_CLASS, WITH_BODY, SHORT_NAME, PREFIXED_SHORT_NAME, BASE)\
  public:\
    void operator = (const T_TDC_INTERFACE_##BASE##_BASE& value_)\
    {\
      set_main_value (&value_);\
    }\
    void operator = (const T_TDC_INSTANCE_##BASE##_BASE& value_)\
    {\
      set_main_value (&value_);\
    }\
    T_##SHORT_NAME (const T_TDC_INTERFACE_##BASE##_BASE& value_)\
    {\
      construct_main_value (&value_, new_T_TDC_HANDLE_##PREFIXED_SHORT_NAME);\
    }\
    T_##SHORT_NAME (const T_TDC_INSTANCE_##BASE##_BASE& value_)\
    {\
      construct_main_value (&value_, new_T_TDC_HANDLE_##PREFIXED_SHORT_NAME);\
    }\
  M_TDC_INSTANCE_ADDITIONAL_BASE (IN_CLASS, WITH_BODY, T_TDC_INTERFACE_##PREFIXED_SHORT_NAME, T_##SHORT_NAME, T_TDC_HANDLE_##PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_PART_ADDITIONAL(IN_CLASS, WITH_BODY, T_INTERFACE, T_INSTANCE, T_DESCRIPTOR, T_HANDLE)\
  public:\
    T_INSTANCE (const T_TDC_ACTION& action_)\
    {\
      construct_from_action (action_, new_##T_HANDLE);\
    }\
    T_TDC_ACTION operator = (const T_TDC_ACTION& action_)\
    {\
      set_action (action_);\
      return action_;\
    }\
  M_TDC_INSTANCE_ADDITIONAL_BASE(IN_CLASS, WITH_BODY, T_INTERFACE, T_INSTANCE, T_HANDLE)

#define M_TDC_INSTANCE_COMP_ADDITIONAL(IN_CLASS, WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_PART_ADDITIONAL (IN_CLASS, WITH_BODY, T_TDC_INTERFACE_##PREFIXED_SHORT_NAME, T_##SHORT_NAME, T_TDC_DESCRIPTOR_##PREFIXED_SHORT_NAME, T_TDC_HANDLE_##PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_ENUM_ADDITIONAL(IN_CLASS, WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)\
    T_##SHORT_NAME (int value_)\
    {\
      construct_from_number (value_, new_T_TDC_DESCRIPTOR_##PREFIXED_SHORT_NAME);\
    }\
    void operator = (int value_)\
    {\
      set_descriptor_value (value_);\
    }\
    bool operator == (T_TDC_ENUM_##PREFIXED_SHORT_NAME value_)\
    {\
      return cmp_descriptor_value (value_);\
    }\
    operator T_TDC_ENUM_##PREFIXED_SHORT_NAME ()\
    {\
      return (T_TDC_ENUM_##PREFIXED_SHORT_NAME) get_descriptor_value ();\
    }\
  M_TDC_INSTANCE_PART_ADDITIONAL (IN_CLASS, WITH_BODY, T_TDC_INTERFACE_##PREFIXED_SHORT_NAME, T_##SHORT_NAME, T_TDC_DESCRIPTOR_##PREFIXED_SHORT_NAME, T_TDC_DESCRIPTOR_##PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_INT_ADDITIONAL(SHORT_NAME)\
    T_##SHORT_NAME (int value_)\
    {\
      construct_from_number (value_, new_T_TDC_DESCRIPTOR_##SHORT_NAME);\
    }\
    void operator = (int value_)\
    {\
      set_descriptor_value (value_);\
    }\
  M_TDC_INSTANCE_PART_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, T_TDC_INTERFACE_##SHORT_NAME, T_##SHORT_NAME, T_TDC_DESCRIPTOR_##SHORT_NAME, T_TDC_DESCRIPTOR_##SHORT_NAME)

//-----

#define M_TDC_INSTANCE_PRIM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, PRIMITIVE)

#define M_TDC_INSTANCE_PSTRUCT_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_COMP_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_PUNION_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_COMP_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_SDU_ADDITIONAL(SHORT_NAME)\
  M_TDC_INSTANCE_COMP_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,SHORT_NAME)

#define M_TDC_INSTANCE_PENUM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_ENUM_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

//-----

#define M_TDC_INSTANCE_MSG_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, MESSAGE)

#define M_TDC_INSTANCE_MSTRUCT_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_COMP_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_MUNION_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_COMP_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_MENUM_ADDITIONAL(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_ENUM_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

//-----

#define M_TDC_INSTANCE_XXX_PRIMITIVE_UNION_ADDITIONAL(SAP,PREFIXED_SAP)\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, SAP,PREFIXED_SAP, PRIMITIVE)

#define M_TDC_INSTANCE_XXX_MESSAGE_UNION_ADDITIONAL(MSG,PREFIXED_MSG)\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, MSG,PREFIXED_MSG, MESSAGE)

#define M_TDC_INSTANCE_PRIMITIVE_ADDITIONAL()\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, PRIMITIVE_UNION,PRIMITIVE_UNION, PRIMITIVE)\
  static int call_tdc_initialize_primitive ();\

#define M_TDC_INSTANCE_MESSAGE_ADDITIONAL()\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITHOUT_BODY, MESSAGE_UNION,MESSAGE_UNION, MESSAGE)\
  static int call_tdc_initialize_message ();\

//-----

#define M_TDC_INSTANCE_PRIM_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, PRIMITIVE)

#define M_TDC_INSTANCE_PSTRUCT_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_COMP_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_PUNION_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_COMP_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_PENUM_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_ENUM_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

//-----

#define M_TDC_INSTANCE_MSG_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME, MESSAGE)

#define M_TDC_INSTANCE_MSTRUCT_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_COMP_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_MUNION_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_COMP_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

#define M_TDC_INSTANCE_MENUM_ADDITIONAL_INLINE(SHORT_NAME,PREFIXED_SHORT_NAME)\
  M_TDC_INSTANCE_ENUM_ADDITIONAL (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SHORT_NAME,PREFIXED_SHORT_NAME)

//-----

template<class T_INSTANCE,class T_HANDLE>class T_TDC_FORCE_LINK_INSTANCE_WORKER
{
  T_TDC_FORCE_LINK_INSTANCE_WORKER(){
    T_HANDLE::new_descriptor = T_INSTANCE::get_new_descriptor();
  }
};

template<class T_INSTANCE,class T_HANDLE>class T_TDC_FORCE_LINK_INSTANCE
{
  static T_TDC_FORCE_LINK_INSTANCE_WORKER<T_INSTANCE,T_HANDLE> worker; 
};
template<class T_INSTANCE,class T_HANDLE>T_TDC_FORCE_LINK_INSTANCE_WORKER<T_INSTANCE,T_HANDLE> T_TDC_FORCE_LINK_INSTANCE<T_INSTANCE,T_HANDLE>::worker;
 
#define M_TDC_XXX_PRIMITIVE_UNION_ADDITIONAL_INLINE_1(SAP,PREFIXED_SAP)\
  extern char set_new_descriptor_T_TDC_HANDLE_##SAP();\
  static char init_new_descriptor_T_TDC_HANDLE_##SAP = set_new_descriptor_T_TDC_HANDLE_##SAP();\

#define M_TDC_XXX_PRIMITIVE_UNION_ADDITIONAL_INLINE(SAP,PREFIXED_SAP)\

#define M_TDC_XXX_MESSAGE_UNION_ADDITIONAL_INLINE(SAP,PREFIXED_SAP)\

/*
#define M_TDC_XXX_PRIMITIVE_UNION_ADDITIONAL_INLINE(SAP,PREFIXED_SAP)\
  extern char set_new_descriptor_T_TDC_HANDLE_##SAP();\
  static char init_new_descriptor_T_TDC_HANDLE_##SAP = set_new_descriptor_T_TDC_HANDLE_##SAP();\
  
#define M_TDC_XXX_MESSAGE_UNION_ADDITIONAL_INLINE(SAP,PREFIXED_SAP)\
  extern char set_new_descriptor_T_TDC_HANDLE_##SAP();\
  static char init_new_descriptor_T_TDC_HANDLE_##SAP = set_new_descriptor_T_TDC_HANDLE_##SAP();\
*/
  
#define M_TDC_INSTANCE_XXX_PRIMITIVE_UNION_ADDITIONAL_INLINE(SAP,PREFIXED_SAP)\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITH_BODY, SAP,PREFIXED_SAP, PRIMITIVE)\
  T_TDC_NEW_DESCRIPTOR get_new_descriptor()\
  {\
    extern T_TDC_DESCRIPTOR_BASE* new_T_TDC_DESCRIPTOR_##SAP();\
    return new_T_TDC_DESCRIPTOR_##SAP;\
  }\

#define M_TDC_INSTANCE_XXX_MESSAGE_UNION_ADDITIONAL_INLINE(MSG,PREFIXED_MSG)\
  M_TDC_INSTANCE_ADDITIONAL_MAIN (M_TDC_IN_CLASS, M_TDC_WITH_BODY, MSG,PREFIXED_MSG, MESSAGE)\
  T_TDC_NEW_DESCRIPTOR get_new_descriptor()\
  {\
    extern T_TDC_DESCRIPTOR_BASE* new_T_TDC_DESCRIPTOR_##MSG();\
    return new_T_TDC_DESCRIPTOR_##MSG;\
  }\

//\}

//-----

#ifdef TDC_TYPE_NAME_COMPLETE
#define M_TDC_TYPE_NAME_COMPLETE TDC_DOT_COMPLETE_HIDE({ T_TDC_TYPE_NAME type_name = {0}; return type_name; })
#else 
#define M_TDC_TYPE_NAME_COMPLETE ;
#endif  

#endif //DOT_COMPLETE_DEFINES

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1J()
  {
    T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
    s.
      i;
    s->
      i0;
}
#else
#endif

//============================================================================
// implementation classes
//============================================================================

typedef bool T_TDC_TABLE_KIND; 

enum T_TDC_ACTION_ENUM
{
  TDC_ACTION_UNKNOWN,
                              //  MANDATORY-SEND    OPTIONAL-SEND   MANDATORY-WAIT    OPTIONAL-WAIT
  TDC_ACTION_SKIP,            //  ERROR             ABSENT          SKIPED            SKIPED
  TDC_ACTION_SHOW,            //  ERROR             ABSENT          SKIPED            SKIPED
  TDC_ACTION_FORBID,          //  ERROR             ABSENT          ERROR             CHECKED
  TDC_ACTION_REQUIRE,         //  ERROR             ERROR           ERROR             CHECKED PRESENT
  TDC_ACTION_MESSAGE,
  TDC_ACTION_DESTROYED,
  TDC_ACTION_SKIP_TO_END,
  TDC_ACTION_SET = 0x10,      //  PRESENT           PRESENT         CHECKED           CHECKED VALUE
  TDC_ACTION_HAS_VALUE = 0x20,
  TDC_ACTION_HAS_REF = 0x40,
  TDC_ACTION_IS_IMPLICIT = 0x80,
  TDC_ACTION_SET_VALUE = TDC_ACTION_SET + TDC_ACTION_HAS_VALUE,
  TDC_ACTION_SET_REF = TDC_ACTION_SET + TDC_ACTION_HAS_REF,   
  TDC_ACTION_CHILD_TOUCHED = TDC_ACTION_SET + TDC_ACTION_HAS_REF + TDC_ACTION_IS_IMPLICIT,    
  TDC_ACTION_DEFAULT = TDC_ACTION_UNKNOWN
};

/** allow one optional extra set of () on action member functions, first set is suplyed by macro
 *  second set might be suplyed by the user, this will have no effect on behaviour of code 
 */
struct T_TDC_ASSIGN_ACTION
{
  void operator ()();
};

extern T_TDC_ASSIGN_ACTION tdc_assign_action;

struct T_TDC_ACTION
{
  //create a waper class for T_TDC_ACTION_ENUM so that the action constants can not be converted to integers
  T_TDC_ACTION_ENUM action;
  T_TDC_ACTION (T_TDC_ACTION_ENUM action_):
      action (action_)
  {
    //nothing
  }
  T_TDC_ACTION ();
};

struct T_TDC_CREATE_ACTION:T_TDC_ACTION
{
  /* This type is returned by constant action function, this allow the user to optional
  omit the "()" after the action name
    Src code   After macro expantion   Function returns            After implicit type convting
    ========   ====================    ========================    =====================
    _skip      tdc_skip()              SKIP::T_TDC_CREATE_ACTION   SKIP::T_TDC_ACTION
    _skip()    tdc_skip()()            SKIP::T_TDC_ACTION          SKIP::T_TDC_ACTION
    _skip()()  tdc_skip()()()          <error T_TDC_ACTION have no operator ()>
  */
  T_TDC_CREATE_ACTION ();
  T_TDC_ACTION operator()();
};

extern T_TDC_CREATE_ACTION SKIP;
extern T_TDC_CREATE_ACTION SHOW;
extern T_TDC_CREATE_ACTION FORBID;
extern T_TDC_CREATE_ACTION REQUIRE;
extern T_TDC_CREATE_ACTION SKIP_TO_END;

extern T_TDC_CREATE_ACTION tdc_skip(); 
extern T_TDC_CREATE_ACTION tdc_show();
extern T_TDC_CREATE_ACTION tdc_forbid(); 
extern T_TDC_CREATE_ACTION tdc_require();
extern T_TDC_CREATE_ACTION tdc_skip_to_end();

//----------------------------------------------------------------------------

enum T_TDC_EVENT_ENUM
{
	TDC_EVENT_SEND,
	TDC_EVENT_AWAIT
};

enum T_TDC_IS_ENUM
{
	TDC_IS_UNKNOWN = 0,
	TDC_IS_VAR = 0x001,
	TDC_IS_STRUCT = 0x002,
	TDC_IS_SDU = 0x004,
	TDC_IS_P = 0x008,
	TDC_IS_M = 0x010,
	TDC_IS_COMP = 0x20,
	TDC_IS_ARRAY = 0x40,
	TDC_IS_UNION = 0x80,
	TDC_IS_MESSAGE_UNION = 0x100,
	TDC_IS_PRIMITIVE_UNION = 0x200,
	TDC_IS_MSG = 0x400,
	TDC_IS_PRIM = 0x800,
	TDC_IS_XXX_MESSAGE_UNION = 0x1000,
	TDC_IS_XXX_PRIMITIVE_UNION = 0x2000,
	TDC_IS_POINTER = 0x4000,
	TDC_IS_PVAR = TDC_IS_VAR | TDC_IS_P,
	TDC_IS_MVAR = TDC_IS_VAR | TDC_IS_M
};

//----------------------------------------------------------------------------

struct T_TDC_HANDLE_BASE;
struct T_TDC_DESCRIPTOR_BASE;
struct T_TDC_INSTANCE_MAIN_BASE;
struct T_TDC_INTERFACE_BASE;
struct T_TDC_INSTANCE_BASE;
struct T_TDC_PATH;
struct T_TDC_PATH_TEXT;

typedef T_TDC_INTERFACE_BASE* (*T_TDC_NEW_INTERFACE)();
typedef T_TDC_HANDLE_BASE* (*T_TDC_NEW_HANDLE)();
typedef T_TDC_DESCRIPTOR_BASE* (*T_TDC_NEW_DESCRIPTOR)();

struct T_PRIMITIVE_UNION;
struct T_TDC_INTERFACE_PRIMITIVE_UNION;
struct T_TDC_DESCRIPTOR_PRIMITIVE_UNION;

struct T_MESSAGE_UNION;
struct T_TDC_INTERFACE_MESSAGE_UNION;
struct T_TDC_DESCRIPTOR_MESSAGE_UNION;

typedef signed long T_TDC_INT_S32;
typedef signed short T_TDC_INT_S16;
typedef signed char T_TDC_INT_S8;
typedef unsigned long T_TDC_INT_U32;
typedef unsigned short T_TDC_INT_U16;
typedef unsigned char T_TDC_INT_U8;
typedef char CHAR;

//----------------------------------------------------------------------------
inline unsigned long num_elements(const unsigned char* array)
{
  return strlen((const char*)array);
}

inline unsigned long num_elements(const signed char* array)
{
  return strlen((const char*)array);
}


//----------------------------------------------------------------------------

enum T_TDC_RW_MODE
{
	TDC_RW_MODE_READ,
	TDC_RW_MODE_WRITE
};

struct T_TDC_HANDLE_BASE
{
  friend struct T_TDC_COPY;
protected:
  T_TDC_ACTION_ENUM implement_get_action () const;
  void implement_set_action (T_TDC_ACTION_ENUM action_);
  T_TDC_DESCRIPTOR_BASE* implement_get_descriptor () const;
  void implement_set_descriptor (T_TDC_DESCRIPTOR_BASE* descriptor_);
  long implement_get_value () const;
  void implement_set_value (long value_);
  friend void add_name_info (T_TDC_HANDLE_BASE* descriptor_ref, int level, int parent, int index);
  T_TDC_PATH_TEXT path_text();
  virtual T_TDC_DESCRIPTOR_BASE* implement_new_descriptor () const;
  static T_TDC_DESCRIPTOR_BASE* call_implement_new_descriptor(T_TDC_NEW_DESCRIPTOR new_descriptor);  
public:
  T_TDC_DESCRIPTOR_BASE* make_descriptor ();
  T_TDC_DESCRIPTOR_BASE* make_array_descriptor (T_TDC_NEW_HANDLE new_element_handle);
  virtual char* get_name () const
  TDC_PURE_BODY({
    //TDC_ERROR();
    return 0;
  })
  virtual long get_sizeof () = 0
  {
    return sizeof *this;
  }
  T_TDC_HANDLE_BASE ();
  ~T_TDC_HANDLE_BASE ();
  T_TDC_HANDLE_BASE* get_element (int index)const;
  T_TDC_HANDLE_BASE* get_union_element ();
  void destroy ();
  T_TDC_DESCRIPTOR_BASE* get_descriptor()const;
  T_TDC_ACTION_ENUM get_action()const;
  long get_value()const;
  int get_ctrl()const;
  void clear();
  void set_descriptor_value(long value_);
  virtual unsigned get_sizeof_target ();
  virtual T_TDC_IS_ENUM is ();
};

#ifdef TDC_USE_ALLOC_DEBUG_COUNTER
struct T_TDC_ALLOC_DEBUG_COUNTER 
{
	int count;
	int max_count;
	int alloc_count;
	int free_count;
	bool alloc[1000];
	T_TDC_ALLOC_DEBUG_COUNTER ()
	{
    /* nothing here should allways be static allocated and there for cleared, 
      futher more it is used before construction so intializing here will be fatal */
	}
	~T_TDC_ALLOC_DEBUG_COUNTER ()
	{
		for (int first_not_freed = 0; !alloc[first_not_freed]; first_not_freed++)
			TDC_ASSERT (first_not_freed < sizeof alloc);
		TDC_ASSERT (count==0); // some where we have a leak
	}
};

template<class T>
struct T_TDC_ALLOC_DEBUG 
{
	static T_TDC_ALLOC_DEBUG_COUNTER counter;
	int alloc_count;
	T_TDC_ALLOC_DEBUG_COUNTER& counter_;
	T_TDC_ALLOC_DEBUG ():
		counter_ (counter)
	{
	  counter.count++;
	  alloc_count = counter.alloc_count++;
    if (alloc_count < sizeof counter.alloc)
	    counter.alloc[alloc_count]=true;
	  if (counter.max_count < counter.count)
		  counter.max_count = counter.count;
	}
	~T_TDC_ALLOC_DEBUG ()
	{
    if (alloc_count < sizeof counter.alloc)
		  counter.alloc[alloc_count]=false;
		TDC_ASSERT (counter.count > 0); // deallocating more than ever allocated
		counter.free_count++;
		counter.count--;
	}
};

template<class T>T_TDC_ALLOC_DEBUG_COUNTER T_TDC_ALLOC_DEBUG<T>::counter;
#endif

enum { 
	TDC_CTRL_DEFAULT = 0,
	TDC_CTRL_NO_DESCRIPTOR = -1,
	TDC_CTRL_NOT_AN_UNION = -2,
	TDC_REF_COUNT_ELEMENTS_DESTROYED = -2,
	TDC_DESCRIPTOR_DESTROYED_VALUE = -4,
	TDC_DUMMY_TAP_HANDLE = -16
};

#define TDC_DESCRIPTOR_DESTROYED ((T_TDC_DESCRIPTOR_BASE*)TDC_DESCRIPTOR_DESTROYED_VALUE)

struct T_TDC_DESCRIPTOR_BASE
#ifdef TDC_USE_ALLOC_DEBUG_COUNTER
	:T_TDC_ALLOC_DEBUG<T_TDC_DESCRIPTOR_BASE>
#endif
{
private:
  friend struct T_TDC_COPY;
	friend void unsubscribe (T_TDC_DESCRIPTOR_BASE* descriptor);
	int ref_count;
	virtual long get_sizeof () 
	TDC_PURE_BODY(
		return 0;
	)

	void destroy_elements ();
protected:
	T_TDC_HANDLE_BASE* implement_get_element (unsigned index, char *first_element);
	T_TDC_HANDLE_BASE* implement_get_union_element (unsigned index, int& ctrl);
public:
	void *operator new (size_t size);//TODO: operator new (size_t size); + eliminate T_TDC_HANDLE_BASE::T_TDC_HANDLE_BASE i.e. todo T_TDC_HANDLE_BASE::operator new (size_t size);
	T_TDC_PATH_TEXT path_text();
	virtual int read_ctrl () const
	{
		return TDC_CTRL_NOT_AN_UNION;
	}
  void set_skip_to_end_from_action (T_TDC_ACTION_ENUM action);
	virtual void set_skip_to_end (bool skip_to_end);
	virtual bool get_skip_to_end () const;
  virtual T_TDC_HANDLE_BASE* make_element (unsigned index);
	virtual T_TDC_HANDLE_BASE* get_element (unsigned index);
	virtual int get_tap_handle ();
	T_TDC_DESCRIPTOR_BASE ();
	virtual ~T_TDC_DESCRIPTOR_BASE ();
	virtual char* get_name () const
	TDC_PURE_BODY(
		return 0;
	)
	virtual T_TDC_IS_ENUM is ()
	{
		//TODO: TDC_ERROR() here and mov is function to distinct base classes;
		return TDC_IS_UNKNOWN;
	}
};

union T_TDC_BASIC_TYPES 
{
	unsigned long u32;
	signed long s32;
	signed int s16;
	signed char s8;
	char c[4];
};

//----------------------------------------------------------------------------

struct T_TDC_INTERFACE_BASE
{
private:
	void* operator &(); /* this member function is only here to prevent the user from taking the address of 
					an interface, which make no sence to the user as it gives the address of
					some internal tdc navigation data */
#ifdef LINT	//TODO: introduce for VC too
  const  void* operator &() const; /* but we don't want ripling errors from LINT
          the ripling errors comes from the fact that LINT seems to have a more correct
          interpretion of where to use the overloaded function and where to truely take
          the address*/
#endif
protected:
	T_TDC_LEVEL level[1];
	T_TDC_ASSIGN_ACTION set_action (const T_TDC_ACTION& action_);
	T_TDC_INTERFACE_BASE ()
	{
		//nothing
	}
public:
	T_TDC_PATH* implement_make_descriptor_ref (T_TDC_RW_MODE rw_mode);
	T_TDC_PATH* make_descriptor_ref (T_TDC_RW_MODE rw_mode);
	const T_TDC_PATH* make_descriptor_ref (T_TDC_RW_MODE rw_mode) const;
	void set_descriptor_value (long value_); 
	//bool cmp_descriptor_value (long value_); 
	void copy_instance (const T_TDC_INSTANCE_BASE * value_);
	void copy_interface (const T_TDC_INTERFACE_BASE * value_);
};

struct T_TDC_INTERFACE_MAIN_BASE
#if DOT_COMPLETE_MEMBERS
#else
	TDC_DOT_COMPLETE_HIDE(:T_TDC_INTERFACE_BASE)
#endif
{
#if DOT_COMPLETE_MEMBERS
#else
protected:
	void set_main_value (const T_TDC_INTERFACE_MAIN_BASE* value_);
	void set_main_value (const T_TDC_INSTANCE_MAIN_BASE* value_);
	T_TDC_INTERFACE_BASE* get_element_navigation(T_TDC_NEW_INTERFACE new_element_interface);
	/*T_TDC_INTERFACE_MAIN_BASE ()
	{
		tdc_level++;
	}*/
#endif
};

struct T_TDC_INTERFACE_REF_BASE
#if DOT_COMPLETE_MEMBERS
#else
	TDC_DOT_COMPLETE_HIDE(:T_TDC_INTERFACE_BASE)
#endif
{
#if DOT_COMPLETE_MEMBERS
	T_TDC_ASSIGN_ACTION _skip () {} /* you can use '= SKIP' as well */
	T_TDC_ASSIGN_ACTION _show () {} /* you can use '= SHOW' as well */
	T_TDC_ASSIGN_ACTION _forbid () {} /* you can use '= FORBID' as well */
	T_TDC_ASSIGN_ACTION _require () {} /* you can use '= REQUIRE' as well */
#else//DOT_COMPLETE_MEMBERS
	T_TDC_INTERFACE_REF_BASE ()
	{
		level[0] = 'A'+tdc_level;
	}
	TDC_DOT_COMPLETE_HIDE(T_TDC_ASSIGN_ACTION tdc_skip ();) /* you can use '= SKIP' as well */
	TDC_DOT_COMPLETE_HIDE(T_TDC_ASSIGN_ACTION tdc_show ();) /* you can use '= SHOW' as well */
	TDC_DOT_COMPLETE_HIDE(T_TDC_ASSIGN_ACTION tdc_forbid ();) /* you can use '= FORBID' as well */
	TDC_DOT_COMPLETE_HIDE(T_TDC_ASSIGN_ACTION tdc_require ();) /* you can use '= REQUIRE' as well */
#endif//DOT_COMPLETE_MEMBERS
};

struct T_TDC_INTERFACE_VAR_BASE
#if DOT_COMPLETE_MEMBERS
#else
	TDC_DOT_COMPLETE_HIDE(:T_TDC_INTERFACE_BASE)
#endif
{
#if DOT_COMPLETE_MEMBERS
	T_TDC_ASSIGN_ACTION _skip () {} /* you can use '= SKIP' as well */
	T_TDC_ASSIGN_ACTION _show () {} /* you can use '= SHOW' as well */
	T_TDC_ASSIGN_ACTION _forbid () {} /* you can use '= FORBID' as well */
	T_TDC_ASSIGN_ACTION _require () {} /* you can use '= REQUIRE' as well */
#else//DOT_COMPLETE_MEMBERS
	T_TDC_INTERFACE_VAR_BASE ()
	{
		level[0] = 'A'+tdc_level;
	}
	TDC_DOT_COMPLETE_HIDE(T_TDC_ASSIGN_ACTION tdc_skip ();) /* you can use '= SKIP' as well */
	TDC_DOT_COMPLETE_HIDE(T_TDC_ASSIGN_ACTION tdc_show ();) /* you can use '= SHOW' as well */
	TDC_DOT_COMPLETE_HIDE(T_TDC_ASSIGN_ACTION tdc_forbid ();) /* you can use '= FORBID' as well */
	TDC_DOT_COMPLETE_HIDE(T_TDC_ASSIGN_ACTION tdc_require ();) /* you can use '= REQUIRE' as well */
#endif//DOT_COMPLETE_MEMBERS
};

//----------------------------------------------------------------------------

#if TDC_DEBUG_DOT_COMPLETE
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1K()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
  s->
    i0;
}
#else
#endif

//----------------------------------------------------------------------------
/** \defgroup PATH PATH
 *  A set of classes and functions that is used for generating the path of 
 *  an element when generating warning or error messages.
 *  TODO: need more work
 */
//\{

struct T_TDC_PATH
#ifdef TDC_USE_ALLOC_DEBUG_COUNTER
	:T_TDC_ALLOC_DEBUG<T_TDC_PATH>
#endif
{
	T_TDC_HANDLE_BASE* handle;
	T_TDC_DESCRIPTOR_BASE* cahed_descriptor;
	int indexes_count;
  union{
    int *indexes;
    struct T_WATCH_INDEXES{
      int i0,i1,i2,i3,i4,i5,i6,i7,i8,i9;
    }*watch_indexes;
  };
	T_TDC_INTERFACE_BASE* pattern;
	T_TDC_PATH* next;
	T_TDC_PATH (T_TDC_HANDLE_BASE* handle_);
	~T_TDC_PATH ();
	void add_index(T_TDC_DESCRIPTOR_BASE* descriptor, int index);
	void add_to_cashed_path_list ();
	T_TDC_HANDLE_BASE* get_leaf();
	T_TDC_PATH_TEXT get_path_text ();
	T_TDC_PATH* duplicate()const;
};

//----------------------------------------------------------------------------

#if TDC_DEBUG_DOT_COMPLETE
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1L()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
  s->
    i0;

}
#else
#endif

//----------------------------------------------------------------------------

struct const_T_TDC_COPY
{
	T_TDC_HANDLE_BASE* descriptor_ref;
	const T_TDC_PATH* path;
	const_T_TDC_COPY (const T_TDC_PATH* path_):
		descriptor_ref (const_cast<T_TDC_PATH*>(path_)->get_leaf()),
		path (path_)
	{
	}
	const_T_TDC_COPY (const T_TDC_HANDLE_BASE* descriptor_ref_):
		descriptor_ref (const_cast<T_TDC_HANDLE_BASE*>(descriptor_ref_)),
		path (0)
	{
	}
	const_T_TDC_COPY (const const_T_TDC_COPY& src):
		descriptor_ref (src.descriptor_ref),
		path (0)
	{
		if (src.path)
			path = src.path->duplicate();
	}
	void operator = (const const_T_TDC_COPY& src)
	{
		descriptor_ref = src.descriptor_ref;
		if (path)
			delete path;
		if (src.path)
			path = src.path->duplicate();
		else
			path = 0;
	}
	const T_TDC_HANDLE_BASE* operator->()
	{
		return descriptor_ref;
	}
	operator const T_TDC_HANDLE_BASE*()const
	{
		return descriptor_ref;
	}
	~const_T_TDC_COPY ()
	{
		if (path)
			delete path;
	}
	T_TDC_PATH_TEXT path_text ();
	T_TDC_ACTION_ENUM get_action () const
	{
		return descriptor_ref->get_action();
	}
	bool has_value () const
	{
		return (descriptor_ref->get_action() & TDC_ACTION_HAS_VALUE) != 0;
	}
	int get_value () const
	{
		return descriptor_ref->get_value();
	}
};

struct T_TDC_COPY:const_T_TDC_COPY
{
  friend struct T_TDC_COPY;
  T_TDC_COPY (T_TDC_PATH* path_):
    const_T_TDC_COPY (path_)
  {
  }
  T_TDC_COPY (T_TDC_HANDLE_BASE* descriptor_ref_):
    const_T_TDC_COPY (descriptor_ref_)
  {
  }
  T_TDC_HANDLE_BASE* operator->()
  {
    return descriptor_ref;
  }
  operator T_TDC_HANDLE_BASE*()const
  {
    return descriptor_ref;
  }
  void copy_descriptor_ref (const const_T_TDC_COPY& value_);
  void copy_number (long value_);
  T_TDC_ASSIGN_ACTION copy_action (T_TDC_ACTION_ENUM action_);
private:
  void set_action_and_value (T_TDC_ACTION_ENUM action_, long value_=0xFCFCFCFC);
};

//----------------------------------------------------------------------------

/** \struct T_TDC_PATH_TEXT 
    T_TDC_PATH_TEXT is used to return a string on the heap that will auto dealloc
    <pre>
		e.g.
		
		T_TDC_PATH_TEXT f()
		{
			return strdup ("...");
		}

		void g()
		{
			printf ("%s", f()); // free of string allocated in f() will happen automatic
								// when ~T_TDC_PATH_TEXT is called at the end of sentence (at ';')
		}
    </pre>
*/
struct T_TDC_PATH_TEXT
{
	char* str;
	T_TDC_PATH_TEXT(char* str_);
	~T_TDC_PATH_TEXT();
};

//\}

//----------------------------------------------------------------------------

#if TDC_DEBUG_DOT_COMPLETE
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1N()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
  s->
    i0;

}
#else
#endif

//============================================================================
/// \defgroup T_TDC_INTERFACE
//\{

struct T_TDC_INTERFACE_REF:T_TDC_INTERFACE_REF_BASE
{
#if DOT_COMPLETE_MEMBERS
#else
	T_TDC_INTERFACE_REF ()
	{
		tdc_level++;
	}
#endif
};

struct T_TDC_INTERFACE_VAR:T_TDC_INTERFACE_VAR_BASE
{
#if DOT_COMPLETE_MEMBERS
#else
	T_TDC_INTERFACE_VAR ()
	{
		//nothing
	}
#endif
};

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1O
{
  int i1O;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1O()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1O s10;
  s10->
    i0;
}
#else
#endif

//----------------------------------------------------------------------------

#if DOT_COMPLETE
#define TDC_HIDDEN_LEVEL_DEF
#define TDC_HIDDEN_LEVEL(T_BASE)
#define TDC_HIDDEN_LEVEL_MAIN_DEF
#define TDC_HIDDEN_LEVEL_MAIN(T_BASE)
struct T_TDC_INTERFACE_POINTER_MAIN_BASE:T_TDC_INTERFACE_REF_BASE
{
};
#else
#define TDC_HIDDEN_LEVEL_DEF 
#define TDC_HIDDEN_LEVEL 
#define TDC_HIDDEN_LEVEL_MAIN_DEF , class T_TDC_INTERFACE_POINTER_MAIN_BASE = T_TDC_INTERFACE_POINTER_MAIN_BASE_DEFAULT
#define TDC_HIDDEN_LEVEL_MAIN(T_BASE) , T_BASE
#endif

struct T_TDC_INTERFACE_PRIMITIVE_BASE:T_TDC_INTERFACE_MAIN_BASE
{
};

struct T_TDC_INTERFACE_MESSAGE_BASE:T_TDC_INTERFACE_MAIN_BASE
{
};

//template<class T TDC_HIDDEN_LEVEL_MAIN_DEF > struct T_TDC_INTERFACE_POINTER_MAIN:T_TDC_INTERFACE_POINTER_BASE
template<class T TDC_HIDDEN_LEVEL_MAIN_DEF > struct T_TDC_INTERFACE_POINTER_MAIN:T_TDC_INTERFACE_POINTER_MAIN_BASE
{
	T_TDC_INTERFACE_POINTER_MAIN ()
	{
		level[0] = 'A'+tdc_level;
	}
#if  DOT_COMPLETE_MEMBERS
#else //DOT_COMPLETE_MEMBERS
	operator = (const T& t) 
	{
		tdc_tbd_pointer_assignment_error();
		//get_navigation()->_set(t);
		return 0;
	}
#endif //DOT_COMPLETE_MEMBERS
	/*operator = (const T_TDC_INSTANCE& t) 
	{
		get_navigation()->_set(t);
	}*/
	T* operator-> (){
		return (T*)get_element_navigation(new_element_interface);		
	}
	/*operator const T&()
	{
		return *get_navigation();
	}*/
private:
#if  DOT_COMPLETE_MEMBERS
#else //DOT_COMPLETE_MEMBERS
	static T_TDC_INTERFACE_BASE* new_element_interface()
	{
		return new T;
	}
#endif //DOT_COMPLETE_MEMBERS
};	

template<class T> struct T_ARRAY;

struct T_TDC_INTERFACE_ARRAY_BASE
  :T_TDC_INTERFACE_REF_BASE
{
#if DOT_COMPLETE
#else
protected:
	T_TDC_INTERFACE_BASE* get_element_navigation(int index, T_TDC_NEW_INTERFACE new_element_interface, T_TDC_NEW_HANDLE new_handle);
	void copy_basic_array (const T_TDC_INT_U8* array, unsigned size, void *address);
	void copy_basic_array (const T_TDC_INT_S8* array, unsigned size, void *address);
	void copy_basic_array (const T_TDC_INT_U16* array, unsigned size, void *address);
	void copy_basic_array (const T_TDC_INT_S16* array, unsigned size, void *address);
	void copy_basic_array (const T_TDC_INT_U32* array, unsigned size, void *address);
	void copy_basic_array (const T_TDC_INT_S32* array, unsigned size, void *address);
#endif
};

/*
**	TODO: Function header (this is a comment for an bug fix)		
**
**	I would expect dropdown to work for this
**		template<class T, int MAX_INDEX   > struct T_TDC_INTERFACE_ARRAY:T_TDC_INTERFACE_POINTER_BASE_DEFAULT
**										^^
**	and to fail for this
**		template<class T, int MAX_INDEX, int DUMMY=0 > struct T_TDC_INTERFACE_ARRAY:T_TDC_INTERFACE_POINTER_BASE_DEFAULT
**										^^^^^^^^^^^^^
**										
**	but it seams to be the otherway around (see more info in tcd_test.cpp::test_1000())
*/

#if DOT_COMPLETE
template<class T /*T_INTERFACE*/, int MAX_INDEX = 1, int DUMMY=0> struct T_TDC_INTERFACE_ARRAY
#else
template<class T /*T_INTERFACE*/, int MAX_INDEX = 1> struct T_TDC_INTERFACE_ARRAY
//template<class T /*T_INTERFACE*/> struct T_TDC_INTERFACE_ARRAY
#endif
	:T_TDC_INTERFACE_ARRAY_BASE
{
public:
	T& operator [] (int index)//HINT: blah
  {
		return *(T*)get_element_navigation (index, new_element_interface, T::implement_new_handle);		
	}
	T* operator-> ()//HINT: blah
  {
		return (T*)get_element_navigation (0, new_element_interface, T::implement_new_handle);		
	}
#if  DOT_COMPLETE_MEMBERS
#else//DOT_COMPLETE_MEMBERS
	T_TDC_INTERFACE_ARRAY ()
	{
		//nothing
	}
	void assign (const T_TDC_INT_U8* array, unsigned size, void *address)
	{
		copy_basic_array (array, size, address);
	}
	void assign (const T_TDC_INT_S8* array, unsigned size, void *address)
	{
		copy_basic_array (array, size, address);
	}
	void assign (const T_TDC_INT_U16* array, unsigned size, void *address)
	{
		copy_basic_array (array, size, address);
	}
	void assign (const T_TDC_INT_S16* array, unsigned size, void *address)
	{
		copy_basic_array (array, size, address);
	}
	void assign (const T_TDC_INT_U32* array, unsigned size, void *address)
	{
		copy_basic_array (array, size, address);
	}
	void assign (const T_TDC_INT_S32* array, unsigned size, void *address)
	{
		copy_basic_array (array, size, address);
	}
	void assign (const T_TDC_ACTION& action_, unsigned size, void *address)
	{
    set_action(action_);
	}
	/*template <class U>
	void assign (const U* u, unsigned size)
	{
		//TDC_TBD();
	}*/
	template <class U>
	void assign (const T_ARRAY<U>& src, unsigned size, void *address)
	{
		T_TDC_COPY handle = make_descriptor_ref (TDC_RW_MODE_WRITE);
		handle.copy_descriptor_ref (src.handle);
	}
	void assign (const T* t, unsigned size, void *address)
	{
		//tdc_check_array_assignment(array, address);
		tdc_tbd_array_assignment_error();
	}
	void assign (const T::T_INSTANCE* array, unsigned size, void *address)
	{
		tdc_check_array_assignment(array, address);
		//copy_instance_array (array, size);
		T_TDC_COPY handle = make_descriptor_ref (TDC_RW_MODE_WRITE);
		T_TDC_HANDLE_BASE* descriptor_ref = handle.descriptor_ref;
		T_TDC_DESCRIPTOR_BASE* descriptor = descriptor_ref->make_descriptor ();
		T_TDC_HANDLE_BASE** elements = ((T_TDC_DESCRIPTOR_ARRAY_BASE*)descriptor)->get_elements();
		if (!*elements)
			*elements = T::implement_new_handle ();
		int count = size / sizeof *array;

		for (int i = 0; i < count; i++)
		{
			T_TDC_COPY dst_element = descriptor->make_element(i);
			const_T_TDC_COPY src_element = (T_TDC_HANDLE_BASE*) array[i].handle;
			dst_element.copy_descriptor_ref (src_element);
		}
	}
	template <class U>
	T_TDC_INTERFACE_ARRAY& operator = (const U& array) 
	{
		int size = sizeof array;			/* if you have error "illegal indirection" here the right hand operator can not be  converted to an array
											see next error message for the bad assignment */
		assign (array, size, (void*)&array);
		return *this;
	}
private:
	static T_TDC_INTERFACE_BASE* new_element_interface()
	{
		return new T;
	}
#endif//DOT_COMPLETE_MEMBERS
};	

/*
**	TODO: Function header (this is a comment for an bug fix)		
**
**	When we change T_TDC_INTERFACE_ARRAY  we have to change T_TDC_INTERFACE_POINTER too
**		template<class T TDC_HIDDEN_LEVEL_DEF > struct T_TDC_INTERFACE_POINTER:T_TDC_INTERFACE_ARRAY<T, 1 TDC_HIDDEN_LEVEL> 
*/

struct T_TDC_INTERFACE_POINTER_BASE:T_TDC_INTERFACE_REF_BASE
{
protected:
	T_TDC_INTERFACE_BASE* get_navigation(T_TDC_NEW_INTERFACE new_element_interface, T_TDC_NEW_HANDLE new_handle);
  T_TDC_INTERFACE_BASE* get_navigation(T_TDC_NEW_INTERFACE new_element_interface, T_TDC_NEW_HANDLE new_handle, int index);
};

template<class T_INTERFACE> struct T_TDC_INTERFACE_POINTER
#if DOT_COMPLETE
#else
	:T_TDC_INTERFACE_POINTER_BASE
#endif
{
	T_INTERFACE* operator-> ()
  {
		return (T_INTERFACE*)get_navigation (new_element_interface, T_INTERFACE::implement_new_handle);		
	}
#if  DOT_COMPLETE_MEMBERS
#else//DOT_COMPLETE_MEMBERS
	T_INTERFACE& operator [] (int index)
  {
		return *(T_INTERFACE*)get_navigation (new_element_interface, T_INTERFACE::implement_new_handle, index);		
	}
	static T_TDC_INTERFACE_BASE* new_element_interface()
	{
		return new T_INTERFACE;
	}
	/*T_TDC_INTERFACE_POINTER ()
	{
		//nothing
	}*/
#endif//DOT_COMPLETE_MEMBERS
};

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1P
{
  int i1P;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1P()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s.
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1P s1P;
  s1P->
    i0;
  T_TDC_INTERFACE_ARRAY<int> i;
i.assign(0,0);
  T_ARRAY<int> array;
  array.

}
#else
#endif

//----------------------------------------------------------------------------
// T_TDC_INTERFACE
//----------------------------------------------------------------------------

struct T_TDC_INTERFACE_PRIM_BASE:T_TDC_INTERFACE_PRIMITIVE_BASE
{
	T_TDC_INTERFACE_PRIM_BASE ()
	{
		level[0] = 'A'+tdc_level;
		tdc_level++;
	}
#if DOT_COMPLETE_MEMBERS//{{
	T_TDC_ASSIGN_ACTION _require () {} // you can use '= REQUIRE' as well
#else//}DOT_COMPLETE_MEMBERS{
	T_TDC_ASSIGN_ACTION tdc_require() // you can use '= REQUIRE' as well
	{
		return set_action (REQUIRE);
	}
#endif//}}DOT_COMPLETE_MEMBERS
};

struct T_TDC_INTERFACE_PSTRUCT_BASE:T_TDC_INTERFACE_REF
{
};

struct T_TDC_INTERFACE_PUNION_BASE:T_TDC_INTERFACE_REF
{
};

struct T_TDC_INTERFACE_SDU_BASE:T_TDC_INTERFACE_REF
{
};

struct T_TDC_INTERFACE_PENUM_BASE:T_TDC_INTERFACE_VAR
{
};

struct T_TDC_INTERFACE_MSG_BASE:T_TDC_INTERFACE_MESSAGE_BASE
{
	T_TDC_INTERFACE_MSG_BASE ()
	{
		level[0] = 'A'+tdc_level;
		tdc_level++;
	}
#if DOT_COMPLETE_MEMBERS
	T_TDC_ASSIGN_ACTION _require () {} // you can use '= REQUIRE' as well
#else//DOT_COMPLETE_MEMBERS
	T_TDC_ASSIGN_ACTION tdc_require() // you can use '= REQUIRE' as well
	{
		return set_action (REQUIRE);
	}
#endif//DOT_COMPLETE_MEMBERS
};

struct T_TDC_INTERFACE_MSTRUCT_BASE:T_TDC_INTERFACE_REF
{
};

struct T_TDC_INTERFACE_MUNION_BASE:T_TDC_INTERFACE_REF
{
};

struct T_TDC_INTERFACE_MENUM_BASE:T_TDC_INTERFACE_VAR
{
};

struct T_TDC_INTERFACE_INT_BASE:T_TDC_INTERFACE_VAR
{
};

struct T_TDC_INTERFACE_XXX_PRIMITIVE_UNION_BASE:T_TDC_INTERFACE_PRIMITIVE_BASE
{
	T_TDC_INTERFACE_XXX_PRIMITIVE_UNION_BASE ()
	{
		level[0] = 'A'+tdc_level;
		tdc_level++;
	}
};

struct T_TDC_INTERFACE_XXX_MESSAGE_UNION_BASE:T_TDC_INTERFACE_MESSAGE_BASE
{
	T_TDC_INTERFACE_XXX_MESSAGE_UNION_BASE ()
	{
		level[0] = 'A'+tdc_level;
		tdc_level++;
	}
};

struct T_TDC_INTERFACE_PRIMITIVE_UNION_BASE:T_TDC_INTERFACE_PRIMITIVE_BASE
{
	T_TDC_INTERFACE_PRIMITIVE_UNION_BASE ()
	{
		level[0] = 'A'+tdc_level;
		tdc_level++;
	}
};

struct T_TDC_INTERFACE_MESSAGE_UNION_BASE:T_TDC_INTERFACE_MESSAGE_BASE
{
	T_TDC_INTERFACE_MESSAGE_UNION_BASE ()
	{
		level[0] = 'A'+tdc_level;
		tdc_level++;
	}
};

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q
{
  int i1Q;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q s1Q;
  s1Q->
    i;
}
#else
#endif

template<class T> struct T_TDC_INTERFACE_XXX_PRIMITIVE:T_TDC_INTERFACE_POINTER_MAIN<T TDC_HIDDEN_LEVEL_MAIN(T_TDC_INTERFACE_PRIMITIVE_BASE) >
{
};

template<class T> struct T_TDC_INTERFACE_XXX_MESSAGE:T_TDC_INTERFACE_POINTER_MAIN<T TDC_HIDDEN_LEVEL_MAIN(T_TDC_INTERFACE_MESSAGE_BASE) >
{
};

template<class T> struct T_TDC_INTERFACE_PRIMITIVE:T_TDC_INTERFACE_POINTER_MAIN<T TDC_HIDDEN_LEVEL_MAIN(T_TDC_INTERFACE_PRIMITIVE_BASE) >
{
};

template<class T> struct T_TDC_INTERFACE_PRIMITIVE_UNION_POINTER:T_TDC_INTERFACE_POINTER_MAIN<T TDC_HIDDEN_LEVEL_MAIN(T_TDC_INTERFACE_PRIMITIVE_BASE) >
{
};

template<class T> struct T_TDC_INTERFACE_MESSAGE:T_TDC_INTERFACE_POINTER_MAIN<T TDC_HIDDEN_LEVEL_MAIN(T_TDC_INTERFACE_MESSAGE_BASE) >
{
};

template<class T> struct T_TDC_INTERFACE_MESSAGE_UNION_POINTER:T_TDC_INTERFACE_POINTER_MAIN<T TDC_HIDDEN_LEVEL_MAIN(T_TDC_INTERFACE_MESSAGE_BASE) >
{
};

//\}

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q1
{
  int i1Q1;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q1u
{
  int i1Q1u;
};
template<class T>
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q1t1
{
  int i1Q1t;
  //T* operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q1()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q1 s1Q1;
  s1Q1->
    i;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q1t1<T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q1u> s1Q1t1u;
  s1Q1t1u.
    i1Q1t;
  s1Q1t1u->
    i1Q1u;
  /*T_TDC_INTERFACE_PRIMITIVE_UNION_POINTER z1Q1_;
  z1Q1_->
    i;*/
  T_TDC_INTERFACE_PRIMITIVE_UNION_POINTER<T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Q1> z1Q1;
  z1Q1.
    x;
  z1Q1->
    i;
}
#else
#endif

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1R
{
  int i1R;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1R()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1R s1R;
  s1R->
    i0;
}
#else
#endif

//----------------------------------------------------------------------------
/// \defgroup T_TDC_DESCRIPTOR T_TDC_DESCRIPTOR
/// base classes to repeat stuff that is similar in many descriptor classes
//----------------------------------------------------------------------------
//\{

struct T_TDC_DESCRIPTOR_ARRAY_BASE:T_TDC_DESCRIPTOR_BASE
{
	int tap_handle;
	int implement_get_tap_array_handle (unsigned c_elements, T_TDC_DESCRIPTOR_BASE* sdu_handle);
	int get_tap_array_handle (unsigned c_elements);
	bool skip_to_end;
	virtual T_TDC_IS_ENUM is ();
	virtual void set_skip_to_end (bool skip_to_end_);
	virtual bool get_skip_to_end ();
	T_TDC_DESCRIPTOR_ARRAY_BASE ();
	virtual char* get_name () const;
	~T_TDC_DESCRIPTOR_ARRAY_BASE ();
	T_TDC_HANDLE_BASE** get_elements();
  unsigned get_c_elements();
  void set_c_elements(unsigned c_elements);
	T_TDC_HANDLE_BASE* make_element (unsigned index);
	T_TDC_HANDLE_BASE* get_element (unsigned index);
};

template<class T, int MAX_INDEX = 1> 
struct T_TDC_DESCRIPTOR_ARRAY:T_TDC_DESCRIPTOR_ARRAY_BASE
{
	typedef T_TDC_DESCRIPTOR_ARRAY<T, MAX_INDEX> T_DESCRIPTOR_ARRAY;
	friend T_TDC_DESCRIPTOR_BASE * new_T_DESCRIPTOR_ARRAY(void)
	{
		return new T_TDC_DESCRIPTOR_ARRAY<T, MAX_INDEX>;
	}
	unsigned c_elements;
	typedef T T_ELEMENT;
  T_ELEMENT* elements;
	T_TDC_DESCRIPTOR_ARRAY()
	{
		c_elements = 0;
		elements = 0;
	}
	virtual int get_tap_handle ()
	{
		return get_tap_array_handle (c_elements);
	}
	#ifndef TDC_TEST_PROFILE 
	virtual long get_sizeof ()
	{
		/* this function should never be called but the base version have no chance of returning a proper value so we keep it just in case */
		TDC_DYNAMIC_DEAD_CODE();
		return sizeof *this;
	}
	#endif
};

struct T_TDC_HANDLE_ARRAY_BASE:T_TDC_HANDLE_BASE
{
	virtual char* get_name () const;
	virtual T_TDC_IS_ENUM is ()
	{
		return TDC_IS_ARRAY;
	}
};

template<class T, int MAX_INDEX = 1> 
struct T_TDC_HANDLE_ARRAY:T_TDC_HANDLE_ARRAY_BASE
{
	typedef T_TDC_DESCRIPTOR_ARRAY<T, MAX_INDEX> T_DESCRIPTOR_ARRAY;
	virtual long get_sizeof ()
	{
		return sizeof *this;
	}
  M_TDC_DESCRIPTOR_HANDLE_ADDITIONAL_PART (T_DESCRIPTOR_ARRAY)
protected:
  virtual T_TDC_DESCRIPTOR_BASE* implement_new_descriptor () const;
};

#if DOT_COMPLETE 
#else
// the dot complete compiler can not handle this code
template<class T, int MAX_INDEX> 
T_TDC_DESCRIPTOR_BASE* T_TDC_HANDLE_ARRAY<T, MAX_INDEX>::implement_new_descriptor(void) const
{
	//return new T_DESCRIPTOR_ARRAY;
	/*TODO: We don't allways need the element array and the code was once remove 
    but it seems like there are still problems with code like:
      T_ARRAY<T_S8> parray_ps8c;
      parray_ps8c[0]. _require;
  */
  T_DESCRIPTOR_ARRAY *descriptor = new T_DESCRIPTOR_ARRAY;
	/*T_TDC_HANDLE_BASE** elements = ((T_TDC_DESCRIPTOR_ARRAY_BASE*)descriptor)->get_elements();
	if (!*elements)
    //*elements = new_handle ();
		*elements = T::implement_new_handle();*/  
  return descriptor;
}
#endif

#ifdef TDC_SIMULATE_POINTER
#define T_TDC_DESCRIPTOR_POINTER T_TDC_DESCRIPTOR_ARRAY
#define T_TDC_HANDLE_POINTER T_TDC_HANDLE_ARRAY
#else

struct T_TDC_DESCRIPTOR_POINTER_BASE:T_TDC_DESCRIPTOR_BASE
{
	virtual T_TDC_IS_ENUM is ();
	virtual char* get_name () const;
	T_TDC_HANDLE_BASE** get_elements();
	virtual T_TDC_HANDLE_BASE* get_element (unsigned index);
	virtual int get_tap_handle ();
 	//virtual int read_ctrl () const;
};

template<class T /*T_HANDLE*/> struct T_TDC_DESCRIPTOR_POINTER:T_TDC_DESCRIPTOR_POINTER_BASE
{
	typedef T_TDC_DESCRIPTOR_POINTER<T> T_DESCRIPTOR_POINTER;
	friend T_TDC_DESCRIPTOR_BASE * new_T_DESCRIPTOR_POINTER(void)
	{
		return new T_TDC_DESCRIPTOR_POINTER<T, MAX_INDEX>;
	}
	//T::T_DESCRIPTOR element;
	T* element;
	//T element;
	#ifndef TDC_TEST_PROFILE 
	virtual long get_sizeof ()
	{
		/* this function should never be called but the base version have no chance of returning a proper value so we keep it just in case */
		TDC_DYNAMIC_DEAD_CODE();
		return sizeof *this;
	}
	#endif
};

struct T_TDC_HANDLE_POINTER_BASE:T_TDC_HANDLE_BASE
{
	virtual char* get_name () const;
	virtual T_TDC_IS_ENUM is ()
	{
		return TDC_IS_POINTER;
	}
	virtual T_TDC_DESCRIPTOR_BASE* implement_new_descriptor () const;
};

template<class T> struct T_TDC_HANDLE_POINTER:T_TDC_HANDLE_POINTER_BASE
{
	typedef T_TDC_DESCRIPTOR_POINTER<T> T_DESCRIPTOR_POINTER;
	virtual long get_sizeof ()
	{
		return sizeof *this;
	}
	M_TDC_DESCRIPTOR_HANDLE_ADDITIONAL_PART (T_DESCRIPTOR_POINTER)
protected:
  virtual T_TDC_DESCRIPTOR_BASE* implement_new_descriptor () const;
};

#if DOT_COMPLETE 
#else
// the dot complete compiler can not handle this code
template<class T> 
T_TDC_DESCRIPTOR_BASE* T_TDC_HANDLE_POINTER<T>::implement_new_descriptor(void) const
{
	return new T_DESCRIPTOR_POINTER;
}
#endif

#endif

//----------------------------------------------------------------------------

struct T_TDC_DESCRIPTOR_MAIN_BASE:T_TDC_DESCRIPTOR_BASE
{
	virtual int get_id () const
	TDC_PURE_BODY(
		return 0;
	)
	T_TDC_HANDLE_BASE* get_element (unsigned index)
	{
		char *first_element = (char*) this + sizeof *this;
		return implement_get_element (index, first_element);
	}
};

struct T_TDC_DESCRIPTOR_XSTRUCT_BASE:T_TDC_DESCRIPTOR_BASE
{ 
	int tap_handle;
	int get_tap_xstruct_handle (T_TDC_TABLE_KIND table_kind);
	T_TDC_DESCRIPTOR_XSTRUCT_BASE ();
	~T_TDC_DESCRIPTOR_XSTRUCT_BASE ();
	T_TDC_HANDLE_BASE* get_element (unsigned index);
};

struct T_TDC_DESCRIPTOR_INT_BASE:T_TDC_HANDLE_BASE
{
  virtual int get_tap_handle ();
};

//-----

struct T_TDC_DESCRIPTOR_PRIM_BASE:T_TDC_DESCRIPTOR_MAIN_BASE
{
	virtual T_TDC_IS_ENUM is ()
	{
		return T_TDC_IS_ENUM (TDC_IS_COMP | TDC_IS_PRIM);
	}
};

struct T_TDC_DESCRIPTOR_PSTRUCT_BASE:T_TDC_DESCRIPTOR_XSTRUCT_BASE
{
	virtual T_TDC_IS_ENUM is ()
	{
		return TDC_IS_STRUCT;
	}
	virtual int get_tap_handle  ();
};

struct T_TDC_DESCRIPTOR_PUNION_BASE:T_TDC_DESCRIPTOR_BASE
{
	virtual T_TDC_IS_ENUM is ()
	{
		return TDC_IS_UNION;
	}
};

struct T_TDC_DESCRIPTOR_SDU_BASE:T_TDC_DESCRIPTOR_BASE
{
	virtual T_TDC_IS_ENUM is ()
	{
		return TDC_IS_SDU;
	}
	virtual int get_tap_handle ()
	{
		return TDC_DUMMY_TAP_HANDLE;
	}
	void invoke_tap (T_TDC_EVENT_ENUM event) const;
};

struct T_TDC_DESCRIPTOR_PENUM_BASE:T_TDC_HANDLE_BASE
{
  virtual int get_tap_handle ();
	virtual T_TDC_IS_ENUM is ()
	{
		return TDC_IS_PVAR;
	}
};

//-----

struct T_TDC_DESCRIPTOR_MSG_BASE:T_TDC_DESCRIPTOR_MAIN_BASE
{
	virtual T_TDC_IS_ENUM is ()
	{
		return T_TDC_IS_ENUM (TDC_IS_COMP | TDC_IS_MSG);
	}
};

struct T_TDC_DESCRIPTOR_MSTRUCT_BASE:T_TDC_DESCRIPTOR_XSTRUCT_BASE
{
	virtual T_TDC_IS_ENUM is ()
	{
		return TDC_IS_STRUCT;
	}
	virtual int get_tap_handle ();
};

struct T_TDC_DESCRIPTOR_MUNION_BASE:T_TDC_DESCRIPTOR_BASE
{
	virtual T_TDC_IS_ENUM is ()
	{
		return TDC_IS_UNION;
	}
};

struct T_TDC_DESCRIPTOR_MENUM_BASE:T_TDC_HANDLE_BASE
{
  virtual int get_tap_handle ();
	virtual T_TDC_IS_ENUM is ()
	{
		return TDC_IS_MVAR;
	}
};

//-----

struct T_TDC_DESCRIPTOR_XXX_PRIMITIVE_UNION_BASE:T_TDC_DESCRIPTOR_BASE
{
};

struct T_TDC_DESCRIPTOR_XXX_MESSAGE_UNION_BASE:T_TDC_DESCRIPTOR_BASE
{
};

struct T_TDC_DESCRIPTOR_PRIMITIVE_UNION_BASE:T_TDC_DESCRIPTOR_BASE
{
};

struct T_TDC_DESCRIPTOR_MESSAGE_UNION_BASE:T_TDC_DESCRIPTOR_BASE
{
	//TODO: construct munions (when called from make_element_info, shold be hamless in get_element_info)
};

//\}

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1S
{
  int i1S;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1S()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1S s1S;
  s1S->
    i0;
}
#else
#endif

//----------------------------------------------------------------------------
/// \defgroup T_TDC_INSTANCE T_TDC_INSTANCE
/// base classes to repeat stuff that is similar in many instance classes
//----------------------------------------------------------------------------
///\{

struct T_TDC_INSTANCE_BASE
{
//private:
	T_TDC_HANDLE_BASE* implement_get_handle () const;
	void implement_set_handle(T_TDC_HANDLE_BASE* handle);
//protected:
  virtual T_TDC_INTERFACE_BASE *new_interface () const;
//public:
	T_TDC_INTERFACE_BASE *get_navigation () const;
//protected:
	T_TDC_INTERFACE_BASE *get_element_navigation (int index) const;
//public:
  T_TDC_INTERFACE_BASE *make_element_navigation (int index, T_TDC_NEW_HANDLE new_element_handle);
	T_TDC_HANDLE_BASE* get_descriptor_ref () const;
	T_TDC_ASSIGN_ACTION set_action(const T_TDC_ACTION& action_);
	void set_value (const T_TDC_INSTANCE_BASE& value_);
	void copy_instance (const T_TDC_INSTANCE_BASE * value_);
	void copy_interface (const T_TDC_INTERFACE_BASE * value_);
	T_TDC_ASSIGN_ACTION construct_from_action(const T_TDC_ACTION& action_, T_TDC_NEW_HANDLE new_handle);
	void construct_from_instance (const T_TDC_INSTANCE_BASE& value_, T_TDC_NEW_HANDLE new_handle);
	void construct_from_interface (const T_TDC_INTERFACE_BASE * value_, T_TDC_NEW_HANDLE new_handle);
	void set_descriptor_value (long value_);
	long get_descriptor_value ();
	bool cmp_descriptor_value (long value_); 
	void construct_from_number (long value_, T_TDC_NEW_HANDLE new_handle);
	void destroy_handle ();
	T_TDC_HANDLE_BASE* construct_handle (T_TDC_NEW_HANDLE new_handle);
  T_TDC_HANDLE_BASE* construct_array_handle (T_TDC_NEW_HANDLE new_handle,T_TDC_NEW_HANDLE new_element_handle);
	T_TDC_DESCRIPTOR_BASE* construct_descriptor (T_TDC_NEW_HANDLE new_handle);
	T_TDC_DESCRIPTOR_BASE* construct_array_descriptor (T_TDC_NEW_HANDLE new_handle,T_TDC_NEW_HANDLE new_element_handle);
	T_TDC_INSTANCE_BASE();
  ~T_TDC_INSTANCE_BASE();
private:
  ///Check for copy constructor missing in class derived from T_TDC_INSTANCE_BASE
	T_TDC_INSTANCE_BASE(const T_TDC_INSTANCE_BASE&);
};

struct T_TDC_INSTANCE_MAIN_BASE:T_TDC_INSTANCE_BASE
{
	void set_main_value (const T_TDC_INTERFACE_MAIN_BASE* value_);
	void set_main_value (const T_TDC_INSTANCE_MAIN_BASE* value_);
	void construct_main_value (const T_TDC_INTERFACE_MAIN_BASE* value_, T_TDC_NEW_HANDLE new_handle);
	void construct_main_value (const T_TDC_INSTANCE_MAIN_BASE* value_, T_TDC_NEW_HANDLE new_handle);
};

struct T_TDC_INSTANCE_PART_BASE
#if DOT_COMPLETE
#else
	TDC_DOT_COMPLETE_HIDE(:T_TDC_INSTANCE_BASE)
#endif
{
};

struct T_TDC_INSTANCE_PRIMITIVE_BASE
#if DOT_COMPLETE
#else
	TDC_DOT_COMPLETE_HIDE(:T_TDC_INSTANCE_MAIN_BASE)
#endif
{
#if DOT_COMPLETE
#else
	TDC_DOT_COMPLETE_HIDE(void invoke_tap (T_TDC_EVENT_ENUM) const;)
#endif
};

struct T_TDC_INSTANCE_MESSAGE_BASE
#if DOT_COMPLETE
#else
	TDC_DOT_COMPLETE_HIDE(:T_TDC_INSTANCE_MAIN_BASE)
#endif
{
};

struct T_TDC_INSTANCE_PRIM_BASE:T_TDC_INSTANCE_PRIMITIVE_BASE
{
};

struct T_TDC_INSTANCE_PSTRUCT_BASE:T_TDC_INSTANCE_PART_BASE
{
};

struct T_TDC_INSTANCE_PUNION_BASE:T_TDC_INSTANCE_PART_BASE
{
};

struct T_TDC_INSTANCE_SDU_BASE:T_TDC_INSTANCE_PART_BASE
{
};

struct T_TDC_INSTANCE_PENUM_BASE:T_TDC_INSTANCE_PART_BASE
{
};

struct T_TDC_INSTANCE_MSG_BASE:T_TDC_INSTANCE_MESSAGE_BASE
{
};

struct T_TDC_INSTANCE_MSTRUCT_BASE:T_TDC_INSTANCE_PART_BASE
{
};

struct T_TDC_INSTANCE_MUNION_BASE:T_TDC_INSTANCE_PART_BASE
{
};

struct T_TDC_INSTANCE_MENUM_BASE:T_TDC_INSTANCE_PART_BASE
{
};

struct T_TDC_INSTANCE_INT_BASE:T_TDC_INSTANCE_PART_BASE
{
};

struct T_TDC_INSTANCE_XXX_PRIMITIVE_UNION_BASE:T_TDC_INSTANCE_PRIMITIVE_BASE
{
};

struct T_TDC_INSTANCE_XXX_MESSAGE_UNION_BASE:T_TDC_INSTANCE_MESSAGE_BASE
{
};

struct T_TDC_INSTANCE_PRIMITIVE_UNION_BASE:T_TDC_INSTANCE_PRIMITIVE_BASE
{
};

struct T_TDC_INSTANCE_MESSAGE_UNION_BASE:T_TDC_INSTANCE_MESSAGE_BASE
{
};

//----------------------------------------------------------------------------

struct T_TDC_INSTANCE_ARRAY_BASE
#if DOT_COMPLETE
#else
	TDC_DOT_COMPLETE_HIDE(:T_TDC_INSTANCE_BASE)
#endif
{
#if DOT_COMPLETE_MEMBERS
	T_TDC_ASSIGN_ACTION _skip () {} /* you can use '= SKIP' as well */
	T_TDC_ASSIGN_ACTION _show () {} /* you can use '= SHOW' as well */
	T_TDC_ASSIGN_ACTION _forbid () {} /* you can use '= FORBID' as well */
	T_TDC_ASSIGN_ACTION _require () {} /* you can use '= REQUIRE' as well */
#else//DOT_COMPLETE_MEMBERS
	T_TDC_ASSIGN_ACTION tdc_skip (); /* you can use '= SKIP' as well */
	T_TDC_ASSIGN_ACTION tdc_show (); /* you can use '= SHOW' as well */
	T_TDC_ASSIGN_ACTION tdc_forbid (); /* you can use '= FORBID' as well */
	T_TDC_ASSIGN_ACTION tdc_require (); /* you can use '= REQUIRE' as well */
	/*T_ARRAY operator = (const T_TDC_ACTION& action_)
	{
		set_action (action_);
		return *this;
	}*/
#endif//DOT_COMPLETE_MEMBERS
};

#if DOT_COMPLETE
template<class T /*T_INSTANCE*/, int DUMMY> struct T_ARRAY
#else
template<class T /*T_INSTANCE*/> struct T_ARRAY
#endif
	:T_TDC_INSTANCE_ARRAY_BASE
{
  T::T_INTERFACE& operator [] (int index_)
  {
		return *(T::T_INTERFACE*)make_element_navigation(index_,T::implement_new_handle);
  }
  /*
  T::T_INTERFACE& operator -> ()
  {
		return *(T::T_INTERFACE*)get_element_navigation(0);
  }
  */
#if DOT_COMPLETE
#else
	typedef T_TDC_HANDLE_ARRAY<T::T_HANDLE> T_HANDLE;
  T_HANDLE* handle;
	friend T_TDC_HANDLE_BASE* new_T_HANDLE()\
	{
		return new T_HANDLE;//(T::implement_new_handle);
	}
	T_ARRAY operator = (const T_ARRAY& value_)
	{
		set_value (value_);
		return *this;
	}
	T_ARRAY ()
	{
		construct_array_handle(new_T_HANDLE,T::implement_new_handle);
	}
	~T_ARRAY ()
	{
		destroy_handle();
	}
	// T_ARRAY (const T_ARRAY<T>& u1)  =>  ambiguous call to overloaded function
	void assign (const T_ARRAY<T>& array, unsigned size, void* address)
	{
		construct_array_descriptor(new_T_HANDLE,T::implement_new_handle);
		T_TDC_COPY(handle).copy_descriptor_ref (array.handle);
	}
	template<class U1> void assign (const U1* array, unsigned size, void* address)
	{
		tdc_check_array_assignment(array, address);
		int count = size / sizeof *array;
		T_TDC_DESCRIPTOR_BASE* descriptor = construct_array_descriptor(new_T_HANDLE,T::implement_new_handle);
		for (int i = 0; i < count; i++)
		{
			T_TDC_COPY element = descriptor->make_element(i);
			element.copy_descriptor_ref ((T_TDC_HANDLE_BASE*) array[i].handle);
		}
	}
	void assign (const T_TDC_ACTION& action_, unsigned size, void *address)
	{
		construct_from_action (action_, new_T_HANDLE);
	}
	/*T_ARRAY (const T_ARRAY& array) //ambiguous call to overloaded function
	{
		assign (array, sizeof array, (void*)&array);
	}*/
	template<class U1> T_ARRAY (const U1& array)
	{
		assign (array, sizeof array, (void*)&array);
	}
	template<class U1> T_ARRAY (const U1& u1, T_TDC_ACTION action)
	{
		const T* array = u1;
		int count = sizeof u1 / sizeof *u1;
		T_TDC_DESCRIPTOR_BASE* descriptor = construct_array_descriptor(new_T_HANDLE,T::implement_new_handle);
    descriptor->set_skip_to_end_from_action (action.action);
		for (int i = 0; i < count; i++)
		{
			T_TDC_COPY element = descriptor->make_element(i);
			element.copy_descriptor_ref ((T_TDC_HANDLE_BASE*) array[i].handle);
		}
	}
  template<class U1> T_ARRAY (const U1& u1, T_TDC_CREATE_ACTION action)
  {
    const T* array = u1;
    int count = sizeof u1 / sizeof *u1;
    T_TDC_DESCRIPTOR_BASE* descriptor = construct_array_descriptor(new_T_HANDLE,T::implement_new_handle);
    descriptor->set_skip_to_end_from_action (action.action);
    for (int i = 0; i < count; i++)
    {
      T_TDC_COPY element = descriptor->make_element(i);
      element.copy_descriptor_ref ((T_TDC_HANDLE_BASE*) array[i].handle);
    }
  }
	template<class U1, class U2> T_ARRAY (const U1& u1, const U2& u2)
	{
		tdc_tbd_array_assignment_error_T_ARRAY();
	}
  /*
	template<class U1, class U2, class U3> T_ARRAY (U1 u1, U2 u2, U3 u3)
	{
		TDC_TBD();
	}
	template<class U1, class U2, class U3, class U4> T_ARRAY (U1 u1, U2 u2, U3 u3, U4 u4)
	{
		TDC_TBD();
	}
	template<class U1, class U2, class U3, class U4, class U5> T_ARRAY (U1 u1, U2 u2, U3 u3, U4 u4, U5 u5)
	{
		TDC_TBD();
	}
  */
	virtual T_TDC_INTERFACE_BASE *new_interface () const
	{
		return new T::T_INTERFACE;
	}
#endif
};


#define	M_TDC_BASIC_ARRAY(T_BASIC)\
	T_ARRAY ()\
	{\
		/*construct_handle(new_T_HANDLE);*/\
	}\
	template<class U1>\
	T_ARRAY (const U1& array, T_TDC_ACTION action)\
	{\
		assign (array, sizeof array, (void*)&array, action);\
	}\
	template<class U1>\
	T_ARRAY (const U1& array)\
	{\
		assign (array, sizeof array, (void*)&array, TDC_ACTION_UNKNOWN);\
	}\
	T_ARRAY operator = (const T_ARRAY& value_)\
	{\
		set_value (value_);\
		return *this;\
	}\
	T_ARRAY operator = (const T_TDC_ACTION& action_)\
	{\
		construct_from_action (action_,new_T_HANDLE);\
		return *this;\
	}\

template<class T, class T_BASIC> struct T_TDC_INSTANCE_BASIC_ARRAY:T_TDC_INSTANCE_ARRAY_BASE
{
	typedef T_TDC_HANDLE_ARRAY<T::T_HANDLE> T_HANDLE;
  T_HANDLE* handle;
	static T_TDC_HANDLE_BASE* new_T_HANDLE()\
	{
		return new T_HANDLE;//(T::implement_new_handle);
	}
	T_TDC_INSTANCE_BASIC_ARRAY ()
	{
		construct_array_descriptor(new_T_HANDLE,T::implement_new_handle);
	}
	void assign (const T_BASIC* array, unsigned size, void* address, T_TDC_ACTION action)
	{
		tdc_check_array_assignment(array, address);
		unsigned count = size / sizeof T_BASIC;
		T_TDC_DESCRIPTOR_BASE* descriptor = construct_array_descriptor(new_T_HANDLE,T::implement_new_handle);
    descriptor->set_skip_to_end_from_action (action.action);
		for (int i = 0; i < count; i++)
		{
			T_TDC_COPY element= descriptor->make_element(i);
			element.copy_number (array[i]);
		}
	}
  void assign (const T_BASIC* array, unsigned size, void* address, T_TDC_CREATE_ACTION action)
  {
    tdc_check_array_assignment(array, address);
    unsigned count = size / sizeof T_BASIC;
    T_TDC_DESCRIPTOR_BASE* descriptor = construct_array_descriptor(new_T_HANDLE,T::implement_new_handle);
    descriptor->set_skip_to_end_from_action (action.action);
    for (int i = 0; i < count; i++)
    {
      T_TDC_COPY element= descriptor->make_element(i);
      element.copy_number (array[i]);
    }
  }
	void assign (const T_ARRAY<T>& array, unsigned size, void* address)
	{
		construct_array_descriptor(new_T_HANDLE,T::implement_new_handle);
		T_TDC_COPY(handle).copy_descriptor_ref (array.handle);
	}
	template<class U1> void assign (const U1* array, unsigned size, void* address)
	{
		tdc_check_array_assignment(array, address);
		int count = size / sizeof *array;
		T_TDC_DESCRIPTOR_BASE* descriptor = construct_array_descriptor(new_T_HANDLE,T::implement_new_handle);
		for (int i = 0; i < count; i++)
		{
			T_TDC_COPY element = descriptor->make_element(i);
			element.copy_descriptor_ref ((T_TDC_HANDLE_BASE*) array[i].handle);
		}
	}
	void assign (const T_TDC_ACTION& action_, unsigned size, void *address)
	{
		construct_from_action (action_, new_T_HANDLE);
	}
	void assign (const T_ARRAY<T_BASIC>& array, unsigned size, void* address, T_TDC_ACTION action)
	{
    construct_from_instance(array,new_T_HANDLE);
	}
  T::T_INTERFACE& operator [] (int index_)
  {
	  return *(T::T_INTERFACE*)get_element_navigation(index_);
  }
	virtual T_TDC_INTERFACE_BASE *new_interface () const
	{
		return new T::T_INTERFACE;
	}
};

//\}

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W
{
  int i1W;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W s1W;
  s1W->
    i0;
}
#else
#endif

//============================================================================
/// \defgroup BuildInTypes build in types
//============================================================================
//\{

//#pragma M_TDC_MESSAGE (M_TDC_STRING(M_TDC_FORWARD_INT (U8)))
M_TDC_FORWARD_INT (U8)
M_TDC_FORWARD_INT (S8)
M_TDC_FORWARD_INT (U16)
M_TDC_FORWARD_INT (S16)
M_TDC_FORWARD_INT (U32)
M_TDC_FORWARD_INT (S32)

M_TDC_FORWARD_PSTRUCT (raw)
M_TDC_FORWARD_PSTRUCT (aim)
M_TDC_FORWARD_COMP (MESSAGE_UNION,TDC_IS_MESSAGE_UNION)

//M_TDC_FORWARD_MSTRUCT(COMP_STENCIL)
M_TDC_FORWARD_BASE(COMP_STENCIL, T_TDC_HANDLE_COMP_STENCIL)
M_TDC_HANDLE(M_TDC_IN_CLASS,M_TDC_WITHOUT_BODY,COMP_STENCIL,TDC_IS_COMP)

#ifndef TDC_DOT_COMPLETE_HIDE_PROTOTYPES
struct T_PRIMITIVE_UNION;
struct T_TDC_INTERFACE_PRIMITIVE_UNION;
struct T_TDC_DESCRIPTOR_PRIMITIVE_UNION;
struct T_TDC_HANDLE_PRIMITIVE_UNION;
T_TDC_HANDLE_BASE* new_T_TDC_HANDLE_PRIMITIVE_UNION();

struct T_MESSAGE_UNION;
struct T_TDC_INTERFACE_MESSAGE_UNION;
struct T_TDC_DESCRIPTOR_MESSAGE_UNION;
struct T_TDC_HANDLE_MESSAGE_UNION;
T_TDC_HANDLE_BASE* new_T_TDC_HANDLE_MESSAGE_UNION();
#endif

struct T_TDC_DESCRIPTOR_S8:T_TDC_DESCRIPTOR_INT_BASE
{
	M_TDC_DESCRIPTOR_INT_ADDITIONAL (S8)
};

struct T_TDC_DESCRIPTOR_U8:T_TDC_DESCRIPTOR_INT_BASE
{
	M_TDC_DESCRIPTOR_INT_ADDITIONAL (U8)
};

struct T_TDC_DESCRIPTOR_S16:T_TDC_DESCRIPTOR_INT_BASE
{
	M_TDC_DESCRIPTOR_INT_ADDITIONAL (S16)
};

struct T_TDC_DESCRIPTOR_U16:T_TDC_DESCRIPTOR_INT_BASE
{
	M_TDC_DESCRIPTOR_INT_ADDITIONAL (U16)
};

struct T_TDC_DESCRIPTOR_S32:T_TDC_DESCRIPTOR_INT_BASE
{
	M_TDC_DESCRIPTOR_INT_ADDITIONAL (S32)
};

struct T_TDC_DESCRIPTOR_U32:T_TDC_DESCRIPTOR_INT_BASE
{
	M_TDC_DESCRIPTOR_INT_ADDITIONAL (U32)
};

struct T_TDC_DESCRIPTOR_raw:T_TDC_DESCRIPTOR_PSTRUCT_BASE
{
	M_TDC_DESCRIPTOR_PSTRUCT_ADDITIONAL (raw)
	T_TDC_HANDLE_U16 l_buf;
	T_TDC_HANDLE_U16 o_buf;
	T_TDC_HANDLE_ARRAY<T_TDC_DESCRIPTOR_U8> buf;
	virtual int get_tap_handle ();
};

struct T_TDC_DESCRIPTOR_COMP_STENCIL:T_TDC_DESCRIPTOR_MSTRUCT_BASE
{
	M_TDC_DESCRIPTOR_MSTRUCT_ADDITIONAL (COMP_STENCIL)
};

struct T_TDC_DESCRIPTOR_VAR_STENCIL:T_TDC_DESCRIPTOR_MENUM_BASE
{
  typedef long T_TDC_ENUM_VAR_STENCIL;
	M_TDC_DESCRIPTOR_MENUM_ADDITIONAL (VAR_STENCIL)
};
 
#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W1
{
  int i1W1;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W1()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W1 s1W1;
  s1W1->
    i0;
}
#else
#endif


typedef T_TDC_DESCRIPTOR_ARRAY<T_TDC_HANDLE_COMP_STENCIL> T_TDC_DESCRIPTOR_ARRAY_STENCIL;
typedef T_TDC_DESCRIPTOR_POINTER<T_TDC_HANDLE_COMP_STENCIL> T_TDC_DESCRIPTOR_POINTER_STENCIL;
typedef T_TDC_HANDLE_POINTER<T_TDC_HANDLE_COMP_STENCIL> T_TDC_HANDLE_POINTER_STENCIL;


#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W4
{
  int i1W4;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W4()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W4 s1W4;
  s1W4->
    i0;
}
#else
#endif

#if !defined TDC_DESCRIPTOR || defined TDC_PRECOMPILE 

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W5
{
  int i1W5;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W5()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W5 s1W5;
  s1W5->
    i0;
}
#else
#endif

struct T_U8:T_TDC_INSTANCE_INT_BASE
{
//enable pragma line to see expanded version of macro in list file
//#pragma M_TDC_MESSAGE (M_TDC_STRING(M_TDC_INSTANCE_INT_ADDITIONAL (U8)))
	M_TDC_INSTANCE_INT_ADDITIONAL (U8)
	T_TDC_INTERFACE_U8* operator-> ();
};
struct T_TDC_INTERFACE_U8:T_TDC_INTERFACE_INT_BASE
{
//enable pragma line to see expanded version of macro in list file
//#pragma M_TDC_MESSAGE (M_TDC_STRING(M_TDC_INTERFACE_INT_ADDITIONAL (U8)))
	M_TDC_INTERFACE_INT_ADDITIONAL (U8)
#ifdef TDC_TYPE_NAME_COMPLETE
	struct T_TDC_TYPE_NAME { char T_U8, ___dummy__basic_type_have_no_members; } _type_name () M_TDC_TYPE_NAME_COMPLETE //HINT: ??? press CTRL + SHIFT + Q to create a reference to this variable
#endif
};

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W6
{
  int i1W6;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W6()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W6 s1W6;
  s1W6->
    i0;
}
#else
#endif

struct T_S8:T_TDC_INSTANCE_INT_BASE
{
	M_TDC_INSTANCE_INT_ADDITIONAL (S8)
	T_TDC_INTERFACE_S8* operator-> ();
};
struct T_TDC_INTERFACE_S8:T_TDC_INTERFACE_INT_BASE
{
	M_TDC_INTERFACE_INT_ADDITIONAL (S8)
#ifdef TDC_TYPE_NAME_COMPLETE
	struct T_TDC_TYPE_NAME { char T_S8, ___dummy__basic_type_have_no_members; } _type_name () M_TDC_TYPE_NAME_COMPLETE //HINT: ??? press CTRL + SHIFT + Q to create a reference to this variable
#endif
};

struct T_U16:T_TDC_INSTANCE_INT_BASE
{
	M_TDC_INSTANCE_INT_ADDITIONAL (U16)
	T_TDC_INTERFACE_U16* operator-> ();
};
struct T_TDC_INTERFACE_U16:T_TDC_INTERFACE_INT_BASE
{
	M_TDC_INTERFACE_INT_ADDITIONAL (U16)
#ifdef TDC_TYPE_NAME_COMPLETE
	struct T_TDC_TYPE_NAME { char T_U16, ___dummy__basic_type_have_no_members; } _type_name () M_TDC_TYPE_NAME_COMPLETE //HINT: ??? press CTRL + SHIFT + Q to create a reference to this variable
#endif
};

struct T_S16:T_TDC_INSTANCE_INT_BASE
{
	M_TDC_INSTANCE_INT_ADDITIONAL (S16)
	T_TDC_INTERFACE_S16* operator-> ();
};
struct T_TDC_INTERFACE_S16:T_TDC_INTERFACE_INT_BASE
{
	M_TDC_INTERFACE_INT_ADDITIONAL (S16)
#ifdef TDC_TYPE_NAME_COMPLETE
	struct T_TDC_TYPE_NAME { char T_S16, ___dummy__basic_type_have_no_members; } _type_name () M_TDC_TYPE_NAME_COMPLETE //HINT: ??? press CTRL + SHIFT + Q to create a reference to this variable
#endif
};

struct T_U32:T_TDC_INSTANCE_INT_BASE
{
	M_TDC_INSTANCE_INT_ADDITIONAL (U32)
	T_TDC_INTERFACE_U32* operator-> ();
};
struct T_TDC_INTERFACE_U32:T_TDC_INTERFACE_INT_BASE
{
	M_TDC_INTERFACE_INT_ADDITIONAL (U32)
#ifdef TDC_TYPE_NAME_COMPLETE
	struct T_TDC_TYPE_NAME { char T_U32, ___dummy__basic_type_have_no_members; } _type_name () M_TDC_TYPE_NAME_COMPLETE //HINT: ??? press CTRL + SHIFT + Q to create a reference to this variable
#endif
};

struct T_S32:T_TDC_INSTANCE_INT_BASE
{
	M_TDC_INSTANCE_INT_ADDITIONAL (S32)
	T_TDC_INTERFACE_S32* operator-> ();
};
struct T_TDC_INTERFACE_S32:T_TDC_INTERFACE_INT_BASE
{
	M_TDC_INTERFACE_INT_ADDITIONAL (S32)
#ifdef TDC_TYPE_NAME_COMPLETE
	struct T_TDC_TYPE_NAME { char T_S32, ___dummy__basic_type_have_no_members; } _type_name () M_TDC_TYPE_NAME_COMPLETE //HINT: ??? press CTRL + SHIFT + Q to create a reference to this variable
#endif
};

struct T_raw:T_TDC_INSTANCE_PSTRUCT_BASE
{
	M_TDC_INSTANCE_PSTRUCT_ADDITIONAL (raw,raw)
	T_TDC_INTERFACE_raw* operator-> ();
};
struct T_TDC_INTERFACE_raw:T_TDC_INTERFACE_PSTRUCT_BASE
{
	M_TDC_INTERFACE_PSTRUCT_ADDITIONAL (raw,raw)
	T_TDC_INTERFACE_U16 l_buf; //number of valid bits
	T_TDC_INTERFACE_U16 o_buf; //offset of first valid bit
	T_TDC_INTERFACE_ARRAY<T_TDC_INTERFACE_U8> buf; //array size = (o_buf + l_buf + 7) /8 
#ifdef TDC_TYPE_NAME_COMPLETE
	struct T_TDC_TYPE_NAME { char T_raw, ___l_buf___o_buf___buf; } _type_name () M_TDC_TYPE_NAME_COMPLETE //HINT: ??? press CTRL + SHIFT + Q to create a reference to this variable
#endif
};

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9
{
  int i1W9;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9 s1W9;
  s1W9->
    i0;
}
#else
#endif

struct T_COMP_STENCIL:T_TDC_INSTANCE_MSTRUCT_BASE
{
  /*void copy_instance (const T_TDC_INSTANCE_BASE * value_)
  {
    TDC_INTERNAL_ERROR();
  }*/
  M_TDC_INSTANCE_MSTRUCT_ADDITIONAL (COMP_STENCIL,COMP_STENCIL)
};
struct T_TDC_INTERFACE_COMP_STENCIL:T_TDC_INTERFACE_MSTRUCT_BASE
{
  M_TDC_INTERFACE_MSTRUCT_ADDITIONAL (COMP_STENCIL,COMP_STENCIL)
};

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9A9
{
  int i1W9A9;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9A9()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9A9 s1W9A9;
  s1W9A9->
    i0;
}
#else
#endif

#ifndef TDC_DOT_COMPLETE_SOURCE_INSIGHT
/**
 allocation call tree:
  function                                                defined in                          
    T_PRIMITIVE_UNION()                                     tdc_base.h
                                                              T_PRIMITIVE_UNION
                                                                M_TDC_INSTANCE_ADDITIONAL_BASE                                                              
      construct_handle(new_T_TDC_HANDLE_PRIMITIVE_UNION)    tdc.cpp
                                                              T_TDC_INSTANCE_BASE
        new_T_TDC_HANDLE_PRIMITIVE_UNION                    ???                                 global
          ???
            implement_new_handle ()                         tdc_lib_main_dsc.cpp                virtual in T_TDC_HANDLE_PRIMITIVE_UNION
                                                              M_TDC_POST_COMP                   
                                                                M_TDC_POST_DESCRIPTOR_COMP      
                                                                  M_TDC_HANDLE_ADDITIONAL 
              new T_TDC_HANDLE_PRIMITIVE_UNION()            not explicit declared
                T_TDC_HANDLE_BASE ()                        tdc_dsc.cpp
                  implement_set_action(TDC_ACTION_DEFAULT)  tdc_dsc.cpp
                  
    T_PRIMITIVE_UNION

        make_descriptor()                                   tdc.cpp
          implement_new_descriptor ()                       tdc.cpp
            new T_TDC_DESCRIPTOR_PRIMITIVE_UNION            tdc_lib_main_dsc.cpp
                                                              M_TDC_POST_COMP
                                                                M_TDC_POST_DESCRIPTOR_COMP
                                                                  

 */
#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9B
{
  int i1W9B;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9B()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->
    i0;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9B s1W9B;
  s1W9B->
    i0;
}
#else
#endif

struct T_PRIMITIVE_UNION
  TDC_DOT_COMPLETE_HIDE(:T_TDC_INSTANCE_PRIMITIVE_UNION_BASE)
{
	TDC_DOT_COMPLETE_HIDE(M_TDC_INSTANCE_PRIMITIVE_ADDITIONAL ())
	T_TDC_INTERFACE_PRIMITIVE_UNION* operator-> ();
};

struct T_MESSAGE_UNION:T_TDC_INSTANCE_MESSAGE_UNION_BASE
{
	TDC_DOT_COMPLETE_HIDE(M_TDC_INSTANCE_MESSAGE_ADDITIONAL ())
	T_TDC_INTERFACE_MESSAGE_UNION* operator-> ();
};

#if TDC_DEBUG_DOT_COMPLETE
struct T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9B
{
  int i1W9B;
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 operator->(){return 0;}
};
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1W9B()
{
  T_PRIMITIVE_UNION primitive_union;
  primitive_union-> XX_TDC_1;
}
#else
#endif

#else
typedef T_TDC_INTERFACE_PRIMITIVE_UNION* T_PRIMITIVE_UNION;
typedef T_TDC_INTERFACE_MESSAGE_UNION* T_MESSAGE_UNION;
#endif

//----------------------------------------------------------------------------

//#if TDC_DOT_COMPLETE
//#else
template<> struct T_ARRAY<U8>:T_TDC_INSTANCE_BASIC_ARRAY<T_U8, U8>
{
	M_TDC_BASIC_ARRAY(U8)
};

template<> struct T_ARRAY<S8>:T_TDC_INSTANCE_BASIC_ARRAY<T_S8, S8>
{
	M_TDC_BASIC_ARRAY(S8)
};

template<> struct T_ARRAY<U16>:T_TDC_INSTANCE_BASIC_ARRAY<T_U16, U16>
{
	M_TDC_BASIC_ARRAY(U16)
};

template<> struct T_ARRAY<S16>:T_TDC_INSTANCE_BASIC_ARRAY<T_S16, S16>
{
	M_TDC_BASIC_ARRAY(S16)
};

template<> struct T_ARRAY<U32>:T_TDC_INSTANCE_BASIC_ARRAY<T_U32, U32>
{
	M_TDC_BASIC_ARRAY(U32)
};

template<> struct T_ARRAY<S32>:T_TDC_INSTANCE_BASIC_ARRAY<T_S32, S32>
{
	M_TDC_BASIC_ARRAY(S32)
};
//#endif

#endif //!defined TDC_DESCRIPTOR || defined TDC_PRECOMPILE 

//\}

//============================================================================
// types used by dynamic part
//============================================================================

struct T_TDC_AWAIT_CONTEXT;
struct T_TDC_SEND_CONTEXT;
struct T_TDC_AWAIT;

#ifdef DOT_COMPLETE
struct T_ON{};
#endif

//============================================================================

enum T_TDC_JMPRET 
{
	TDC_JMPRET_SETJMP = 0, //setjmp is hard coded to return 0
	TDC_JMPRET_INITIAL = 1,
	TDC_JMPRET_FAIL = 2,
	TDC_JMPRET_PASS = 3,
	TDC_JMPRET_BAD_BREAK = 4,
	TDC_JMPRET_CASE_PASS = 5,
	TDC_JMPRET_STEP_PASS = 6,
	TDC_JMPRET_ON_PASS = 7,
	TDC_JMPRET_ON_TEST = 8,
	TDC_JMPRET_ALT_ENTER = 9,
	TDC_JMPRET_OTHERWISE_PASS = 10,
	TDC_JMPRET_OTHERWISE_PARK = 11,
	TDC_JMPRET_TRAP_PASS = 12,
	TDC_JMPRET_POPED = 13,
	TDC_JMPRET_USER_ERROR = 14,
	TDC_JMPRET_INTERNAL_ERROR = 15,
  TDC_JMPRET_TAP_EXCLUDED = 16
};

enum T_TDC_CONTEXT
{
  TDC_CONTEXT_UNKNOWN,
  TDC_CONTEXT_FUNCTION,
  TDC_CONTEXT_CASE,
  TDC_CONTEXT_STEP,
  TDC_CONTEXT_ALT,
  TDC_CONTEXT_ON,
  TDC_CONTEXT_OTHERWISE,
  TDC_CONTEXT_TRAP
};

struct T_TDC_DYNAMIC_CONTEXT_DATA
{
	T_TDC_DYNAMIC_CONTEXT_DATA* parent;
	jmp_buf mark;
	T_TDC_DYNAMIC_CONTEXT_DATA* prev;
	T_TDC_JMPRET jmpret;
	char* file;
	int line;
	int event;
	bool testing;
	char *text;
	int destroy_on_fail_handles_count;
  T_TDC_CONTEXT context;
	T_TDC_HANDLE_BASE** destroy_on_fail_handles;
	void add_destroy_on_fail (T_TDC_HANDLE_BASE* handle);
  void remove_destroy_on_fail (T_TDC_HANDLE_BASE* handle);
	T_TDC_DYNAMIC_CONTEXT_DATA* pop ();
	T_TDC_DYNAMIC_CONTEXT_DATA (T_TDC_CONTEXT 
    context_, T_TDC_DYNAMIC_CONTEXT_DATA* parent_, char* file_, int line_, char* text_);
	~T_TDC_DYNAMIC_CONTEXT_DATA();
	void jmp(T_TDC_JMPRET jmpret_);
};

struct T_TDC_CONTEXT_BASE
{
};

struct T_TDC_FUNCTION_CONTEXT:T_TDC_CONTEXT_BASE //the name of this type is contructed to give more info from error message
{
	/* 
		Got this error?
			error C2039: 'tdc_alt_context' : is not a member of 'T_TDC_FUNCTION_CONTEXT'
		you are violating the following rules:
		
			"ON(...)...;" and "OTHERWISE()...;" must be inside an "ALT{...}" 

		TODO: test above statement
	*/
	T_TDC_DYNAMIC_CONTEXT_DATA tdc_function_context; //this member name is contructed to give more info from error message
	T_TDC_FUNCTION_CONTEXT (char* file_, int line_);
};

struct T_TDC_USER_ERROR_CASE_BASE
{
  // for testing TDC only
  unsigned int failed;
};

struct T_TDC_CASE 
{
	/** \struct T_TDC_CASE 
		this type can not have constructor or member functions as it virolate 'extern "C"' in T_CASE macro

		this type should only contain one member so that if returned from a function with '__declspec(dllexport)' 
		it will be returned in the same way as if such a function returned int, returning an int is what tap2.exe
		expect

		from an internal tdc point of view there is no difference between a function of type T_CASE and one of
		type T_TDC_CASE, this is used in some tdc_test test cases

    founctions of type T_CASE (T_TDC_CASE) should not have explicit returns instead they should contain
    a BEGIN_CASE(...) which will make an implicit return (return is hidden inside macro);
	*/
	unsigned int passed;
};

struct T_TDC_CASE_CONTEXT:T_TDC_CONTEXT_BASE
{
	T_TDC_DYNAMIC_CONTEXT_DATA tdc_not_alt_context;
	T_TDC_CASE_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA* parent_, char* file_, int line_, char* text_);
	void tdc_end_for ();
	bool tdc_do_return ();
  void prepare_exit (int exit_code, char *message);
	T_TDC_CASE tdc_return_value ();
  T_TDC_USER_ERROR_CASE_BASE tdc_user_error_return_value ();
};

struct T_TDC_only_one__BEGIN_CASE__allowed_per_function:T_TDC_CONTEXT_BASE
{
	/* TODO: wrong text type name should say it all
		Got this error?
			error C2039: 'tdc_alt_context' : is not a member of 'T_TDC__OTHERWISE__MUST_BE_LAST_IN_ALT'

		you are violating one of the following rules:
		A)
			"ON(...)...;" cannot follow "OTHERWISE()...;"

		B)
			"AWAIT(...)" is not allow in side "ALT{...}" enclose it with an "ON(...)"
			e.g. ALT{... ON(AWAIT(...))...; ...}
		C)
			"FAIL()", "PASS()", "SEND(...)" are not allowed inside "ALT{...}" they must be 
			inside the body of an "ON(AWAIT(something))...;" or "OTHERWISE()...;"

		remember to use curly parantheses "{...}" when the body contain more than one command
	*/
	T_TDC_DYNAMIC_CONTEXT_DATA* parent;
	T_TDC_only_one__BEGIN_CASE__allowed_per_function (T_TDC_DYNAMIC_CONTEXT_DATA& parent_);
};

/// return of a step
struct T_STEP 
{
	T_STEP ()
	{
		//nothing
	}
};

struct T_TDC_STEP_CONTEXT:T_TDC_CONTEXT_BASE
{
	T_TDC_DYNAMIC_CONTEXT_DATA tdc_not_alt_context;
	T_TDC_STEP_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA* parent_, char* file_, int line_, char* text_);
	void tdc_end_for ();
	bool tdc_do_return ();
	T_STEP tdc_return_value ();
};

struct T_TDC_only_one__BEGIN_STEP__allowed_per_function:T_TDC_CONTEXT_BASE
{
	/* TODO: wrong text type name should say it all
		Got this error?
			error C2039: 'tdc_alt_context' : is not a member of 'T_TDC_only_one__BEGIN_STEP__allowed_per_function'

		you are violating one of the following rules:
		A)
			"ON(...)...;" cannot follow "OTHERWISE()...;"

		B)
			"AWAIT(...)" is not allow in side "ALT{...}" enclose it with an "ON(...)"
			e.g. ALT{... ON(AWAIT(...))...; ...}
		C)
			"FAIL()", "PASS()", "SEND(...)" are not allowed inside "ALT{...}" they must be 
			inside the body of an "ON(AWAIT(something))...;" or "OTHERWISE()...;"

		remember to use curly parantheses "{...}" when the body contain more than one command
	*/
	T_TDC_DYNAMIC_CONTEXT_DATA* parent;
	T_TDC_only_one__BEGIN_STEP__allowed_per_function (T_TDC_DYNAMIC_CONTEXT_DATA& parent_);
};

struct T_TDC_ALT_CONTEXT:T_TDC_CONTEXT_BASE
{
	/* 
		Got this error?
			error C2039: 'tdc_not_alt_context' : is not a member of 'T_TDC_ALT_CONTEXT'

		you are violating one of the following rules:
		A)
			"AWAIT(...)" is not allow in side "ALT{...}" enclose it with an "ON(...)"
			e.g. ALT{... ON(AWAIT(...))...; ...}
		B)
			"FAIL()", "PASS()", "SEND(...)", "ALT{...}" are not allowed inside "ALT{...}" they must be 
			inside the body of an "ON(AWAIT(something))...;" or "OTHERWISE()...;"

		remember to use curly parantheses "{...}" when the body contain more than one command
	*/
	T_TDC_DYNAMIC_CONTEXT_DATA tdc_alt_context;
	T_TDC_ALT_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent_, char* file_, int line_);
	void tdc_end_for();
	bool tdc_enter_for();
};

struct T_TDC_ON_CONTEXT:T_TDC_CONTEXT_BASE
{
	T_TDC_DYNAMIC_CONTEXT_DATA tdc_not_alt_context;
	T_TDC_ON_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent_, char* file_, int line_, char* text_);
	void tdc_end_for ();
	bool tdc_enter_for();
	bool tdc_on_expects_await(T_TDC_AWAIT& await_);
};

struct T_TDC__OTHERWISE__MUST_BE_LAST_IN_ALT:T_TDC_CONTEXT_BASE
{
	/* 
		Got this error?
			error C2039: 'tdc_alt_context' : is not a member of 'T_TDC__OTHERWISE__MUST_BE_LAST_IN_ALT'

		you are violating one of the following rules:
		A)
			"ON(...)...;" cannot follow "OTHERWISE()...;"

		B)
			"AWAIT(...)" is not allow in side "ALT{...}" enclose it with an "ON(...)"
			e.g. ALT{... ON(AWAIT(...))...; ...}
		C)
			"FAIL()", "PASS()", "SEND(...)" are not allowed inside "ALT{...}" they must be 
			inside the body of an "ON(AWAIT(something))...;" or "OTHERWISE()...;"

		remember to use curly parantheses "{...}" when the body contain more than one command
	*/
	T_TDC_DYNAMIC_CONTEXT_DATA* parent;
	T_TDC__OTHERWISE__MUST_BE_LAST_IN_ALT (T_TDC_DYNAMIC_CONTEXT_DATA& parent_);
};

struct T_TDC_OTHERWISE_CONTEXT:T_TDC_CONTEXT_BASE
{
	T_TDC_DYNAMIC_CONTEXT_DATA tdc_not_alt_context;
	T_TDC_OTHERWISE_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA* parent_, char* file_, int line_);
	void tdc_end_for ();
	bool tdc_enter_for ();
};

struct T_TDC_TRAP_CONTEXT:T_TDC_CONTEXT_BASE
{
	T_TDC_DYNAMIC_CONTEXT_DATA tdc_not_alt_context;
	T_TDC_TRAP_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent_, char* file_, int line_);
	void tdc_end_for ();
	bool tdc_trap_testing ();
	bool tdc_initial ();
	bool tdc_enter_for ();
};

struct T_TDC_EVENT_CONTEXT:T_TDC_CONTEXT_BASE
{
  T_TDC_EVENT_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_, char* text_, char* primitive_);
  T_TDC_EVENT_CONTEXT ();
};

//============================================================================

#ifdef TDC_PORT
struct T_PORT
{
  T_PORT operator +(T_PORT& port); // be aware: ("x->y","z") + ("a->b","c") != ("x;a->y;b","z;c")
#if DOT_COMPLETE
  T_ON AWAIT (T_PRIMITIVE_UNION primitive){}
  void SEND (T_PRIMITIVE_UNION primitive){}
#else//DOT_COMPLETE
  char* src_list;
  char* dst_list;
  char* sap_list;
  bool is_send_port; // -> or <-> was specified
  bool is_await_port;// <- or <-> was specified
  T_PORT* next; // used when 2 or more ports have been linked together with operator +
  void construct(char* src_and_dst_list, char* sap_list_);
  T_PORT (char* src_and_dst_list, char* sap_list_);
  T_PORT (char* src_and_dst_list);
  T_PORT (T_PORT& port,T_PORT* port2=0);
  ~T_PORT ();
  T_TDC_AWAIT_CONTEXT tdc_await_context(T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_, char* primitive_);
  T_TDC_SEND_CONTEXT tdc_send_context(T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_, char* primitive_);
#endif//DOT_COMPLETE
};

extern T_PORT DEFAULT_PORT;
#endif

//============================================================================
/// \defgroup AWAIT
//\{

/// Special values that can be awaited for the sake of testing TDC 
enum T_TDC_AWAIT_TESTING {
	TDC_AWAIT_FAIL, /// Hardcode AWAIT to fail
	TDC_AWAIT_PASS, /// Hardcode AWAIT to pass
	TDC_AWAIT_FORCE_PASS, 
};

#if DOT_COMPLETE
int TDC_AWAIT_FAIL;
int TDC_AWAIT_PASS;
int TDC_AWAIT_FORCE_PASS;
#endif

///What an AWAIT returns
struct T_TDC_AWAIT
{
  T_TDC_AWAIT();
private:
  template<class T> T_TDC_AWAIT operator, (T)
  {
    tdc_user_error ("AWAIT can not be part of a comma expression"); // should be courth compiletime as this function is private
    return T_TDC_AWAIT ();
  }
};

struct T_TDC_AWAIT_CONTEXT:T_TDC_EVENT_CONTEXT
{
  friend T_TDC_AWAIT_CONTEXT tdc_await_context (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_, char* primitive_);
#ifdef TDC_PORT
  T_TDC_AWAIT_CONTEXT (T_PORT* port_, T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_, char* primitive_);
#else
  T_TDC_AWAIT_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_, char* primitive_);
#endif
  ~T_TDC_AWAIT_CONTEXT ();
  T_TDC_AWAIT tdc_implement_await (T_TDC_AWAIT_TESTING pass_or_fail);
  T_TDC_AWAIT tdc_implement_await (const T_TDC_INSTANCE_PRIMITIVE_BASE& primitive);
  template<class T> T_TDC_AWAIT tdc_implement_await (T (*f) ())
  {
    T primitive = f ();
    return tdc_implement_await (primitive);
  }
  template<class T> T_TDC_AWAIT tdc_implement_await (void (*f) (T))
  {
    T primitive;
    f (primitive);
    return tdc_implement_await (primitive);
  }
};

//\}

//----------------------------------------------------------------------------
/// \defgroup SEND
//\{

struct T_TDC_SEND_CONTEXT
  :T_TDC_EVENT_CONTEXT
{
  friend T_TDC_SEND_CONTEXT tdc_send_context (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_, char* primitive_);
#ifdef TDC_PORT
  T_TDC_SEND_CONTEXT (T_PORT* port_, T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_, char* primitive_);
#else
  T_TDC_SEND_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_, char* primitive_);
#endif
  ~T_TDC_SEND_CONTEXT ();
  void tdc_implement_send (T_TDC_AWAIT_TESTING pass_or_fail);
  void tdc_implement_send (const T_TDC_INSTANCE_PRIMITIVE_BASE& primitive);
  template<class T>void tdc_implement_send (T (*f) ())
  {
    T primitive = f ();
    return tdc_implement_send (primitive);
  }  
  template<class T>void tdc_implement_send (void (*f) (T))
  {
    T primitive;
    f (primitive);
    return tdc_implement_send (primitive);
  }
};

//\}

//----------------------------------------------------------------------------
/// \defgroup COMMAND COMMAND 
//\{

///What a COMMAND returns
struct T_TDC_COMMAND
{
	T_TDC_COMMAND ();
private:
  ///Hides ',' operator for COMMANDS
	template<class T>
	T_TDC_COMMAND operator, (T)
	{
    tdc_user_error ("COMMAND can not be part of comma expression"); // should be courth compiletime as this function is private		
		return T_TDC_COMMAND();
	}
};

struct T_TDC_COMMAND_CONTEXT:T_TDC_EVENT_CONTEXT
{
	T_TDC_COMMAND_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_,int line_, char* text_);
	T_TDC_COMMAND_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_,int line_, char* text_, char* command_);
	T_TDC_COMMAND_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_,int line_, char* text_, char* value_text_, long value_);
	T_TDC_COMMAND operator || (T_TDC_COMMAND& command_);
};

//\}

/// \defgroup PARKING PARKING Command
//\{

enum T_TDC_PARKING_ENUM /// Values should be indentical to those defined in tap_int.h
{
  DISABLE = 0,
  SHORT_TERM = 1,
  LONG_TERM = 2,
  SHORT_TERM_FAIL = 3,
  LONG_TERM_FAIL = 4
};

struct T_TDC_PARKING_CONTEXT:private T_TDC_COMMAND_CONTEXT
{
	T_TDC_PARKING_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_,int line_, char* value_text_, T_TDC_PARKING_ENUM value_);
  T_TDC_PARKING_CONTEXT (T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_,int line_, char* value_text_, bool value_);
	T_TDC_PARKING_ENUM operator || (T_TDC_PARKING_ENUM value_);
};

//\}

//============================================================================

extern T_TDC_FUNCTION_CONTEXT tdc_syntax_context;

extern T_TDC_DYNAMIC_CONTEXT_DATA* tdc_dynamic_context;

//----------------------------------------------------------------------------

extern void tdc_implement_fail(T_TDC_JMPRET jmpret);

extern void tdc_implement_user_fail(T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_);

extern void tdc_implement_pass();

extern void tdc_implement_user_pass(T_TDC_DYNAMIC_CONTEXT_DATA& parent, char* file_, int line_);

//----------------------------------------------------------------------------

extern T_TDC_COMMAND tdc_implement_command (const char* text);

extern T_TDC_COMMAND tdc_implement_timeout (int time);

extern T_TDC_COMMAND tdc_implement_mute (int time);

extern T_TDC_COMMAND tdc_implement_start_timeout (int time);

extern T_TDC_COMMAND tdc_implement_wait_timeout ();

extern T_TDC_PARKING_ENUM tdc_implement_parking (T_TDC_PARKING_ENUM enabled);
extern T_TDC_PARKING_ENUM tdc_implement_parking (bool enabled);

//============================================================================

#endif //TDC_TESTING

//============================================================================
/// \defgroup TryCatchProtection
//\{

/** 
  map try, catch and throw to our version that prevent the user from using them
  by having both a plain and a "tdc_implement_..." version it is faily simple
  temporary to disable the mapping through #undef when need be
  this is some thing that the user should never do, but tdc have to, so the debugger
  see our longjmps as exceptions

  //TODO: do this stuff debugger see longjumps as exceptions
*/

/// try, catch and throw conflicts with longjmp in vc6, use TRAP
#define try tdc_implement_try		
/// try, catch and throw conflicts with longjmp in vc6, use ELSE
#define catch(exception) tdc_implement_catch(exception)	
/// try, catch and throw conflicts with longjmp in vc6, use FAIL or PASS
#define throw tdc_implement_throw	

/// try, catch and throw conflicts with longjmp in vc6, use TRAP
#define tdc_implement_try while(tdc_try_disabled)		
/// try, catch and throw conflicts with longjmp in vc6, use ELSE
#define tdc_implement_catch(exception) while(tdc_catch_disabled)	
/// try, catch and throw conflicts with longjmp in vc6, use FAIL or PASS
#define tdc_implement_throw while(tdc_throw_disabled)throw	

//\}
//----------------------------------------------------------------------------

#if DOT_COMPLETE //DOT_COMPLETE_PROTOTYPES
#ifndef M_TDC_DOC //we don't want theses to occour twice in the call graph

// dummy functions to generate hint in dot-complete (e.g. when pressing <ctrl-shift-space> )
// remember that functions must have a body to be considered for dot-completion by Visual Studio

void FAIL (void){}

void PASS (void){}

void SEND (T_PRIMITIVE_UNION primitive){}

T_ON AWAIT (T_PRIMITIVE_UNION primitive){}

void COMMAND (char* command_string){}

void TIMEOUT (int timeout){}

void START_TIMEOUT (int timeout){}

void WAIT_TIMEOUT (){}

void MUTE (int timeout){}

T_TDC_PARKING_ENUM PARKING (T_TDC_PARKING_ENUM enable){}

T_similar_syntax_as_while_statement BEGIN_CASE (char* trace_comment){} 

T_similar_syntax_as_while_statement BEGIN_STEP (char* trace_comment){}

T_similar_syntax_as_while_statement ON (T_AWAIT){}

T_similar_syntax_as_while_statement OTHERWISE (){}

#endif//M_TDC_DOC
#else//DOT_COMPLETE_PROTOTYPES

// real implementation

/*
  special constructions used in these macros:

    ensure that only a ; is allowed after this macro
      do {...} while(0)

    make variable declared in for(...) private to the statements ({...} or ...;) after this macro
      if(0);else 

    make implicit return at exit of loop (x might be replace by more code in the actual macros)
      for (int x=0;;x=1) if (x) return ...; else
*/

#define FAIL() do {tdc_implement_user_fail (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__); } while (0)

#define PASS() do {tdc_implement_user_pass (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__); } while (0)

#define SEND(primitive) tdc_send_context(tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__, #primitive).tdc_implement_send(T_PRIMITIVE_UNION(primitive))

#define AWAIT(primitive) tdc_await_context(tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__, #primitive).tdc_implement_await(T_PRIMITIVE_UNION(primitive))

#define START_TIMEOUT(timeout) (T_TDC_COMMAND_CONTEXT (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__, "START_TIMEOUT", #timeout, timeout) || tdc_implement_start_timeout(timeout))

#ifdef WAIT_TIMEOUT 
#undef WAIT_TIMEOUT /* To avoid class with defined macro in winbase.h */
#endif
#define WAIT_TIMEOUT() (T_TDC_COMMAND_CONTEXT (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__, "WAIT_TIMEOUT") || tdc_implement_wait_timeout())

#define MUTE(timeout) (T_TDC_COMMAND_CONTEXT (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__, "MUTE", #timeout, timeout) || tdc_implement_mute(timeout))

#define COMMAND(command) (T_TDC_COMMAND_CONTEXT (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__, #command) || tdc_implement_command(command))

#define TIMEOUT(timeout) (T_TDC_COMMAND_CONTEXT (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__, #timeout) || tdc_implement_timeout(timeout))

#define PARKING(enable) (T_TDC_PARKING_CONTEXT (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__, #enable, enable) || tdc_implement_parking(enable))

// some short cuts where user forgot '()' after '_skip' etc.

#define _skip tdc_skip()
#define _show tdc_show()
#define _forbid tdc_forbid()
#define _require tdc_require()
#define _skip_to_end tdc_skip_to_end()

#define _SKIP tdc_skip()
#define _SHOW tdc_show()
#define _FORBID tdc_forbid()
#define _REQUIRE tdc_require()
#define _SKIP_TO_END tdc_skip_to_end()

#endif//DOT_COMPLETE_PROTOTYPES

#define M_TDC_COMMAND_MUST_BE_LAST_IN_SCOPE true

/** \defgroup BEGIN_CASE BEGIN_CASE(){}
  Usage:
  <pre>
    T_CASE casename () 
    {
      BEGIN_CASE ("trace_comment")
      {
        ...
      }
    }
  </pre>
*/
//\{

/// return type for a test CASE 
#define T_CASE \
	extern "C" __declspec(dllexport) T_TDC_CASE 

/** \def T_TDC_USER_ERROR_CASE
  Test cases defined with T_TDC_USER_ERROR_CASE 
  are cases that passes when they fail and fails when they passes,
  this is only intended for testing TDC it self.
  T_TDC_USER_ERROR_CASE will not reverse the result of a syntax error.

  <pre>
  Usage:
	  T_TDC_USER_ERROR_CASE casename () 
	  {
		  TDC_BEGIN_USER_ERROR_CASE ("trace_comment")
		  {
			  ...
		  }
	  }
  </pre>
*/

/// return type for a test USER ERROR CASE 
#define T_TDC_USER_ERROR_CASE\
  extern "C" __declspec(dllexport) T_TDC_USER_ERROR_CASE_BASE

/// stuff common  to BEGIN_CASE and TDC_BEGIN_USER_ERROR_CASE
#define M_TDC_BEGIN_CASE_BASE(trace_comment,return_value)\
	T_TDC_only_one__BEGIN_CASE__allowed_per_function tdc_begin_case (tdc_syntax_context.tdc_function_context);\
  if(0);else/*hide tdc_syntax_context after for body*/\
	for (T_TDC_CASE_CONTEXT tdc_context (tdc_begin_case.parent,__FILE__,__LINE__,trace_comment),\
			&tdc_syntax_context = ((tdc_context.tdc_not_alt_context.jmpret = T_TDC_JMPRET (setjmp (tdc_context.tdc_not_alt_context.mark))),tdc_context);\
		M_TDC_COMMAND_MUST_BE_LAST_IN_SCOPE;\
		tdc_context.tdc_end_for ())\
	if (tdc_context.tdc_do_return ())\
		return tdc_context.return_value ();\
	else

/// BEGIN_CASE should be the outermost scope in a CASE
#define BEGIN_CASE(trace_comment)\
  M_TDC_BEGIN_CASE_BASE(trace_comment,tdc_return_value)

/// TDC_BEGIN_USER_ERROR_CASE a BEGIN_CASE for T_TDC_USER_ERROR_CASE
#define TDC_BEGIN_USER_ERROR_CASE(trace_comment)\
  M_TDC_BEGIN_CASE_BASE(trace_comment,tdc_user_error_return_value)

//\}		

/** \defgroup BEGIN_STEP BEGIN_STEP(){}
<pre>
  Usage:
	  T_STEP stepname (...) 
	  {
		  BEGIN_STEP ("trace_comment")
		  {
			  ...
		  }
	  }
</pre>
*/
//\{

//TODO: move functionality from tdc_context to tdc_begin_step and delete tdc_context 
#define BEGIN_STEP(trace_comment)\
  T_TDC_only_one__BEGIN_STEP__allowed_per_function tdc_begin_step (tdc_syntax_context.tdc_function_context), &tdc_syntax_context = tdc_begin_step;\
  if (0); else\
  for (T_TDC_STEP_CONTEXT tdc_context (tdc_begin_step.parent,__FILE__,__LINE__,trace_comment),\
      &tdc_syntax_context = ((tdc_context.tdc_not_alt_context.jmpret = T_TDC_JMPRET (setjmp (tdc_context.tdc_not_alt_context.mark))),tdc_context);\
    M_TDC_COMMAND_MUST_BE_LAST_IN_SCOPE;\
    tdc_context.tdc_end_for ())\
  if (tdc_context.tdc_do_return ())\
    return tdc_context.tdc_return_value ();\
  else

//\}

/** \defgroup TRAP_ONFAIL TRAP{} & ONFAIL{}
<pre>
  Usage:
    TRAP
    {
      ...
      FAIL();
    }
    ONFAIL
    {
      ...
    }
</pre>
*/
//\{

#define TRAP\
	if (0); else\
	for (T_TDC_TRAP_CONTEXT tdc_context (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__);\
		tdc_context.tdc_initial () && \
			(tdc_context.tdc_not_alt_context.jmpret = T_TDC_JMPRET (setjmp (tdc_context.tdc_not_alt_context.mark)),\
			tdc_context.tdc_enter_for ());\
		)\
	if (tdc_context.tdc_trap_testing ())\
	for (T_TDC_TRAP_CONTEXT &tdc_syntax_context = tdc_context; ; tdc_syntax_context.tdc_end_for())

#ifdef M_TDC_DOC
#define ONFAIL() //Soure Insight will only draw call gaphs if "()" is present
#else
#define ONFAIL else // we don't want to do something special here, as we can't ensure warning on a plain else
#endif

//\}

/** \defgroup ALT_ON_OTHERWISE ALT{}, ON() & OTHERWISE()
<pre>
  Usage:
    ALT
    {
      ON (AWAIT (...)) ...;
      ON (AWAIT (...)) ...;
      ON (AWAIT (...)) ...;
      OTHERWISE () ...;
    }
</pre>
*/
//\{

#define ALT\
	if (0); else\
	for (T_TDC_ALT_CONTEXT tdc_context (tdc_syntax_context.tdc_not_alt_context, __FILE__, __LINE__), &tdc_syntax_context = tdc_context;\
		tdc_context.tdc_alt_context.jmpret = T_TDC_JMPRET (setjmp (tdc_context.tdc_alt_context.mark)),\
			tdc_context.tdc_enter_for ();\
		tdc_context.tdc_end_for ())

#define ON(await)\
	if (0); else\
	for (T_TDC_ON_CONTEXT tdc_context (tdc_syntax_context.tdc_alt_context,__FILE__,__LINE__,#await), &tdc_syntax_context = tdc_context;\
		tdc_context.tdc_not_alt_context.jmpret = T_TDC_JMPRET (setjmp (tdc_context.tdc_not_alt_context.mark)),\
			tdc_context.tdc_enter_for () && tdc_context.tdc_on_expects_await (await);\
		tdc_context.tdc_end_for ())

#define OTHERWISE()\
	T_TDC__OTHERWISE__MUST_BE_LAST_IN_ALT tdc_otherwise (tdc_syntax_context.tdc_alt_context), &tdc_syntax_context = tdc_otherwise;\
	if (0); else\
	for (T_TDC_OTHERWISE_CONTEXT tdc_syntax_context (tdc_otherwise.parent, __FILE__, __LINE__);\
		tdc_syntax_context.tdc_not_alt_context.jmpret = T_TDC_JMPRET (setjmp (tdc_syntax_context.tdc_not_alt_context.mark)),\
			tdc_syntax_context.tdc_enter_for (), M_TDC_COMMAND_MUST_BE_LAST_IN_SCOPE ;\
		tdc_syntax_context.tdc_end_for ()) 

//\}

//============================================================================

#if TDC_DEBUG_DOT_COMPLETE
void F_TDC_DEBUG_DOT_COMPLETE__TDC_H_1Z()
{
  T_TDC_DEBUG_DOT_COMPLETE__TDC_H_1 s;
  s. 
    i;
  s->i0;
  T_TDC_INTERFACE_ARRAY<int> i;
  i.
}
#else
#endif

//============================================================================

#endif //TDC_BASE_H


