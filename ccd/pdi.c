/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  pdi.c
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

#define PDI_C

#include "typedefs.h"
#include "ccdapi.h"
#include "ccdtable.h"
#include "ccddata.h"
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include "pdi.h"

#define GET_PD(m) ((m)->buf[((m)->o_buf)>>3] & 0x0f)
#define GET_TI(m) ((m)->buf[((m)->o_buf)>>3] >> 4)

/* Constants borrowed from grr.h */
#define CTRL_BLK_NO_OPT     1
#define CTRL_BLK_OPT        2

static T_PDI_DECODEINFO* g_DummyDecodeInfo = NULL;

static T_PDI_DECODEINFO m_def_dinfo[]={
  /* type */   /* attrib */   /* prim */  /* entity */ /* mt */ /* pdi_prepare_ccdmsg */ /* primmbr */
  {PDI_DECODETYPE_L3PDU_N,   "sdu", "PH_*", "", 0xff, NULL, NULL},
  {PDI_DECODETYPE_L3PDU_N,   "sdu", "MPH_*", "", 0xff, NULL, NULL},
  {PDI_DECODETYPE_L3PDU_N,   "sdu", "DL_*", "", 0xff, NULL, NULL},
  {PDI_DECODETYPE_L3PDU,     "sdu", "XX_TAP*", "", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD,      "sdu", "XX_*", "XX", 0xff, NULL, NULL},
  {PDI_DECODETYPE_L3PDU,     "sdu", "*", "", 0xff, NULL, NULL}
};
#define DEF_DINFO_COUNT (sizeof(m_def_dinfo) / sizeof(*m_def_dinfo))

static UBYTE pdi_readmtype (T_MSGBUF* msg, UBYTE len)
{
  UBYTE mt = msg->buf[msg->o_buf>>3] >> (8-len);
  msg->l_buf -= len;
  msg->o_buf += len;
  return mt;
}

#define PDI_MBUFLEN 1024
static struct
{
  U16 l_buf;
  U16 o_buf;
  char buf[PDI_MBUFLEN];
} pdi_msgbuf;

static int pdi_rmac_hdr (T_PDI_CCDMSG* ccdmsg, char* evalue, int evlen)
{
  char* ptr_blk;
  UBYTE payload, rrbp, sp;
  evalue++; /* Assume it is an array and skip length delivered from ccdedit */
  memcpy (pdi_msgbuf.buf, evalue, evlen > PDI_MBUFLEN ? PDI_MBUFLEN : evlen); 
  ptr_blk = evalue;
  pdi_msgbuf.l_buf = evlen * 8;
  pdi_msgbuf.o_buf = 0;
  ccdmsg->mbuf = (T_MSGBUF*) &pdi_msgbuf;

  /* the following is borrowed from grlc_gfff.c */
  payload = (ptr_blk[0] & 0xC0) >> 6;
  rrbp    = (ptr_blk[0] & 0x30) >> 4;
  sp      = (ptr_blk[0] & 0x08) >> 3;

  if (payload == CTRL_BLK_NO_OPT)
  {
    /* msg starts at byte #2 */
    pdi_msgbuf.l_buf -= 8;
    pdi_msgbuf.o_buf += 8;
  }
  else if ((payload == CTRL_BLK_OPT) &&
           !(ptr_blk[1] & 0x01) &&
           !(ptr_blk[1] & 0x80) &&
            (ptr_blk[1] & 0x02))
  {
    /* msg starts at byte #3 */
    pdi_msgbuf.l_buf -= 16;
    pdi_msgbuf.o_buf += 16;
  }
  else if ((payload == CTRL_BLK_OPT) &&
            (ptr_blk[1] & 0x01) &&
           !(ptr_blk[1] & 0x80) &&
            (ptr_blk[1] & 0x02))
  {
    /* msg starts at byte #4 */
    pdi_msgbuf.l_buf -= 24;
    pdi_msgbuf.o_buf += 24;
  }
  else
  {
    return PDI_NONE;
  }
  return PDI_CCDMSG;
}

T_PDI_CONTEXT* CCDDATA_PREF(pdi_createDefContext)()
{
  const T_PDI_DECODEINFO* dinfo;
  int dinfo_count=ccddata_get_pdi_dinfo(&dinfo);
  if (dinfo_count==0)
  {
    dinfo_count=DEF_DINFO_COUNT;
    dinfo=m_def_dinfo;
  }
  return CCDDATA_PREF(pdi_createContext)(dinfo,dinfo_count);
}


T_PDI_CONTEXT* CCDDATA_PREF(pdi_createContext)(const T_PDI_DECODEINFO *dinfo, unsigned int dicount)
{
  int i;
  USHORT sap, opc, dir, di;
  USHORT pmtx;
  T_PDI_DECODEINFO* decodeInfo[1024];
  int decodeInfoCount;
  int len;

  T_PDI_CONTEXT *context;

  if (context = (T_PDI_CONTEXT*)malloc(sizeof(T_PDI_CONTEXT)))
  {
    // copy dinfo
    if (!(context->dinfo = (T_PDI_DECODEINFO*)malloc(sizeof(T_PDI_DECODEINFO)*dicount)))
    {
      free(context);
      return NULL;
    }
    memcpy(context->dinfo, dinfo, sizeof(T_PDI_DECODEINFO)*dicount); 

    // PD -> CCDENT
    memset(context->PdEntityTable, -1, sizeof(T_PDI_PdEntityTable));
    context->PdEntityTable[PD_XX] = ccddata_get_ccdent("XX");
    context->PdEntityTable[PD_CC] = ccddata_get_ccdent("CC");
    context->PdEntityTable[PD_MM] = ccddata_get_ccdent("MM");
    context->PdEntityTable[PD_RR] = ccddata_get_ccdent("RR");
    context->PdEntityTable[PD_GMM] = ccddata_get_ccdent("GMM");
    context->PdEntityTable[PD_SMS] = ccddata_get_ccdent("SMS");
    context->PdEntityTable[PD_SS] = ccddata_get_ccdent("SS");
    context->PdEntityTable[PD_SM] = ccddata_get_ccdent("SM");
    context->PdEntityTable[PD_TST] = ccddata_get_ccdent("TST");

    /* initialize mi_length */
    context->mi_length = ccddata_get_mi_length ();

    // count pcomp
    i = 0;
    while (ccddata_get_pcomp((USHORT)i)->name != NULL) i++;
    context->PrimDecodeInfo = (T_PDI_DECODEINFO***)malloc(i*sizeof(T_PDI_DECODEINFO**));
    memset(context->PrimDecodeInfo, 0, i*sizeof(int*));

    // search all primitives
    for (sap = 0; sap <= ccddata_get_max_sap_num(); sap++)
      for (opc = 0; opc <= (USHORT)ccddata_get_max_primitive_id(); opc++)
        for (dir = 0; dir <= 1; dir++)
          if ((pmtx = ccddata_get_pmtx(sap, opc, dir)) != NO_REF)
          {
            const char* pname;
            pname = ccddata_get_pcomp(pmtx)->name;

            decodeInfoCount = 0;
            for (di = 0; di < dicount; di++)
            {
              int wildcard;
              len = strlen(context->dinfo[di].prim);
              if (context->dinfo[di].prim[len-1] == '*')
              {
                wildcard = 1;
                len--;
              }
              else
              {
                wildcard = 0;
                len = strlen(pname);
              }

              if (wildcard)
              {
                if (!strncmp(context->dinfo[di].prim, pname, len))
                {
                  decodeInfo[decodeInfoCount] = &context->dinfo[di];
                  decodeInfoCount++;
                }
              }
              else
              {
                if (!strcmp(context->dinfo[di].prim, pname))
                {
                  decodeInfo[decodeInfoCount] = &context->dinfo[di];
                  decodeInfoCount++;
                }
              }
            }

            // store decodeInfo for this primitive
            if (decodeInfoCount != 0)
            {
              decodeInfo[decodeInfoCount] = g_DummyDecodeInfo; 
              decodeInfoCount++;
              context->PrimDecodeInfo[pmtx] = (T_PDI_DECODEINFO**)
                           malloc(decodeInfoCount*sizeof(T_PDI_DECODEINFO*));
              if (context->PrimDecodeInfo+pmtx)
              {
                memcpy(context->PrimDecodeInfo[pmtx], &decodeInfo,
                       decodeInfoCount*sizeof(T_PDI_DECODEINFO*));

              }
              else
                context->PrimDecodeInfo[pmtx] = &g_DummyDecodeInfo;
            }
            else
              context->PrimDecodeInfo[pmtx] = &g_DummyDecodeInfo;

          } // endif (pmtx != NO_REF)
  }

  return context;
}



void CCDDATA_PREF(pdi_destroyContext)(T_PDI_CONTEXT *context)
{
  int i = 0;

  if (context==NULL) return;

  while (ccddata_get_pcomp((USHORT)i)->name != NULL)
  {
    if ((context->PrimDecodeInfo[i] != NULL) &&
        (context->PrimDecodeInfo[i][0] != NULL))
      free(context->PrimDecodeInfo[i]);
    i++;
  }
  if (context->PrimDecodeInfo != NULL)
    free(context->PrimDecodeInfo);

  free(context->dinfo);

  free(context);
}



void CCDDATA_PREF(pdi_startPrim)(T_PDI_CONTEXT *context, ULONG opc)
{
  context->sapi = 0;

  if (opc & 0x80000000)
  {
    context->sap = (USHORT) (opc & 0x3fff);
    context->opc = (USHORT) ((opc >> 16) & 0xff);
  }
  else
  {
    context->sap = (USHORT) (((opc & 0x3f00)>>8) & 0xff);
    context->opc = (USHORT) (opc & 0xff);
  }
  context->dir = (UBYTE) (((opc & 0x4000)>>14) & 0x01);

  context->pmtx = ccddata_get_pmtx(context->sap, context->opc, context->dir);
  context->mtypenum = 0;
}

void CCDDATA_PREF(pdi_getDecodeInfo)(T_PDI_CONTEXT *context, const char *ename,
                       char *evalue, int evlen, T_PDI *decinfo)
{
  int i=0;
  T_PDI_DECODEINFO* di;

  decinfo->decodetype = PDI_NONE;

  while (di = context->PrimDecodeInfo[context->pmtx][i++])
  {
    if ((di->type == PDI_DECODETYPE_SAPI) && (strcmp(ename, "sapi") == 0))
    {
      context->sapi = evalue[0];
    }

    if (!strcmp(ename, di->attrib))
    {
      decinfo->pdi.ccdmsg.msg_type = 0xff;

      if (di->pdi_prepare_ccdmsg)
      {
        decinfo->decodetype = (*di->pdi_prepare_ccdmsg)
                                (&decinfo->pdi.ccdmsg, context->mtypeval,
                                 context->mtypenum);
        if (decinfo->decodetype == PDI_NONE)
        {
          continue;
        }
      }

      switch (di->type)
      {
        case PDI_DECODETYPE_AIM:
        case PDI_DECODETYPE_AIM_N:
        case PDI_DECODETYPE_AIM_CHECK:
        case PDI_DECODETYPE_AIM_N_CHECK:
          decinfo->decodetype = PDI_CCDMSG;
          memcpy (pdi_msgbuf.buf, evalue,
                  evlen > PDI_MBUFLEN ? PDI_MBUFLEN : evlen); 
          pdi_msgbuf.l_buf = evlen * 8;
          pdi_msgbuf.o_buf = 0;
          
          /* first byte: don't care */
          pdi_msgbuf.l_buf -= 8;
          pdi_msgbuf.o_buf += 8;
          decinfo->pdi.ccdmsg.mbuf = (T_MSGBUF*) &pdi_msgbuf;
          decinfo->pdi.ccdmsg.pd = GET_PD (decinfo->pdi.ccdmsg.mbuf);
          if (strcmp (di->entity,
                CCDDATA_PREF(pdi_pd2name)(decinfo->pdi.ccdmsg.pd)))
          {
            /* pd does not match the configured entity */
            decinfo->decodetype = PDI_CCDMSG;
            continue;
          }
          else
          {
            pdi_msgbuf.l_buf -= 8;
            pdi_msgbuf.o_buf += 8;
          }
          //decinfo->pdi.ccdmsg.ti = GET_TI (decinfo->pdi.ccdmsg.mbuf);
          decinfo->pdi.ccdmsg.dir = (di->type == PDI_DECODETYPE_AIM) ||
                                    (di->type == PDI_DECODETYPE_AIM_CHECK) ?
                                    context->dir : (~context->dir)&1;
          decinfo->pdi.ccdmsg.entity = (UBYTE)ccddata_get_ccdent(di->entity);
          decinfo->pdi.ccdmsg.msg_type = pdi_readmtype (
                               decinfo->pdi.ccdmsg.mbuf,
                               context->mi_length[decinfo->pdi.ccdmsg.entity]);
          break;
        case PDI_DECODETYPE_L3PDU_N:
        case PDI_DECODETYPE_L3PDU:
          decinfo->decodetype = PDI_CCDMSG;

          decinfo->pdi.ccdmsg.pd = GET_PD ((T_MSGBUF*) evalue);
          decinfo->pdi.ccdmsg.ti = GET_TI ((T_MSGBUF*) evalue);
          decinfo->pdi.ccdmsg.dir = (di->type == PDI_DECODETYPE_L3PDU) ?
                                    context->dir : (~context->dir)&1;

          decinfo->pdi.ccdmsg.mbuf = (T_MSGBUF*)evalue;

          if (CCDDATA_PREF(pdi_getEntityByPD)(context, decinfo->pdi.ccdmsg.pd)==-1)
          {
            decinfo->decodetype = PDI_NONE;
          }
          else
          {
            decinfo->pdi.ccdmsg.entity = (UBYTE)CCDDATA_PREF(pdi_getEntityByPD)(context, decinfo->pdi.ccdmsg.pd);

            ((T_MSGBUF*)evalue)->o_buf += 8;
            ((T_MSGBUF*)evalue)->l_buf -= 8;

            decinfo->pdi.ccdmsg.msg_type = pdi_readmtype ((T_MSGBUF*)evalue,
                               context->mi_length[decinfo->pdi.ccdmsg.entity]);
          }

          /* remove SSN bit */
          if (!strcmp ("DL_DATA_REQ", ccddata_get_pcomp (context->pmtx)->name))
          {
            if (decinfo->pdi.ccdmsg.pd == PD_CC ||
                decinfo->pdi.ccdmsg.pd == PD_MM ||
                decinfo->pdi.ccdmsg.pd == PD_SS)
            {
              decinfo->pdi.ccdmsg.msg_type &= ~0x40;
            }
          }
          break;

        case PDI_DECODETYPE_NOPD:
        case PDI_DECODETYPE_NOPD_N:
        case PDI_DECODETYPE_RR_SHORT:
          decinfo->decodetype = PDI_CCDMSG;

          decinfo->pdi.ccdmsg.pd = 0;          
          decinfo->pdi.ccdmsg.ti = 0;
          decinfo->pdi.ccdmsg.dir = (di->type == PDI_DECODETYPE_NOPD) ?
                                    context->dir : (~context->dir)&1;

          decinfo->pdi.ccdmsg.mbuf = (T_MSGBUF*)evalue;
          decinfo->pdi.ccdmsg.entity = (UBYTE)ccddata_get_ccdent(di->entity);
          decinfo->pdi.ccdmsg.msg_type = pdi_readmtype ((T_MSGBUF*)evalue,
                               context->mi_length[decinfo->pdi.ccdmsg.entity]);
          break;

        case PDI_DECODETYPE_NOPD_NOTYPE:
        case PDI_DECODETYPE_NOPD_NOTYPE_N:
          decinfo->decodetype = PDI_CCDMSG;

          decinfo->pdi.ccdmsg.pd = 0;          
          decinfo->pdi.ccdmsg.ti = 0;
          decinfo->pdi.ccdmsg.dir = (di->type == PDI_DECODETYPE_NOPD_NOTYPE) ?
                                    context->dir : (~context->dir)&1;

          decinfo->pdi.ccdmsg.mbuf = (T_MSGBUF*)evalue;

          if (decinfo->pdi.ccdmsg.msg_type == 0xff)
          {
            decinfo->pdi.ccdmsg.msg_type = di->msg_type;
          }
          
          decinfo->pdi.ccdmsg.entity = (UBYTE)ccddata_get_ccdent(di->entity);
          break;

        case PDI_DECODETYPE_MAC_H:
        case PDI_DECODETYPE_MAC_H_N:
        case PDI_DECODETYPE_MAC_H_CHECK:
        case PDI_DECODETYPE_MAC_H_N_CHECK:
          if ((decinfo->decodetype = pdi_rmac_hdr (&decinfo->pdi.ccdmsg,
                                               evalue, evlen)) == PDI_CCDMSG)
          {
            decinfo->pdi.ccdmsg.pd = 0;          
            decinfo->pdi.ccdmsg.ti = 0;
            decinfo->pdi.ccdmsg.dir = ((di->type == PDI_DECODETYPE_MAC_H) ||
                                       (di->type == PDI_DECODETYPE_MAC_H_CHECK))
                                       ? context->dir : (~context->dir)&1;
            decinfo->pdi.ccdmsg.entity = (UBYTE)ccddata_get_ccdent(di->entity);
            decinfo->pdi.ccdmsg.msg_type =
              pdi_readmtype (decinfo->pdi.ccdmsg.mbuf,
                               context->mi_length[decinfo->pdi.ccdmsg.entity]);
          }
          break;

        case PDI_DECODETYPE_SAPI:
          decinfo->decodetype = PDI_NONE;
          if (context->sapi == 1)  // only sapi1 (GMM) has data for ccd
          {
            decinfo->decodetype = PDI_CCDMSG;

            decinfo->pdi.ccdmsg.pd = GET_PD ((T_MSGBUF*) evalue);
            decinfo->pdi.ccdmsg.ti = GET_TI ((T_MSGBUF*) evalue);
            decinfo->pdi.ccdmsg.dir = context->dir;

/* !!! TBD  !!! */
          /* find msg_type*/
/* !!! TBD  !!! */

            decinfo->pdi.ccdmsg.mbuf = (T_MSGBUF*)evalue;

            ((T_MSGBUF*)evalue)->o_buf += 8;
            ((T_MSGBUF*)evalue)->l_buf -= 8;

            if (CCDDATA_PREF(pdi_getEntityByPD)(context, decinfo->pdi.ccdmsg.pd)==-1)
            {
              decinfo->decodetype = PDI_NONE;
            }
            else
            {
              decinfo->pdi.ccdmsg.entity = (UBYTE)CCDDATA_PREF(pdi_getEntityByPD)(context, decinfo->pdi.ccdmsg.pd);
            }
          }
          break;

        default:
          decinfo->decodetype = PDI_NONE;
      } 

      break;
    } /* endif (strcmp) */
    else
    {
      if (evlen > 4 || evlen < 0 || evlen == 3)
      {
        /* don't check prim members for non base types */
        continue;
      }
      /* check for prim members later needed for finding out msg type */
      if (di->primmbr)
      {
        int i;
        for (i=0; di->primmbr[i] && context->mtypenum<PDI_MAXPMEMFORMTYPE; i++)
        {
          if (!strcmp(ename, di->primmbr[i]))
          {
            switch (evlen)
            {
              case 1:
                context->mtypeval[context->mtypenum++] =
                  (ULONG) * (UBYTE*) evalue;
                break;
              case 2:
                context->mtypeval[context->mtypenum++] =
                  (ULONG) * (USHORT*) evalue;
                break;
              case 4:
              default:
                context->mtypeval[context->mtypenum++] = * (ULONG*) evalue;
                break;
            }
          }
        }
      }
    }
  } /* endwhile */

}

short CCDDATA_PREF(pdi_getEntityByPD)(const T_PDI_CONTEXT *context, unsigned char pd)
{
  if ((pd > 16) || (context->PdEntityTable == NULL))
    return -1;
  else
    return context->PdEntityTable[pd];
}


const char* CCDDATA_PREF(pdi_pd2name)(unsigned char pd)
{
  switch (pd) {
    case PD_XX: return "XX";
    case PD_CC: return "CC";
    case PD_MM: return "MM";
    case PD_RR: return "RR";
    case PD_GMM: return "GMM";
    case PD_SMS: return "SMS";
    case PD_SS: return "SS";
    case PD_SM: return "SM";
    case PD_TST: return "TST";

    default: return "??";
  }
}
