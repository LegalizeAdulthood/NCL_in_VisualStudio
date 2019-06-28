/*********************************************************************
**    NAME         :  reenvout.c
**       CONTAINS:
**       ur_environ_out()
**    COPYRIGHT 1985 (c) UNICAD Inc.  All Rights Reserved.
**    MODULE NAME AND RELEASE LEVEL 
**       reenvout.c , 25.1
**    DATE AND TIME OF LAST MODIFICATION
**       04/29/15 , 15:11:29
*********************************************************************/

#include "udebug.h"
#include "usysdef.h"
#include "umoveb.h"
#include "rienvtab.h"
#include "ribase.h"
#include "rerrdef.h"
#include "riddldef.h"
#include "xenv1.h"
#include "mattrddl.h"

/*********************************************************************
**    E_FUNCTION     :  ur_environ_out(envfd)
**       save the environment to file envfd
**    PARAMETERS   
**       INPUT  : 
**				envfd	int			file descriptor for the file to save to
**       OUTPUT :  
**          none
**    RETURNS      : 0 if successful else error code
**    SIDE EFFECTS : none
**    WARNINGS     : none
*********************************************************************/
ur_environ_out(envfd)
int			envfd;				/* file descriptor to save to */
{
	extern struct UR_env_table_rec UR_environ_table[];
	struct UR_data	*ent_ptr;	/* a pointer to an entry 				*/
	UU_KEY_ID	rel_key_id;		/* a relative key_id tuple				*/
#define	atmax	64
	struct attr_def	atdefs[atmax];	/* attribute definitions */
	int		num_attr;			/* number of attributes parsed */
	int		rel_typ;
	UU_LOGICAL relocate_mtid;	/* TRUE if we need to relocate mtid's	*/
	char		*attr_ptr;			/* ptr to attributes within tuple		*/
	int		atndx;				/* index into atdefs */
	int		atype;				/* the data type of the attribute */
	char		*data_ptr;			/* ptr to data within attribute			*/
	int		data_offset;		/* offset to next data item				*/
	int		rndx;					/* row index of attribute array	*/
	int		cndx;					/* col index of attribute array	*/
	int	status;
	int	i,j,k;
	int	l;
	int	lrecl;
	char	*name;	/* pointer to fix name */
   int   nb;
#ifdef UU_DEBUGON
	int	m;
	char	prfx[4];
#endif

	uu_denter(UU_RTRC,(us,"ur_environ_out(%d)", envfd));
	relocate_mtid = UU_TRUE;
	status = ur_chk_data_dict() ;
	if (status != 0)
	{
		uu_dprint(-1,(us,
				"ERROR: unable to read data dictionary, ur_environ_out"));
		uu_dexit;
		return(-1);				/* return error code */
	}
	for(i = 0; UR_environ_table[i].name[0] != '\0'; i++)
	{
		/* copy from regular to new so we can massage the keys and not destroy */
		uu_dprint(UU_RITRC,(us,"move %d bytes from 0x%x to 0x%x for %s",
					UR_environ_table[i].length, UR_environ_table[i].adrs,
					UR_environ_table[i].new_adrs, UR_environ_table[i].name));
		uu_dprint(UU_RITRC,(us,"cpln key = 0x%x",ur_get_dispattr_cpln_key()));
		uu_dprint(UU_RITRC,(us,"new cpln key = 0x%x",ur_get_new_dispattr_cpln_key()));
		uu_move_byte(UR_environ_table[i].adrs, UR_environ_table[i].new_adrs,
							UR_environ_table[i].length);
	}
	uu_dprint(UU_RITRC,(us,"environment moved to new"));

	/* write env file from environment */
	for(i = 0; UR_environ_table[i].name[0] != '\0'; i++)
	{
		/* look up in data dictionary and fix keys */
		name = UR_environ_table[i].name + 3;	/* skip prefix---kludge--- */
		num_attr = ur_data_dict(atdefs, atmax,name,&rel_typ);
		if(num_attr <= 0)
		{
			uu_dprint(-1,(us,"ERROR:ur_environ_out can't find %s in data dict.",
							name));
			status = URM_RELNTFND; /* rel not in data dictionary error */
			goto s_p90;
		}
		ent_ptr = (struct UR_data *) UR_environ_table[i].new_adrs;
		if (atdefs[0].attr_type == KEY_ID)
		{
			rel_key_id = ent_ptr->key_id;
			if(relocate_mtid)
			{
				uri_sp02a(&rel_key_id);
			}
			/* put it back into the modal before we write */
			ent_ptr->key_id = rel_key_id;
			/* adjust geometry with key_id's imbedded in the data */
			attr_ptr = (char *)ent_ptr+sizeof(UU_KEY_ID)+sizeof(long);
			atndx = 1;
		}
		else
		{
			attr_ptr = (char *)ent_ptr;
			atndx = 0;
		}
		for (; atndx<num_attr; atndx++)
		{
			atype = atdefs[atndx].attr_type;
			/* set array indexs, non array has 1 row, 1 col */
			rndx = atdefs[atndx].num_rows;
			cndx = atdefs[atndx].num_cols;
			data_ptr = attr_ptr;
			uu_dprint(UU_RITRC,(us,"field %d type %d[%drows,%dcols] at adrs 0x%x",atndx,atype,rndx,cndx,data_ptr));
			if((atype == KEY_ID) || (atype == REL_ID))
			{
				data_offset = atdefs[atndx].attr_off / (rndx * cndx);
				for(k = 1; k <= cndx; k++)
				{
					for(j = 1; j <= rndx; j++)
					{
						uu_dprint(UU_RITRC,(us,"nxt adrs 0x%x", data_ptr));
						/* perform key jiggery-pokery */
						uu_dprint(UU_RTRC,(us,"key to relocate: 0x%x",
							*((long *)data_ptr)));
						uri_sp02a(data_ptr);
						data_ptr = data_ptr + data_offset;
					}	/* for each row */
				}	/* for each column */
			}	/* if type is key */
			attr_ptr += atdefs[atndx].attr_off;
		} /* for each definition field */

		/* do blocked write directly from environment */
		lrecl = UR_environ_table[i].length;
      status =ux_write_block(envfd,UR_environ_table[i].new_adrs,lrecl,&nb,
						UX_PRTERRS);
#ifdef UU_DEBUGON
		for (m = 0; m < 3; m++) prfx[m] = UR_environ_table[i].name[m];
		prfx[3] = '\0';
		ur_dump_modal(name, prfx, UU_TRUE);
#endif
		uu_dprint(UU_RITRC,(us,"cpln key = 0x%x",ur_get_dispattr_cpln_key()));
		uu_dprint(UU_RITRC,(us,"new cpln key = 0x%x",ur_get_new_dispattr_cpln_key()));
		uu_dprint(UU_RITRC,(us,"&cpln key = 0x%x",&UM_new_dispattr.cpln_key));

		/* check the returned status for the write */
		if(status != 0)
		{
			uu_dprint(-1,(us,"ERROR:writing env to file %d from ux_write_block",
							status));
			/* return error message - error writing the file */
			uu_dexit;
			return(status);
		}
	}
s_p90:	/* error bailout */
	uu_dexit;
	return(0);
}
