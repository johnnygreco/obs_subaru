/*****************************************************************************
******************************************************************************
**
** FILE:
**	hdr.c
**
** ABSTRACT:
**	This file contains routines that manipulate header information
**	for the "Pure C" layer.
**
** ENTRY POINT		       SCOPE   DESCRIPTION
** -------------------------------------------------------------------------
** p_shHdrFree      	       private Free the header buffers (not the vector)
** p_shHdrMallocForVec         private Allocate space for the header vector
** p_shHdrFreeForVec	       private Free the header vector
** p_shHdrPrint		       private Print header vector to stdout
** shHdrNew                    public  Malloc space for a new header
** shHdrDel                    public  Delete the space used by a header
** shHdrGetAscii	       public  Search header for ASCII value
** shHdrGetDbl                 public  Search header for DOUBLE value
** shHdrGetInt                 public  Search header for INTEGER value
** shHdrGetLogical             public  Search header for LOGICAL value
** shHdrGetLine                public  Search header for keyword and return 
**                                        whole line on which it was found
** shHdrGetLinno               public  Search header for keyword and return 
**                                        line number on which it was found
** shHdrGetLineTotal           public  Return the total number of lines in the
**                                        header.
** shHdrDelByLine              public  Delete indicated line number from header
** shHdrInsertLine             public  Insert a new line in the header
** shHdrReplaceLine            public  Replace an existing line in the header
** shHdrInsertLogical          public  Insert a LOGICAL value in the header
** shHdrInsertDbl              public  Insert a DOUBLE value in the header
** shHdrInsertInt              public  Insert an INTEGER value in the header
** shHdrInsertAscii            public  Insert an ASCII value in the header
** shHdrMakeLineWithAscii      public  Construct a line suitable for inclusion
**                                        in header containing an ASCII value
** shHdrMakeLineWithDbl        public  Construct a line suitable for inclusion
**                                        in header containing a DOUBLE value
** shHdrMakeLineWithInt        public  Construct a line suitable for inclusion
**                                        in header containing an INTEGER value
** shHdrMakeLineWithLogical    public  Construct a line suitable for inclusion
**                                        in header containing a LOGICAL value
** shHdrCopy                   public  Copy a header from one to another
** shHdrInit                   private Initializes the header
** shHdrDelByKeyword           public  Deletes header line w/ keyword specified
**
** p_shHdrCntrIncrement        private Increment header modification count
**
** ENVIRONMENT:
**	ANSI C.
**
** AUTHORS:
**	Creation date:  May 7, 1993
**	   Vijay K. Gurbani
**
** MODIFICATIONS:
** 05/14/1993   VKG   Modified function names according to convention
** 05/20/1993   VKG   Modified shRegHdrInit() to create a header if none exists
** 05/24/1993   VKG   Renamed functions shRegHdrFree, shRegHdrMallocForVec, and
**                       shRegHdrFreeForVec to p_shRegHdrFree, 
**                       p_shRegHdrMallocForVec, and p_shRegHdrFreeForVec. This
**                       implies that these are private functions now.
** 05/24/1993   VKG   Changed name of shRegHdr to shRegHdrAccess
** 05/26/1993   VKG   Modified shRegHdrInit so that it does not populate the
**                       header with default lines
** 05/26/1993   VKG   Made shRegHdrInit private
** 01/17/1994	THN   Converted from REGION-specific to generic header code.
** 03/07/1994   EFB   Changed SHHDR to HDR and SHHDR_VEC HDR_VEC
** 03/08/1994   EFB   Added shHdrNew and shHdrDel
******************************************************************************
******************************************************************************
*/
#include <stdio.h>
#include <string.h>

#include "shCUtils.h"
#include "region.h"
#include "prvt/region_p.h"
#include "shCFitsIo.h"
#include "shCGarbage.h"
#include "libfits.h"
#include "shCHdr.h"

/*
 * Functions local to this source file only.
 */
static void p_shHdrCntrIncrement(HDR *a_hdr);
static RET_CODE shHdrInit(HDR *a_hdr);


/*============================================================================
 * ROUTINE: p_shHdrCntrIncrement
 *
 * DESCRIPTION:
 *   This routine increments a header modification counter each time the header
 *   is modified.
 *
 * CALL:
 *   (void) p_shHdrCntrIncrement(HDR *a_hdr);
 *          a_hdr - pointer to the header structure
 *============================================================================
 */
static void p_shHdrCntrIncrement(HDR *a_hdr)
{
   if (a_hdr != NULL)
       a_hdr->modCnt += 1;

   return;
}

/*============================================================================
 * ROUTINE: shHdrInit
 *
 * DESCRIPTION:
 *   This routine initializes a FITS header. 
 *
 * CALL:
 *   (RET_CODE) shHdrInit(HDR *a_hdr);
 *                  a_hdr    - pointer to the header structure
 *
 * CALLS TO:
 *   p_shHdrFree()
 *   p_shHdrCntrIncrement()
 *   p_shHdrMallocForVec()
 *
 * RETURN VALUES:
 *      SH_HEADER_INSERTION_ERROR - Failed to initialize header
 *      SH_SUCCESS                - Succeeded
 *============================================================================
 */
static RET_CODE shHdrInit(HDR *a_hdr)
{

   if (a_hdr->hdrVec == NULL)    /* Create a header if none exists */
       p_shHdrMallocForVec(a_hdr);
   else
       p_shHdrFree(a_hdr);      /* De-allocate previous header, if exists */

   p_shHdrCntrIncrement(a_hdr);
 
   return SH_SUCCESS;
}

/*============================================================================
 * ROUTINE: p_shHdrFree
 *
 * DESCRIPTION:
 *    Free the header buffer and remove all traces of it from the header
 *    vector.
 *
 * CALL:
 *   (void) p_shHdrFree(HDR *a_hdr);
 *              a_hdr    - pointer to the header structure
 *
 * CALLS TO:
 *   p_shHdrCntrIncrement()
 *============================================================================
 */
void p_shHdrFree(HDR *a_hdr)
{
/*###*   struct region_p *rp;
 *###*
 *###*   rp = a_reg->prvt;
 *###*/

   if ((a_hdr->hdrVec != NULL) && (a_hdr->hdrVec[0] != NULL))  {
/*###*/       /*
 *###*        * If this is a physical header, and a free procedure for it is defined,
 *###*        * then call it
 *###*        */
/*###*       if ((rp->type == SHREGPHYSICAL) && (rp->phys) &&
 *###*           (rp->phys->physHdrFree))  
 *###*/           /*
 *###*            * The first element of the header structure contains the starting
 *###*            * address of the physical header
 *###*            */
/*###*           (*(rp->phys->physHdrFree))(rp->phys->physIndex, a_hdr->hdrVec[0]);
 *###*/
       /*
        * Now, for each physical and virtual region, free each of the header
        * lines. The routines responsible for populating these header lines
        * always malloc memory for them
        * Note: do not free the vector.
        */
       f_hdel(a_hdr->hdrVec);
       p_shHdrCntrIncrement(a_hdr);
   }

   return;
}

/*============================================================================
 * ROUTINE: p_shHdrMallocForVec
 *
 * DESCRIPTION:
 *   Routine to allocate the base memory for a header vector
 *
 * CALL:
 *   (void) p_shHdrMallocForVec(HDR *a_hdr);
 *          a_hdr    - pointer to the header structure
 *
 * CALLS TO:
 *   shMalloc()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   Nothing
 *============================================================================
 */
void p_shHdrMallocForVec(HDR *a_hdr)
{
   int i;

   /*
    * shMalloc aborts if there is no memory on the heap. So, don't bother
    * with sending a return value if a_hdr->hdrVec is NULL after a call to
    * shMalloc()
    */
   a_hdr->hdrVec = ((HDR_VEC *)shMalloc((MAXHDRLINE + 1) * sizeof(HDR_VEC)));

   for (i = 0; i <= MAXHDRLINE; i++)
        a_hdr->hdrVec[i] = NULL;

   p_shHdrCntrIncrement(a_hdr);

   return;
}

/*============================================================================
 * ROUTINE: p_shHdrFreeForVec
 *
 * DESCRIPTION:
 *   Routine to de-allocate the base memory for a header vector
 *
 * CALL:
 *   (void) p_shHdrFreeForVec(HDR *a_hdr);
 *          a_hdr    - pointer to the header structure
 *
 * CALLS TO:
 *   shFree()
 *   p_shHdrCntrIncrement()
 *============================================================================
 */
void p_shHdrFreeForVec(HDR *a_hdr)
{
   if (a_hdr->hdrVec != NULL)  {
       p_shHdrFree(a_hdr);
       shFree(a_hdr->hdrVec);
       a_hdr->hdrVec = NULL;
       p_shHdrCntrIncrement(a_hdr);
   }
}

void		 p_shHdrPrint
   (
   const HDR		*a_hdr		/* IN:    Header to print             */
   )

/*++
 * ROUTINE:	 p_shHdrPrint
 *
 *	Print a header vector's contents to stdout.
 *
 * RETURN VALUES:	None
 *
 * SIDE EFFECTS:	None
 *
 * SYSTEM CONCERNS:	None
 *--
 ******************************************************************************/

   {	/* p_shHdrPrint */

   /*
    * Declarations.
    */

   register HDR_VEC		*hdrVec;
   register unsigned long  int	 hdrVecIdx;

   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

   if (a_hdr == ((HDR *)0))
      {
      goto rtn_return;
      }

   if ((hdrVec = a_hdr->hdrVec) != ((HDR_VEC *)0))
      {
      for (hdrVecIdx = 0; hdrVec[hdrVecIdx] != NULL; hdrVecIdx++)
         {
         printf ("%s\n",  hdrVec[hdrVecIdx]);
         }
      }

rtn_return : ;

   return;

   }	/* p_shHdrPrint */

/******************************************************************************/

/*============================================================================
 * ROUTINE: shHdrNew
 *
 * DESCRIPTION:
 *   Malloc the space necessary for a new header.
 *
 * CALL:
 *   (HDR *)shHdrNew();
 *
 * CALLS TO:
 *   shMalloc
 *   p_shHdrMallocForVec
 * 
 * RETURN VALUES:
 *   On success a pointer to the new header structure.  Else NULL.
 *============================================================================
 */
HDR *shHdrNew(void)
{
   HDR *localHdr;

   /* Get space for the header structure itself.  shMalloc aborts if there is
      no space on the heap, so no need to check the return */
   localHdr = (HDR *)shMalloc(sizeof(HDR));

   /* Now allocate space for the header vector. */
   p_shHdrMallocForVec(localHdr);

   return(localHdr);
}

/*============================================================================
 * ROUTINE: shHdrDel
 *
 * DESCRIPTION:
 *   Delete the space taken up by a header.
 *
 * CALL:
 *   (void )shHdrDel(HDR *);
 *
 * CALLS TO:
 *   p_shHdrFreeForVec
 *   shFree
 * 
 * RETURN VALUES:
 *   NONE
 *============================================================================
 */
void shHdrDel(HDR *a_Hdr)
{
   /* Free the space used by the header vector and the header lines. */
   p_shHdrFreeForVec(a_Hdr);

   /* Now free the header structure itself */
   shFree((char *)a_Hdr);
}

/*============================================================================
 * ROUTINE: shHdrGetAscii
 *
 * DESCRIPTION:
 *   Get an alphanumeric keyword string from a FITS header. This routine is
 *   basically a wrapper around the libfits module f_akey.
 *
 * CALL:
 *   (RET_CODE) shHdrGetAscii(HDR *a_hdr, const char *a_keyword,char *a_dest);
 *          a_hdr     - pointer to the header structure
 *          a_keyword - pointer to the keyword to search by
 *          a_dest    - pointer to the destination buffer. It is the user's 
 *                         responsibility to make sure that the size of this
 *                         buffer is big enough to hold the result.
 *
 * CALLS TO:
 *   f_akey()
 * 
 * RETURN VALUES:
 *   SH_SUCCESS         - on success
 *   SH_HEADER_IS_NULL  - if header member is NULL
 *   SH_GENERIC_ERROR   - otherwise
 *============================================================================
 */
RET_CODE shHdrGetAscii(const HDR *a_hdr, const char *a_keyword, char *a_dest)

{
   if (a_hdr->hdrVec == NULL)
       return SH_HEADER_IS_NULL;

   if (f_akey(a_dest, (char **) a_hdr->hdrVec, (char *)a_keyword))
       return SH_SUCCESS;

   return SH_GENERIC_ERROR;
}

/*============================================================================
 * ROUTINE: shHdrGetDbl
 *
 * DESCRIPTION:
 *   Get the value of a double precision keyword from a FITS header. This 
 *   routine is basically a wrapper around the libfits module f_dkey.
 *
 * CALL:
 *   (RET_CODE) shHdrGetDbl(HDR *a_hdr, const char *a_keyword,double *a_dest);
 *              a_hdr     - pointer to the header structure
 *              a_keyword - pointer to the keyword to search by
 *              a_dest    - pointer to the destination buffer. 
 *
 * CALLS TO:
 *   f_dkey()
 * 
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_HEADER_IS_NULL - if header member is NULL
 *   SH_GENERIC_ERROR  - otherwise
 *============================================================================
 */
RET_CODE shHdrGetDbl(const HDR *a_hdr, const char *a_keyword, double *a_dest)

{
   if (a_hdr->hdrVec == NULL)
       return SH_HEADER_IS_NULL;

   if (f_dkey(a_dest, (char **) a_hdr->hdrVec, (char *) a_keyword))
       return SH_SUCCESS;

   return SH_GENERIC_ERROR;
}

/*============================================================================
 * ROUTINE: shHdrGetInt
 *
 * DESCRIPTION:
 *   Get an integer keyword value from a FITS header. This routine is basically
 *   a wrapper around the libfits module f_ikey.
 *
 * CALL:
 *   (RET_CODE) shHdrGetInt(HDR *a_hdr, const char *a_keyword, int *a_dest);
 *          a_hdr     - pointer to the header structure
 *          a_keyword - pointer to the keyword to search by
 *          a_dest    - pointer to the destination buffer. 
 *
 * CALLS TO:
 *   f_ikey()
 * 
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_HEADER_IS_NULL - if header member is NULL
 *   SH_GENERIC_ERROR  - otherwise
 *============================================================================
 */
RET_CODE shHdrGetInt(const HDR *a_hdr, const char *a_keyword, int *a_dest)

{
   if (a_hdr->hdrVec == NULL)
       return SH_HEADER_IS_NULL;

   if (f_ikey(a_dest, (char **) a_hdr->hdrVec, (char *) a_keyword))
       return SH_SUCCESS;

   return SH_GENERIC_ERROR;
}

/*============================================================================
 * ROUTINE: shHdrGetLogical
 *
 * DESCRIPTION:
 *   Get a logical keyword value from a FITS header. This routine is basically
 *   a wrapper around the libfits module f_lkey.
 *
 * CALL:
 *   (RET_CODE) shHdrGetLogical(HDR *a_hdr, const char *a_keyword,
 *                              int *a_dest);
 *          a_hdr     - pointer to the header structure
 *          a_keyword - pointer to the keyword to search by
 *          a_dest    - pointer to the destination buffer. 
 *
 * CALLS TO:
 *   f_lkey()
 * 
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_HEADER_IS_NULL - if the header is NULL
 *   SH_GENERIC_ERROR  - otherwise
 *============================================================================
 */
RET_CODE shHdrGetLogical(const HDR *a_hdr, const char *a_keyword, int *a_dest)

{
   if (a_hdr->hdrVec == NULL)
       return SH_HEADER_IS_NULL;

   if (f_lkey(a_dest, (char **)a_hdr->hdrVec, (char *)a_keyword))
       return SH_SUCCESS;

   return SH_GENERIC_ERROR;
}

/*============================================================================
 * ROUTINE: shHdrGetLine
 *
 * DESCRIPTION:
 *   Get a line identified by the given keyword from a FITS header. This 
 *   routine is basically a wrapper around the libfits module f_getlin.
 *
 * CALL:
 *   (RET_CODE) shHdrGetLine(HDR *a_hdr, const char *a_keyword, char *a_dest);
 *          a_hdr     - pointer to the header structure
 *          a_keyword - pointer to the keyword to search by
 *          a_dest    - pointer to the destination buffer. It is the caller's
 *                         responsibility to ensure that a_dest is large
 *                         enough to hold the line
 *
 * CALLS TO:
 *   f_getlin()
 *
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_HEADER_IS_NULL - if the header is NULL
 *   SH_GENERIC_ERROR  - otherwise
 *============================================================================
 */
RET_CODE shHdrGetLine(const HDR *a_hdr, const char *a_keyword, char *a_dest)
{
   if (a_hdr->hdrVec == NULL)
       return SH_HEADER_IS_NULL;

   if (f_getlin(a_dest, (char **)a_hdr->hdrVec, (char *)a_keyword))
       return SH_SUCCESS;

   return SH_GENERIC_ERROR;
}

/*============================================================================
 * ROUTINE: shHdrGetLineno
 *
 * DESCRIPTION:
 *   Search the header for the keyword specified. If found, stuff the
 *   corresponding line number in a_dest.
 *
 * CALL:
 *   (RET_CODE) shHdrGetLineno(HDR *a_hdr, const char *a_keyword,int *a_dest);
 *          a_hdr     - pointer to the header structure
 *          a_keyword - pointer to the keyword to search by
 *          a_dest    - pointer to the destination buffer.
 *
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_HEADER_IS_NULL - if the header is NULL
 *   SH_GENERIC_ERROR  - otherwise
 *============================================================================
 */
RET_CODE shHdrGetLineno(const HDR *a_hdr, const char *a_keyword, int *a_dest)
{
   RET_CODE rc;
   int      i,
            keyword_length;

   if (a_hdr->hdrVec == NULL)
       return SH_HEADER_IS_NULL;   /* No point in going further */

   rc = SH_GENERIC_ERROR;     /* Let's be pessimistic for a change */
   keyword_length = strlen(a_keyword);

   for (i = 0; i < MAXHDRLINE; i++)   {
        if (a_hdr->hdrVec[i] == NULL)
            break;
        if (strncmp(a_keyword, a_hdr->hdrVec[i], keyword_length) == 0)  {
            *a_dest = i;
            rc = SH_SUCCESS;
            break;
        }
   }

   return rc;
}

/*============================================================================
 * ROUTINE: shHdrGetLineTotal
 *
 * DESCRIPTION:
 *   Search the header until a nil is reached, signaling the end of the header.
 *   Return the total number of lines in the header.
 *
 * CALL:
 *   (RET_CODE) shHdrGetLineTotal(HDR *a_hdr, int *a_lineTotal);
 *          a_hdr       - pointer to the header structure
 *          a_lineTotal - pointer to the total number of lines
 *
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *============================================================================
 */
RET_CODE shHdrGetLineTotal(const HDR *a_hdr, int *a_lineTotal)
{
   RET_CODE rc;
   int      i;

   rc = SH_SUCCESS;
   if (a_hdr->hdrVec != NULL) {
      for (i = 0; i < MAXHDRLINE; i++)   {
        if (a_hdr->hdrVec[i] == NULL)
            break;
      }
      *a_lineTotal = i;
   }
   else {
      *a_lineTotal = 0;
   }

   return rc;
}

/*============================================================================
 * ROUTINE: shHdrGetLineCont
 *
 * DESCRIPTION:
 *   Return the contents of a header line.
 *
 * CALL:
 *   (RET_CODE) shHdrGetLineCont(HDR *a_hdr, int a_line, char *a_dest);
 *          a_hdr   - pointer to the header structure
 *          a_line  - Line number to return contents
 *          a_dest  - pointer to the destination string buffer
 *
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_GENERIC_ERROR  - if line number out of bounds
 *   SH_HEADER_IS_NULL - if the header is NULL
 *============================================================================
 */
RET_CODE shHdrGetLineCont(const HDR *a_hdr, const int a_line, char *a_dest)
{
   if (a_hdr->hdrVec == NULL)
       return SH_HEADER_IS_NULL;   /* No header */

   if (a_line < 0 || a_line >= MAXHDRLINE) return SH_GENERIC_ERROR;

   if (a_hdr->hdrVec[a_line] != NULL)
      {strcpy(a_dest, a_hdr->hdrVec[a_line]);}
   else
      {a_dest[0] = '\0';}

   return SH_SUCCESS;
}

/*============================================================================
 * ROUTINE: shHdrDelByLine
 *
 * DESCRIPTION:
 *   Delete the line identified by a_lineNo from the header. This routine is
 *   basically a wrapper around the libfits routine f_hldel.
 *
 * CALL:
 *   (RET_CODE) shHdrDelByLine(HDR *a_hdr, const int a_lineNo);
 *          a_hdr     - pointer to the header structure
 *          a_lineNo  - line number to be deleted
 *
 * CALLS TO:
 *   f_hldel()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_HEADER_IS_NULL - if the header is NULL
 *   SH_GENERIC_ERROR  - otherwise
 *============================================================================
 */
RET_CODE shHdrDelByLine(HDR *a_hdr, const int a_lineNo)
{
   if (a_hdr->hdrVec == NULL)
       return SH_HEADER_IS_NULL;

   if (f_hldel(a_hdr->hdrVec, a_lineNo))
       return SH_SUCCESS;

   p_shHdrCntrIncrement(a_hdr);

   return SH_GENERIC_ERROR;
}

/*============================================================================
 * ROUTINE: shHdrInsertLine
 *
 * DESCRIPTION:
 *   Insert a_line in the FITS header after line a_lineNo. This routine is
 *   basically a wrapper around the libfits module f_hlins.
 *
 * CALL:
 *   (RET_CODE) shHdrInsertLine(HDR *a_hdr, const int a_lineNo,
 *                                    const char *a_line);
 *          a_hdr     - pointer to the header structure
 *          a_lineNo  - line number after which to insert the new line
 *          a_line    - pointer to the new line to be inserted
 *
 * CALLS TO:
 *   f_hlins()
 *   p_shHdrMallocForVec()
 *   shHdrInit()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   SH_SUCCESS                - on success
 *   SH_HEADER_INSERTION_ERROR - otherwise
 *============================================================================
 */
RET_CODE shHdrInsertLine(HDR *a_hdr, const int a_lineNo, const char *a_line)
{
   /*
    * If there is no header vector, create one by reserving memory for it and
    * insert the given line into it. If there is already a header, simply insert
    * the line in.
    */
   if (a_hdr->hdrVec == NULL)  {
       p_shHdrMallocForVec(a_hdr); 
       if ((shHdrInit(a_hdr)) == 0)
            return SH_HEADER_INSERTION_ERROR;
   }

   if (f_hlins((char **)a_hdr->hdrVec, MAXHDRLINE, a_lineNo,
               (char *)a_line))
   {
       p_shHdrCntrIncrement(a_hdr);
       return SH_SUCCESS;
   }

   return SH_HEADER_INSERTION_ERROR;
}

/*============================================================================
 * ROUTINE: shHdrInsertLogical
 *
 * DESCRIPTION:
 *   Insert a logical value in the FITS header. This routine is basically
 *   a wrapper around the libfits module f_hlins.
 *
 * CALL:
 *   (RET_CODE) shHdrInsertLogical(HDR *a_hdr, const char *a_key,
 *                                 const int a_value, const char *a_comment);
 *          a_hdr     - pointer to the header structure
 *          a_key     - Keyword 
 *          a_value   - Value for the logical
 *          a_comment - Comments (can be blank or NULL)
 *
 * CALLS TO:
 *   p_shHdrMallocForVec()
 *   shHdrInit()
 *   f_hlins()
 *   f_mklkey()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   SH_SUCCESS                - on success
 *   SH_HEADER_INSERTION_ERROR - otherwise
 *============================================================================
 */
RET_CODE shHdrInsertLogical(HDR *a_hdr, const  char *a_key,
                            const  int  a_value, const  char *a_comment)
{
   const char *comment;
   char  string[HDRLINESIZE];
   int   rc;

   /*
    * If there is no header vector, create one by reserving memory for it and
    * insert the given line into it. If there is already a header, simply insert
    * the line in.
    */
   if (a_hdr->hdrVec == NULL)  {
       p_shHdrMallocForVec(a_hdr);
       if ((shHdrInit(a_hdr)) == 0)
            return SH_HEADER_INSERTION_ERROR;
   }

   if (a_comment == NULL || *a_comment == 0)
       comment = "";
   else
       comment = a_comment;

   rc = f_hlins(a_hdr->hdrVec, MAXHDRLINE, MAXHDRLINE + 1,
                f_mklkey(string, (char *)a_key, (int)a_value, 
                         (char *)comment));
   if (!rc)
       return SH_HEADER_INSERTION_ERROR;

   p_shHdrCntrIncrement(a_hdr);

   return SH_SUCCESS;
}

/*============================================================================
 * ROUTINE: shHdrInsertDbl
 *
 * DESCRIPTION:
 *   Insert a double value in the FITS header. This routine is basically
 *   a wrapper around the libfits module f_hlins.
 *
 * CALL:
 *   (RET_CODE) shHdrInsertLogical(HDR *a_hdr, const char *a_key,
 *                                    const double a_value, 
 *                                    const char *a_comment);
 *          a_hdr     - pointer to the header structure
 *          a_key     - Keyword 
 *          a_value   - Value for the double
 *          a_comment - Comments (can be blank or NULL)
 *
 * CALLS TO:
 *   p_shHdrMallocForVec()
 *   shHdrInit()
 *   f_hlins()
 *   f_mkdkey()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   SH_SUCCESS                - on success
 *   SH_HEADER_INSERTION_ERROR - otherwise
 *============================================================================
 */
RET_CODE shHdrInsertDbl(HDR *a_hdr, const char *a_key,
                           const double a_value, const char *a_comment)
{
   const char *comment;
   char  string[HDRLINESIZE];
   int   rc;

   /*
    * If there is no header vector, create one by reserving memory for it and
    * insert the given line into it. If there is already a header, simply insert
    * the line in.
    */
   if (a_hdr->hdrVec == NULL)  {
       p_shHdrMallocForVec(a_hdr);
       if ((shHdrInit(a_hdr)) == 0)
            return SH_HEADER_INSERTION_ERROR;
   }

   if (a_comment == NULL || *a_comment == 0)
       comment = "";
   else
       comment = a_comment;

   rc = f_hlins(a_hdr->hdrVec, MAXHDRLINE, MAXHDRLINE + 1,
                f_mkdkey(string, (char *)a_key, (double)a_value,
                         (char *)comment));
   if (!rc)
       return SH_HEADER_INSERTION_ERROR;

   p_shHdrCntrIncrement(a_hdr);

   return SH_SUCCESS;
}

/*============================================================================
 * ROUTINE: shHdrInsertInt
 *
 * DESCRIPTION:
 *   Insert a int value in the FITS header. This routine is basically
 *   a wrapper around the libfits module f_hlins.
 *
 * CALL:
 *   (RET_CODE) shHdrInsertLogical(HDR *a_hdr, const char *a_key,
 *                                    const int a_value, const char *a_comment);
 *          a_hdr     - pointer to the header structure
 *          a_key     - Keyword 
 *          a_value   - Value for the integer
 *          a_comment - Comments (can be blank or NULL)
 *
 * CALLS TO:
 *   p_shHdrMallocForVec()
 *   shHdrInit()
 *   f_hlins()
 *   f_mkikey()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   SH_SUCCESS                - on success
 *   SH_HEADER_INSERTION_ERROR - otherwise
 *============================================================================
 */
RET_CODE shHdrInsertInt(HDR *a_hdr, const char *a_key,
                           const int a_value, const char *a_comment)
{
   const char *comment;
   char  string[HDRLINESIZE];
   int   rc;

   /*
    * If there is no header vector, create one by reserving memory for it and
    * insert the given line into it. If there is already a header, simly insert
    * the line in.
    */
   if (a_hdr->hdrVec == NULL)  {
       p_shHdrMallocForVec(a_hdr);
       if ((shHdrInit(a_hdr)) == 0)
            return SH_HEADER_INSERTION_ERROR;
   }

   if (a_comment == NULL || *a_comment == 0)
       comment = "";
   else
       comment = a_comment;

   rc = f_hlins(a_hdr->hdrVec, MAXHDRLINE, MAXHDRLINE + 1,
                f_mkikey(string, (char *)a_key, (int)a_value,
                         (char *)comment));
   if (!rc)
       return SH_HEADER_INSERTION_ERROR;

   p_shHdrCntrIncrement(a_hdr);

   return SH_SUCCESS;
}

/*============================================================================
 * ROUTINE: shHdrInsertAscii
 *
 * DESCRIPTION:
 *   Insert a line in the FITS header given keyname, ASCII value and comment. 
 *   This routine is basically a wrapper around the libfits module f_hlins.
 *
 * CALL:
 *   (RET_CODE) shHdrInsertLogical(HDR *a_hdr, const char *a_key,
 *                                    const char *a_value, 
 *                                    const char *a_comment);
 *          a_hdr     - pointer to the header structure
 *          a_key     - Keyword 
 *          a_value   - Value for the ASCII line
 *          a_comment - Comments (can be blank or NULL)
 *
 * CALLS TO:
 *   p_shHdrMallocForVec()
 *   shHdrInit()
 *   f_hlins()
 *   f_mkakey()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   SH_SUCCESS                - on success
 *   SH_HEADER_INSERTION_ERROR - otherwise
 *============================================================================
 */
RET_CODE shHdrInsertAscii(HDR *a_hdr, const char *a_key,
                             const char *a_value, const char *a_comment)
{
   const char *comment;
   char  string[HDRLINESIZE];
   int   rc;

   /*
    * If there is no header vector, create one by reserving memory for it and
    * insert the given line into it. If there is already a header, simply insert
    * the line in.
    */
   if (a_hdr->hdrVec == NULL)  {
       p_shHdrMallocForVec(a_hdr);
       if ((shHdrInit(a_hdr)) == 0)
            return SH_HEADER_INSERTION_ERROR;
   }

   if (a_comment == NULL || *a_comment == 0)
       comment = "";
   else
       comment = a_comment;

   rc = f_hlins(a_hdr->hdrVec, MAXHDRLINE, MAXHDRLINE + 1,
                f_mkakey(string, (char *)a_key, (char *)a_value,
                         (char *)comment));
   if (!rc)
       return SH_HEADER_INSERTION_ERROR;

   p_shHdrCntrIncrement(a_hdr);

   return SH_SUCCESS;
}

/*============================================================================
 * ROUTINE: shHdrMakeLineWithDbl
 *
 * DESCRIPTION:
 *   Construct a line suitable for inclusion into a header, given double data.
 *   This routine is basically a wrapper around the libfits module f_mkdkey
 *
 * CALL:
 *   (void) shHdrMakeLineWithDbl(const char *a_keyword,
 *                                  const double a_value,
 *                                  const char *a_comment, char *a_dest);
 *          a_keyword - pointer to the keyword
 *          a_value   - Value for the double to be inserted in the line
 *          a_comment - Comments (can be blank or NULL)
 *          a_dest    - Pointer to the destination buffer to store the
 *                         constructed line. Note: it's the caller's respon-
 *                         sibility to assure that this buffer is large enough
 *                         to hold the constructed line.
 *
 * CALLS TO:
 *   f_mkdkey()
 *============================================================================
 */
void shHdrMakeLineWithDbl(const char *a_keyword, const double a_value,
                             const char *a_comment, char *a_dest)
{
   const char *comment;

   if (a_comment == NULL || *a_comment == 0)
       comment = "";
   else
       comment = a_comment;

   (void) f_mkdkey(a_dest, (char *) a_keyword, (double) a_value, 
                  (char *) comment);

   return;
}

/*============================================================================
 * ROUTINE: shHdrMakeLineWithAscii
 *
 * DESCRIPTION:
 *   Construct a line suitable for inclusion into a header, given ASCII data.
 *   This routine is basically a wrapper around the libfits module f_mkakey
 *
 * CALL:
 *   (void) shHdrMakeLineWithAscii(const char *a_keyword,
 *                                    const char *a_value,
 *                                    const char *a_comment, char *a_dest);
 *          a_keyword - pointer to the keyword
 *          a_value   - Value for the ASCII data to be inserted in the line
 *          a_comment - Comments (can be blank or NULL)
 *          a_dest    - Pointer to the destination buffer to store the
 *                         constructed line. Note: it's the caller's respon-
 *                         sibility to assure that this buffer is large enough
 *                         to hold the constructed line.
 *
 * CALLS TO:
 *   f_mkakey()
 *============================================================================
 */
void shHdrMakeLineWithAscii(const char *a_keyword, const char *a_value,
                               const char *a_comment, char *a_dest)
{
   const char *comment;

   if (a_comment == NULL || *a_comment == 0)
       comment = "";
   else
       comment = a_comment;

   (void) f_mkakey(a_dest, (char *) a_keyword, (char *) a_value,
                  (char *) comment);

   return;
}

/*============================================================================
 * ROUTINE: shHdrMakeLineWithInt
 *
 * DESCRIPTION:
 *   Construct a line suitable for inclusion into a header, given integer data.
 *   This routine is basically a wrapper around the libfits module f_mkikey
 *
 * CALL:
 *   (void) shHdrMakeLineWithInt(const char *a_keyword,
 *                                  const int a_value,
 *                                  const char *a_comment, char *a_dest);
 *          a_keyword - pointer to the keyword
 *          a_value   - Value for the integer to be inserted in the line
 *          a_comment - Comments (can be blank or NULL)
 *          a_dest    - Pointer to the destination buffer to store the
 *                         constructed line. Note: it's the caller's respon-
 *                         sibility to assure that this buffer is large enough
 *                         to hold the constructed line.
 *
 * CALLS TO:
 *   f_mkikey()
 *============================================================================
 */
void shHdrMakeLineWithInt(const char *a_keyword, const int a_value,
                             const char *a_comment, char *a_dest)
{
   const char *comment;

   if (a_comment == NULL || *a_comment == 0)
       comment = "";
   else
       comment = a_comment;

   (void) f_mkikey(a_dest, (char *) a_keyword, (int ) a_value,
                  (char *) comment);

   return;
}

/*============================================================================
 * ROUTINE: shHdrMakeLineWithLogical
 *
 * DESCRIPTION:
 *   Construct a line suitable for inclusion into a header, given logical data.
 *   This routine is basically a wrapper around the libfits module f_mklkey
 *
 * CALL:
 *   (void) shHdrMakeLineWithLogical(const char *a_keyword,
 *                                      const int a_value,
 *                                      const char *a_comment, 
 *                                      char *a_dest);
 *          a_keyword - pointer to the keyword
 *          a_value   - Value for the logical to be inserted in the line
 *          a_comment - Comments (can be blank or NULL)
 *          a_dest    - Pointer to the destination buffer to store the
 *                         constructed line. Note: it's the caller's respon-
 *                         sibility to assure that this buffer is large enough
 *                         to hold the constructed line.
 *
 * CALLS TO:
 *   f_mklkey()
 *============================================================================
 */
void shHdrMakeLineWithLogical(const char *a_keyword, const int a_value,
                                 const char *a_comment, char *a_dest)
{
   const char *comment;

   if (a_comment == NULL || *a_comment == 0)
       comment = "";
   else
       comment = a_comment;

   (void) f_mklkey(a_dest, (char *) a_keyword, (int) a_value,
                  (char *) comment);

   return;
}

/*============================================================================
 * ROUTINE: shHdrReplaceLine
 *
 * DESCRIPTION:
 *   Replace, in the FITS header, line identified by a_line on the line number
 *   identified by a_lineNo. This function is basically a wrapper around the 
 *   libfits module f_hlrep.
 *
 * CALL:
 *   (RET_CODE) shHdrInsertLine(HDR *a_hdr, const int a_lineNo,
 *                                    const char *a_line);
 *          a_hdr     - pointer to the header structure
 *          a_lineNo  - line number after which to insert the new line
 *          a_line    - pointer to the new line to be inserted
 *
 * CALLS TO:
 *   f_hlrep()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_HEADER_IS_NULL - if header has not been allocated
 *   SH_GENERIC_ERROR  - on all other occasions
 *============================================================================
 */
RET_CODE shHdrReplaceLine(HDR *a_hdr, const int a_lineNo,
                             const char *a_line)
{
   if (a_hdr->hdrVec == NULL)   /* The header must exist in order to */
       return SH_HEADER_IS_NULL;     /* be replaced */

   if ((f_hlrep(a_hdr->hdrVec, a_lineNo, (char *)a_line)) == 0)
       return SH_GENERIC_ERROR;

   p_shHdrCntrIncrement(a_hdr);

   return SH_SUCCESS;
}

/*============================================================================
 * ROUTINE: shHdrCopy
 *
 * DESCRIPTION:
 *   This function is responsible for copying one header to another.  If a_inHdr
 *   is NULL, there's nothing much it can do except return an error code.
 *   If a_outHdr's header vector is NULL, memory is allocated as needed.
 *   If a_outHdr's header vector is NOT null, it is wiped out.  Thus it is
 *   the caller's responsibility to make sure that they have saved the contents
 *   of a_outHdr's header if needed.
 *
 * CALL:
 *   (RET_CODE) shHdrInsertLine(const HDR *a_inHdr, HDR *a_outHdr);
 *              a_inHdr  - header to copy from
 *              a_outHdr - header to copy to
 *   
 *
 * CALLS TO:
 *   f_hlrep()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_HEADER_IS_NULL - if header has not been allocated
 *============================================================================
 */
RET_CODE shHdrCopy(const HDR *a_inHdr, HDR *a_outHdr)
{
   int i;

   if (a_inHdr->hdrVec == NULL)          /* Source header is null, nothing */
       return SH_HEADER_IS_NULL;         /* to copy from */

   if (a_outHdr->hdrVec == NULL)   /* Destination header is null, */
       p_shHdrMallocForVec(a_outHdr); /* allocate memory for it */
   else                                  /* If it's not null, free what it's */
       p_shHdrFree(a_outHdr);         /* currently pointing to */

   for (i = 0; i < MAXHDRLINE; i++)
        if (a_inHdr->hdrVec[i] != NULL)  {
            a_outHdr->hdrVec[i] = 
                 (char *)shMalloc(strlen(a_inHdr->hdrVec[i]) + 1);
            strcpy(a_outHdr->hdrVec[i], a_inHdr->hdrVec[i]);
        }

   p_shHdrCntrIncrement(a_outHdr);

   return SH_SUCCESS;
}

/*============================================================================
 * ROUTINE: shHdrDelByKeyword
 *
 * DESCRIPTION:
 *   This function is responsible for deleting a header vector identified
 *   by a keyword passed in as a parameter. This function is basically a
 *   wrapper around the libfits module f_kdel
 *
 * CALL:
 *   (RET_CODE) shHdrDelByKeyword(HDR *a_hdr, const char *a_keyword
 *              a_hdr     - pointer to the header
 *              a_keyword - line to be deleted is identified by this keyword
 *
 *
 * CALLS TO:
 *   f_kdel()
 *   p_shHdrCntrIncrement()
 *
 * RETURN VALUES:
 *   SH_SUCCESS        - on success
 *   SH_HEADER_IS_NULL - if header has not been allocated
 *============================================================================
 */
RET_CODE shHdrDelByKeyword(HDR *a_hdr, const char *a_keyword)
{
   if (a_hdr->hdrVec == NULL)
       return SH_HEADER_IS_NULL;

   if (f_kdel(a_hdr->hdrVec, (char *)a_keyword))  {
       p_shHdrCntrIncrement(a_hdr);
       return SH_SUCCESS;
   }

   return SH_GENERIC_ERROR;
}
