/*********************************************************************
**    NAME         :  redat
**       CONTAINS:
**       ur_delete_attr()
**       ur_delete_attr1()
**    COPYRIGHT 1985 (c) UNICAD Inc.  All Rights Reserved.
**    MODULE NAME AND RELEASE LEVEL 
**       redat.c , 25.1
**    DATE AND TIME OF LAST MODIFICATION
**       04/29/15 , 15:11:28
*********************************************************************/

#include  "usysdef.h"
#include "umessages.h"
#include	"udebug.h"
#include	"ribase.h"
#include	"rmtuple.h"
#include "riddle.h"
#include "r1emsgpk.h"

/*********************************************************************
** E_FUNCTION : status = ur_delete_attr(key)
**      Delete attribute tuple for key master (attr tuple may just
**      have its use count decreased if > 1).
**    PARAMETERS   
**       INPUT  : 
**           key        master tuple key to delete attribute.
**       OUTPUT :  
**				none
**    RETURNS      :  0 if function successful, -1 otherwise
**    SIDE EFFECTS : none
**    WARNINGS     : none
*********************************************************************/

ur_delete_attr(key)
UU_KEY_ID         key;          /* master tuple key */
{
	UR_messagePacket	theMessage;

	uu_denter(UU_RTRC, (us, "ur_delete_attr(key=0x%x)", key));
	ur1_initMsgPacket(&theMessage, UU_DEFAULT_DEL_ATTR, 0);
	uu_dprint(UU_RTRC, (us, "warning I exit here without really exiting"));
	uu_dexit;
	return(ur_delete_attr1(key, &theMessage, 1));
}


/*********************************************************************
** E_FUNCTION : status = ur_delete_attr1(key, theMessage, messageCnt)
**      Delete attribute tuple for key master (attr tuple may just
**      have its use count decreased if > 1).
**    PARAMETERS   
**       INPUT  : 
**           key        master tuple key to delete attribute.
**				theMessage,	for scheduler
**       OUTPUT :  
**				none
**    RETURNS      :  0 if function successful, -1 otherwise
**    SIDE EFFECTS : none
**    WARNINGS     : none
*********************************************************************/

ur_delete_attr1(key, theMessage, messageCnt)
UU_KEY_ID	key;          /* master tuple key */
UR_messagePacket	theMessage[];
int					messageCnt;
{
	struct UR_attr attr;				/* temp generic attr tuple */
	int				status;
	UU_KEY_ID      rel_key;
	UR_REL_NUM     rel;
	UR_TUPLE_INDX  tuple;
	int            i;
	struct UR_MTID_rec	*m_ptr;	/* pointer to a master tuple */

	/*------------ begin function code ----------------------------------*/
	uu_denter(UU_RTRC, (us, "ur_delete_attr1(key=0x%x messageCnt=%d)", key, messageCnt));
	ur_k2rt(key, &rel, &tuple);
	status = ur_get_tuple_ptr(rel, tuple, &m_ptr);
	if (ur_editablep(m_ptr))
	{
		rel_key = m_ptr->assocs[UR_ATTR_INDX];
	}
	else
	{
		status = -1;	/* entity is not editable */
	}
	/* get attr tuple key */
	if (status==0)
	{
		if (rel_key==0) {
			status = -1;		/* no attribute data */
			goto theExit;			/* cannot continue */
		}
		ur_k2rt(rel_key, &rel, &tuple); /* get relation and index*/
		status = ur_retrieve_tuple(rel, tuple, &attr); /* get data */
		if (status==0)
		{
			if (attr.use_count==1)
			{
				/* this mtuple is only user - delete the attr tuple */
				status = ur_delete_tuple(rel, tuple);
			}
			else
			{
				/* decrease the count and update the attr tuple */
				attr.use_count--;
				ur_update_tuple(rel, tuple, &attr);
			}
			/* set mtuple attr key to zero */
			rel_key = 0;
			ur_update_mtuple_attr(key, UR_ATTR_INDX, rel_key);
		}

		/* get master rel & tuple, and a pointer to the master fixed data */
		ur_k2rt(key, &rel, &tuple);
		status = ur_get_tuple_ptr(rel, tuple, &m_ptr);	

		/* now notify associates */
		if ((*m_ptr).no_assocs > UR_MAX_INDX + 1)
		{
			uri_notify(m_ptr, key, theMessage, messageCnt);
		}
	}
	/*------------- function exit ---------------------------------------*/
theExit:                              /****** EXIT LABEL ******/
	uu_dprint(UU_RITRC, (us, "ur_delete_attr exit status=%d", status));
	uu_dexit;
	return(status);
}
