/******************************************************************************
 *    (c)2009 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 * USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: dirdbase.c $
 * $brcm_Revision: 11 $
 * $brcm_Date: 9/28/09 8:56p $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: /AppLibs/dlna/dms/dirdbase.c $
 * 
 * 11   9/28/09 8:56p ismailk
 * SW7405-3080: Set the QAM mode from the channel_info.txt file.
 * 
 * 10   9/25/09 3:37p ismailk
 * SW7405-3080: DMS IP Streaming (Live channel) Feature.
 * 
 * 9   7/30/09 6:49p ajitabhp
 * PR55165: Removing Connection Stalling Support.
 * 
 * 8   7/30/09 1:22p ajitabhp
 * PR55165: Handling Non DLNA compliant data.
 * 
 * 7   7/24/09 10:44a ajitabhp
 * PR55165: Removed UnUsed variables.
 * 
 * 6   7/10/09 4:15p ajitabhp
 * PR55165: Tracking DLNA Profileing Error
 * 
 * 5   7/10/09 1:16p ajitabhp
 * PR55165: DLNA Flags for Non-DLNA Content.
 * 
 * 4   7/9/09 5:31p ajitabhp
 * PR55165: Bug Fixes for The following:
 * 1. Some files were not detected as DLNA profile files.
 * 2. There were invalid characters passed to the client when a NON dlna
 *  file exists with DLNA files.
 * 3. The server crashed when there was a DLNA file without the DLNA
 *  profile.
 * 
 * 3   6/30/09 7:45p ajitabhp
 * PR55165: MP4 was not parsed correctly.
 * 
 * 2   6/29/09 12:13p ajitabhp
 * PR55165: Added Path Input From User.
 * 
 *****************************************************************************/
/*
* Directory data base implementation header file.
* This is just a preliminary implementation of the   
* directory structure of content directory. IT WILL'
* BE MODIFIED LATER as the Media Server code gets matured.
* We are just using the mocked up version of the database
* to make sure that we advertise the services properly.
*/

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <avformat.h>
#include "trace.h"
#include "dirdbase.h"
#include "profiles.h"
#include "AudioGetTagInfo.h"

extern struct mime_type_t MIME_Type_List[];
/*Functions for creating the database*/

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


void
add_to_free_list(pdirdbase_context db_ctx,
                 void *add_to_free)
{
    pcleanup_item new_item,temp;
    new_item = malloc(sizeof(cleanup_item));
    if(!new_item)
    {
        BcmLogMsg("%s:%s:%d","Failed To Allocate Memory",__FILE__,__LINE__);
        return;
    }

    new_item->Address = add_to_free;
    new_item->next_item = NULL;

    if(!db_ctx->freelist){
        db_ctx->freelist = new_item;
        db_ctx->freecnt++;
        return;
    }    

    temp = db_ctx->freelist;
    while(temp)
    {
        if(temp->next_item)
        {    
            temp = temp->next_item;
        }else{
            temp->next_item = new_item;
            db_ctx->freecnt++;
            break;
        }
    }
}

/* Add by tuhanyu, for create/destory object and import/delete resource action, 2011/05/03,  start */
pbcm_upnp_object
delete_from_free_list(pdirdbase_context db_ctx, unsigned int id)
{
    pbcm_upnp_object item = NULL;
    pcleanup_item       temp = NULL, backup = NULL; 

    temp = db_ctx->freelist;
    item = temp->Address;
    if(item->Id == id)
    {
        db_ctx->freelist = temp->next_item;
        free(temp);
        return item;
    }
    else
    {
        backup = temp;
        temp = temp->next_item;
    }
    
    while(temp)
    {
        item = temp->Address;
        if(item->Id == id)
        {
            backup->next_item = temp->next_item;
            free(temp);
            break;
        }
        else
        {
            backup = temp;
            temp = temp->next_item;
            item = NULL;
        }
    }

    return item;
}
/* Add by tuhanyu, end */

static 
pbcm_upnp_object 
create_empty_cont(pdirdbase_context db_ctx,
                  char *title, 
                  unsigned int ContID)
{
    pbcm_upnp_object NewContainer=NULL;

    NewContainer = 
            (pbcm_upnp_object) 
                        malloc(sizeof(bcm_upnp_object));

    if(!NewContainer)
    {
        BcmLogMsg("%s:%s:%d","Failed To Allocate Memory",__FILE__,__LINE__);
        return NULL;
    }

    add_to_free_list(db_ctx,NewContainer);
    memset(NewContainer,0,sizeof(bcm_upnp_object));
    NewContainer->cont_flags |= OBJ_TYPE_CONT;
    NewContainer->Id = ContID;

    /*Copy the Title*/
    memcpy(NewContainer->title,
            title,
            min(sizeof(NewContainer->title), strlen(title)));


    strcpy(NewContainer->mime_class, CONT_CLS_UPNP_FOLDER);
    NewContainer->u.container_info.Container_cnt=0;
    return NewContainer;
}

static
BOOL
insert_container(pdirdbase_context    pdb_context,
                 pbcm_upnp_object    parent,
                 unsigned int        ContID,
                 char    *            title)
{
    pbcm_upnp_object pNewCont=NULL,temp=NULL;

    if(!pdb_context)
        return FALSE;

    if(!parent && pdb_context->DirRoot)
        return FALSE;

    pNewCont = create_empty_cont(pdb_context,title,ContID);
    if(!pNewCont)
    {
        BcmLogMsg("%s:%s:%d","Failed To Create Container",__FILE__,__LINE__);
        return FALSE;
    }
    /*Null pointer will be passed to this function when Root is to be created*/
    if(!parent){
        pdb_context->DirRoot = pNewCont;
        return TRUE;
    }
    
    temp = parent;
    while(temp)
    {
        if(temp->next_container)
            temp=temp->next_container;
        else {
            temp->next_container = pNewCont;
            pNewCont->parent = parent;
            parent->u.container_info.Container_cnt++;
            break;
        }
    }

    return TRUE;
}

#ifdef HAVE_DLNA

static
char *
GetDlnaInfoFromProtocol(char *Protocol)
{
    char *RetSubStr=NULL,*x=NULL;
    if(!Protocol) return NULL;
    x = strdup(Protocol);
    RetSubStr = strtok(x,":DLNA");
    while(RetSubStr)
    {
        if(!strncmp(RetSubStr,"DLNA",4))
            return RetSubStr;

        RetSubStr = strtok(NULL,":");
    }

    free(x);
    return NULL;
}

/* This function assumes that the buffer in which the 
 * information is to be appended is already passed in.
 */
static 
void
AppendDlnaInfoToProtocol(dlna_org_play_speed_t speed,
                         dlna_org_conversion_t ci,
                         dlna_org_operation_t op,
                         dlna_org_flags_t flags,
                         char ** protocol)
{
    char dlna_info[128];
    memset(dlna_info,0,sizeof(dlna_info));
    if(speed == DLNA_ORG_PLAY_SPEED_NORMAL)
    {
            sprintf (dlna_info, "%s=%.2x;%s=%d;%s=%.8x%.24x",
                        "DLNA.ORG_OP", op,
                        "DLNA.ORG_CI", ci,
                        "DLNA.ORG_FLAGS", flags, 0);

    }else{
            sprintf (dlna_info, "%s=%.2x;%s=%d;%s=%d;%s=%.8x%.24x",
                        "DLNA.ORG_OP", op,
                        "DLNA.ORG_PS", speed,/*Add only if it is other than(greater than) 1*/
                        "DLNA.ORG_CI", ci,
                        "DLNA.ORG_FLAGS", flags, 0);
    }

    strcat (*protocol, dlna_info);
    return;
}

#endif

static
CONTENT_TYPE
GetMediaType(char *FullFiePath)
{
    unsigned int Index=0;
    AVFormatContext *ctx;
    CONTENT_TYPE MType= MTYPE_UNKNOWN;

    av_register_all();

    if (av_open_input_file (&ctx, FullFiePath, NULL, 0, NULL) != 0)
    {
        BcmLogMsg("GetMediaType: Failed to Open File %s\n",FullFiePath);
        return MType;
    }

    if(av_find_stream_info(ctx)<0)
    {
        BcmLogMsg("Error : Couldn't find stream information !\n");
        return MType; 
    }
    
    for(Index=0;Index < ctx->nb_streams;Index++)
    {
        if(ctx->streams[Index]->codec->codec_type == CODEC_TYPE_VIDEO)
        {
            if(1 == ctx->nb_streams) 
                MType |= MTYPE_IMAGE;
            else 
                MType |= MTYPE_VIDEO;

        }else if(ctx->streams[Index]->codec->codec_type == CODEC_TYPE_AUDIO){
            MType |= MTYPE_AUDIO;
        }
    }
    
    if((MType & MTYPE_VIDEO) && 
        (MType & MTYPE_AUDIO))
        MType = MTYPE_AV;

    av_close_input_file (ctx);
    return MType;
}

static
dlna_org_flags_t
GetDLNAFlags(CONTENT_TYPE        MType)
{
    dlna_org_flags_t    dlna_flags=0;
    dlna_flags =    DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |    /* If Stall is set , This should be set*/
                    //DLNA_ORG_FLAG_CONNECTION_STALL |            /* We support Pause and Pause-Release Media operations*/
                    DLNA_ORG_FLAG_DLNA_V15;    
    
    switch (MType)
    {
        case MTYPE_IMAGE:
        {
            dlna_flags |= DLNA_ORG_FLAG_INTERACTIVE_TRANSFERT_MODE;
            break;
        }
        case MTYPE_AUDIO:
        {
            dlna_flags |= DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE;
            break;
        }
        case MTYPE_AV:
        {
            dlna_flags |= DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE;
            break;
        }
        default:
        {
            BcmLogMsg("Unknown Media Type\n");
            break;
        }
    }

    return dlna_flags;
}

static
char *
get_file_date(const char *filename)
{
	 /* Get file create time*/
	 struct stat buf;
    char *RetVal = (char*)malloc(20);	 
    struct tm *T;
    
	 if(stat(filename, &buf) == -1)
	 {
	     printf("Get the file stat of %s failed.\n", filename);
		  return NULL;
	 }

	 T = localtime(&buf.st_atime);

	 sprintf(RetVal,"%04d-%02d-%02d",
	 		T->tm_year + 1900,
	 		T->tm_mon+1,
			T->tm_mday);  

	 return RetVal;
}

static
pbcm_upnp_object
create_new_item( pdirdbase_context    db_ctx,
                 unsigned int        Id,
                 char                *fullPath, 
                 unsigned int        itemSize,
                 char                *mime_class,            
                 char                *title,
                 char                *protocol,
                 CONTENT_TYPE        MType)    
{
    BOOL    DLNACompliant=FALSE;
    struct buffer_t *SrcProtoBuff=NULL;
    pbcm_upnp_object  NewItem=NULL;
    char *dlna_protocol=NULL;

    NewItem = malloc(sizeof(bcm_upnp_object));
    if(!NewItem)
    {
        BcmLogMsg("%s:Failed to Allocate Item\n",__FUNCTION__);
        return FALSE;
    }

    add_to_free_list(db_ctx,NewItem);
    memset(NewItem,0,sizeof(bcm_upnp_object));
    NewItem->cont_flags=0; 
    NewItem->Id = Id;

    /*Copy the Title*/
    memcpy(NewItem->title,
            title,
            min(sizeof(NewItem->title), strlen(title)));

    sprintf(NewItem->u.item_info.u.file.url,"%s/?id=%d", db_ctx->base_file_url,Id);
    strcpy(NewItem->u.item_info.u.file.fullpath,fullPath);
    NewItem->u.item_info.u.file.itemSize = itemSize;

    if(MType == MTYPE_AUDIO)
    {
        /* use libid3tag library to get Tag ID information */
        if(!InitAudioTagInfo(fullPath))
        {
            char *temp_ptr = NULL;

            temp_ptr = GetAudioTagInfo("TPE1");
            if(temp_ptr)
            {
                NewItem->u.item_info.u.file.artist = strdup(temp_ptr);
            }
            else
            {
                NewItem->u.item_info.u.file.artist = strdup("Unkown");
            }

            temp_ptr = GetAudioTagInfo("TRCK");
            if(temp_ptr)
            {
                NewItem->u.item_info.u.file.track = strdup(temp_ptr);
            }
            else
            {
                NewItem->u.item_info.u.file.track = strdup("0");
            }

            temp_ptr = GetAudioTagInfo("TALB");
            if(temp_ptr)
            {
                NewItem->u.item_info.u.file.album = strdup(temp_ptr);
            }
            else
            {
                NewItem->u.item_info.u.file.album = strdup("Unkown");
            }

            temp_ptr = GetAudioTagInfo("TCON");
            if(temp_ptr)
            {
                NewItem->u.item_info.u.file.genre = strdup(temp_ptr);
            }
            else
            {
                NewItem->u.item_info.u.file.genre = strdup("Unkown");
            }

            temp_ptr = GetAudioTagInfo("TYER");
            if(temp_ptr)
            {
                NewItem->u.item_info.u.file.date = strdup(temp_ptr);
            }
            else
            {
                temp_ptr = get_file_date(fullPath);
                if(temp_ptr)
                    NewItem->u.item_info.u.file.date = temp_ptr;
                else
	                 NewItem->u.item_info.u.file.date = strdup("2010-01-01");
            }

            FinishAudioTagInfo();
        }
    }

    strcpy(NewItem->mime_class, mime_class);
    strcpy(NewItem->u.item_info.u.file.protocol, protocol);/*Set Some Default Protocols*/
    NewItem->u.item_info.u.file.MediaType = MType;
    
#ifdef HAVE_DLNA

    NewItem->u.item_info.dlna_profile = dlna_guess_media_profile(db_ctx->dlna_context,fullPath);

    if(NewItem->u.item_info.dlna_profile)
    {
        DLNACompliant = TRUE;
        if ( (NewItem->u.item_info.dlna_profile->class == DLNA_CLASS_IMAGE) &&
             (NewItem->u.item_info.u.file.MediaType != MTYPE_IMAGE))
        {
//            printf("Did not detect Correct Media Type For Image\n");
            NewItem->u.item_info.u.file.MediaType = MTYPE_IMAGE;
        }
        
        if( (NewItem->u.item_info.dlna_profile->class == DLNA_CLASS_AUDIO) &&
            (NewItem->u.item_info.u.file.MediaType != MTYPE_AUDIO))
        {
//            printf("Did not detect Correct Media Type For Audio\n");
            NewItem->u.item_info.u.file.MediaType = MTYPE_AUDIO;
        }
        
        if( (NewItem->u.item_info.dlna_profile->class == DLNA_CLASS_AV) && 
            (NewItem->u.item_info.u.file.MediaType != MTYPE_AV))
        {
//            printf("Did not detect Correct Media Type\n");
            NewItem->u.item_info.u.file.MediaType = MTYPE_AV;
        }
    }else{
            BcmLogMsg("No DLNA profile for %s\n",fullPath);
    }

    NewItem->u.item_info.ProtocolID = DLNA_PROTOCOL_INFO_TYPE_HTTP;
    NewItem->u.item_info.PlaySpeedID = DLNA_ORG_PLAY_SPEED_NORMAL;
    NewItem->u.item_info.ConvIndicator = DLNA_ORG_CONVERSION_NONE;
    NewItem->u.item_info.Operations = DLNA_ORG_OPERATION_RANGE;
    NewItem->u.item_info.DlnaFlags = GetDLNAFlags(NewItem->u.item_info.u.file.MediaType);

    if(DLNACompliant)
    {    
        dlna_protocol = dlna_write_protocol_info(    DLNA_PROTOCOL_INFO_TYPE_HTTP,
                                                    DLNA_ORG_PLAY_SPEED_NORMAL,
                                                    DLNA_ORG_CONVERSION_NONE,
                                                    DLNA_ORG_OPERATION_RANGE,
                                                    NewItem->u.item_info.DlnaFlags,
                                                    NewItem->u.item_info.dlna_profile);
        if(dlna_protocol)
        {
            memset(NewItem->u.item_info.u.file.protocol,0,sizeof(NewItem->u.item_info.u.file.protocol));
            strcpy(NewItem->u.item_info.u.file.protocol, dlna_protocol);
            NewItem->u.item_info.DlnaFeatures = GetDlnaInfoFromProtocol(NewItem->u.item_info.u.file.protocol);
            free(dlna_protocol);
        }else{
            BcmLogMsg("!! ERROR ERROR: Failed To Write Protocol !!\n");
        }

    }else{
        char * protocol = &NewItem->u.item_info.u.file.protocol[0];


        AppendDlnaInfoToProtocol(    NewItem->u.item_info.PlaySpeedID,
                                    NewItem->u.item_info.ConvIndicator,
                                    NewItem->u.item_info.Operations,
                                    NewItem->u.item_info.DlnaFlags,
                                    &protocol);        
    }


#endif

    SrcProtoBuff = (DLNACompliant) ? db_ctx->SrcProtocol:db_ctx->NonDlnaProtocols;

    if(!SrcProtoBuff->buf)
        buffer_appendf(SrcProtoBuff,"%s",NewItem->u.item_info.u.file.protocol);    
    else if(!strstr(SrcProtoBuff->buf, 
                    NewItem->u.item_info.u.file.protocol))
    {
        buffer_appendf(SrcProtoBuff,",%s",NewItem->u.item_info.u.file.protocol);    
    }

    return NewItem;
}


void
insert_item(pbcm_upnp_object pContainer,
            pbcm_upnp_object pInsertItem)
{
    pbcm_upnp_object temp=NULL;

    if(!pContainer || !pInsertItem)
        return;

    temp = pContainer;
    while(temp)
    {
        if(temp->next_item)
            temp=temp->next_item;
        else {
            temp->next_item = pInsertItem;
            pInsertItem->parent = pContainer;
            pContainer->u.container_info.item_cnt++;
            break;
        }
    }

    return;
}

/* Add by tuhanyu, for create/destory object and import/delete resource action, 2011/05/03,  start */
void
delete_item(pbcm_upnp_object pobject)
{
    pbcm_upnp_object parent = NULL, temp = NULL, backup = NULL;

    parent = pobject->parent;
    temp = parent->next_item;
    if(temp == pobject)
    {
        parent->next_item = temp->next_item;
        parent->u.container_info.item_cnt--;
        return;
    }
    else
    {
        backup = temp;
        temp = temp->next_item;
    }
    while(temp)
    {
        if(temp == pobject)
        {
            backup->next_item = temp->next_item;
            parent->u.container_info.item_cnt--;
            break;
        }
        else
        {
            backup = temp;
            temp = temp->next_item;
        }
    }

    return;
}
/* Add by tuhanyu, end */

static
pbcm_upnp_object
get_container(pdirdbase_context db_ctx, unsigned int ContID)
{
    pbcm_upnp_object temp=NULL;

    if(!db_ctx || !db_ctx->DirRoot)    /*Data base is not setup yet*/
        return NULL;

    temp =  db_ctx->DirRoot;

    while(temp) /*Go Through the linked list and look for the container ID*/
    {
        if(temp->Id == ContID)
            break;

        temp = temp->next_container;
    }

    return temp;
}

static
pbcm_upnp_object
get_item(pdirdbase_context db_ctx, unsigned int ItemID)
{
    pbcm_upnp_object Container=NULL,Item=NULL;

/* Add by tuhanyu, for proceed exit singularly, 2011/04/02, start */
    if(!db_ctx || !db_ctx->DirRoot)    /*Data base is not setup yet*/
        return NULL;
/* Add by tuhanyu, end */
    
    Container =  db_ctx->DirRoot;

    while(Container) /*Go Through the linked list and look for the container ID*/
    {
        Item = Container->next_item;
        while(Item)
        {
            if(Item->Id == ItemID)
                return Item;
            else
                Item = Item->next_item;
        }
        Container = Container->next_container;
    }

    return NULL;
}

BOOL
GetFileExtension(char* FileName, char **ReturnExt)
{
    char* peek =NULL, *temp;
    unsigned int len;
    
    
    if(!FileName || !ReturnExt)
        return FALSE;

    peek = &FileName [strlen(FileName) - 1];
    temp = peek;
    len = strlen(FileName);
    while (peek >= FileName)
    {
        if(temp - peek >= MAX_EXT_LEN - 1)
        {
            break;
        }
        
        if (*peek == '.')
        {
            strcpy (*ReturnExt, (peek+1));    /*Skip the Dot*/
            break;
        }
        peek--;
    }
    return TRUE;
}

struct mime_type_t *
GetMimeFromTable(char *Extension,
                 CONTENT_TYPE *MType)
{
    struct mime_type_t *pmime=NULL;
    char * ContentClass=NULL;
    pmime = &MIME_Type_List[0];

    if(*MType == MTYPE_AUDIO) ContentClass = ITEM_CLS_UPNP_AUDIO;
    else if(*MType == MTYPE_AV) ContentClass = ITEM_CLS_UPNP_VIDEO;
    else ContentClass = ITEM_CLS_UPNP_PHOTO;

    while(pmime->extension)
    {
        if( (!strcasecmp(pmime->extension,Extension))
#if 0 /* don't use ffmpeg to save time */
        		&&
            (!strcasecmp(pmime->mime_class,ContentClass))
#endif            
            )
        {
        		if (pmime->mime_class == ITEM_CLS_UPNP_AUDIO)
        			 *MType = MTYPE_AUDIO;
        		else if (pmime->mime_class == ITEM_CLS_UPNP_VIDEO)
        			 *MType = MTYPE_AV;
        		else if (pmime->mime_class == ITEM_CLS_UPNP_PHOTO)
       			 *MType = MTYPE_IMAGE;
            return pmime;
        }
        else pmime++;
    }

    return NULL;
}


static 
pbcm_upnp_object
get_container_from_class(pdirdbase_context db_context, char *mime_class)
{
    unsigned int        Count,ContID;
    pbcm_upnp_object    Cont=NULL;
    unsigned int			ItemCount = 0;

	 /* judge all item count can't be larger than MAX_ITEM_COUNT */
    ContID=ROOT_CONTAINER_ID;
    for(Count=0; Count < MAX_CONTAINERS; Count++)
    {  
        Cont = get_container(db_context,ContID);
        if(!Cont) break;

        if(!strcmp(Cont->title, "Root"))
        {
            ContID++;
            continue;
        }
        if(!strcmp(Cont->title,"Video"))
        {
            ItemCount += Cont->u.container_info.item_cnt;           
        }

        if(!strcmp(Cont->title,"Music"))
        {
            ItemCount += Cont->u.container_info.item_cnt;                      
        }

        if(!strcmp(Cont->title,"Photo"))
        {
            ItemCount += Cont->u.container_info.item_cnt;                  
        }
        ContID++;
    }
	 if(ItemCount > MAX_ITEM_COUNT)
	     return NULL;
	 //end
     
    ContID=ROOT_CONTAINER_ID;
    for(Count=0; Count < MAX_CONTAINERS; Count++)
    {  
        Cont = get_container(db_context,ContID);
        if(!Cont)
        {
            BcmLogMsg("%s:Container Not Found\n",__FUNCTION__);
            return NULL;
        }

        if(!strcmp(Cont->title, "Root"))
        {
            ContID++;
            continue;
        }
        if((!strcmp(Cont->title,"Video")) && 
            !(strcmp(mime_class,ITEM_CLS_UPNP_VIDEO)))
        {
            return Cont;
        }

        if((!strcmp(Cont->title,"Music")) && 
            !(strcmp(mime_class,ITEM_CLS_UPNP_AUDIO)))
        {
            return Cont;           
        }

        if((!strcmp(Cont->title,"Photo")) && 
            !(strcmp(mime_class,ITEM_CLS_UPNP_PHOTO)))
        {
            return Cont;          
        }
        ContID++;
    }
	 
    return NULL;
}

/* 
* Parse the file and find weather it is Audio,
* Video or AV file.    
*/


static
BOOL
ScanFileCallback(pcb_context pContext, 
                 PFOUND_FILE_INFO pFoundFile)
{
    char    FileExt[MAX_EXT_LEN],*pExt;
    pdirdbase_context db_context;
    struct    mime_type_t *pmime=NULL;
    pbcm_upnp_object Item=NULL,Cont=NULL;
    CONTENT_TYPE MType=MTYPE_UNKNOWN;

    pExt = FileExt;
    if(!pContext || !pFoundFile)
        return FALSE; 

    memset(FileExt,0,sizeof(FileExt));
    db_context = pContext->pdb_context;

    GetFileExtension(pFoundFile->FileName,&pExt);

    /* 
     * Minimal Parsing for Knowing the File Type. 
     * Full Parsing Happens Later
     */
#if 0 /* don't use ffmpeg to save time */     
    MType = GetMediaType(pFoundFile->FullFilePath);
#endif    
    pmime = GetMimeFromTable(FileExt, &MType); 

    if(pmime) /*We did find a entry in the table for this Mime type. We support this file*/
    {
        Cont = get_container_from_class(db_context,pmime->mime_class);
        if(Cont)
        {      
            pContext->StartID = db_context->objnum;
            Item = create_new_item(db_context,
                                    pContext->StartID,
                                    pFoundFile->FullFilePath,
                                    pFoundFile->FileSize,
                                    pmime->mime_class,
                                    pFoundFile->FileName,
                                    pmime->mime_protocol,
                                    MType);
            insert_item(Cont,Item);
            db_context->objnum ++;
            return TRUE;
        }else{
            BcmLogMsg("%s:Failed to Find Container\n",__FUNCTION__);
            return FALSE;
        }
    }

    return FALSE;
}

static 
void
ScanSharedDir(char *StartDir,            /* The Root for starting the Scan*/
        char ScanSubDir,        /* Scan The Sub Directories*/
        char *Extension,        /* The Files To Report eg. mpg*/
        NEW_FILE_FOUND pfnFoundFile,    /* The callback function when file is found*/
        void    *cb_context)
{
#define DMS_SHARE_PATH	"/var/dms_share_path"
	FILE *fp = NULL;
	char line[512] = {0};
	char musicDir[256] = {0};
	char photoDir[256] = {0};
	char videoDir[256] = {0};	
	char *ptr = line;
   char *utf8_path = NULL;
    
	if ((fp = fopen(DMS_SHARE_PATH, "r")) == NULL)
	{
		/* maybe DMS disable or use full /mnt dir */
		ScanDir(StartDir, ScanSubDir, Extension, pfnFoundFile, cb_context, 0);
		return;
	}
	
	while(fgets(line, sizeof(line), fp) != NULL)
	{
		/* must be valid media path */
		if (strlen(line) <= 7)
			break;

		line[strlen(line) - 1] = '\0';

		/* for music path */
		if ((line[0] == 'm') && (line[1] == 'u') && (line[2] == 's') &&
				(line[3] == 'i') && (line[4] == 'c'))
		{
			snprintf(musicDir, sizeof(line), "%s/", ptr+strlen("music="));		
		} else if ((line[0] == 'p') && (line[1] == 'h') && (line[2] == 'o') &&
				(line[3] == 't') && (line[4] == 'o'))
		{
			snprintf(photoDir, sizeof(line), "%s/", ptr+strlen("photo="));	
		} else if ((line[0] == 'v') && (line[1] == 'i') && (line[2] == 'd') &&
				(line[3] == 'e') && (line[4] == 'o'))
		{
			snprintf(videoDir, sizeof(line), "%s/", ptr+strlen("video="));
		}
	}

	fclose(fp);

	if (videoDir[0] != '\0') {	
      ScanDir(videoDir, ScanSubDir, Extension, pfnFoundFile, cb_context, 1);      
   }			
	if (musicDir[0] != '\0') {
      ScanDir(musicDir, ScanSubDir, Extension, pfnFoundFile, cb_context, 2);
   }
	if (photoDir[0] != '\0') {
      ScanDir(photoDir, ScanSubDir, Extension, pfnFoundFile, cb_context, 3);
   }
}

static 
int
InitiateScan(pdirdbase_context    db_context, 
             unsigned int        StartID,
             char                *search_mime_class,
             char                *ContentDir)
{
    char *ScanRootDir=CONTENT_DIRECTORY;
    cb_context  context;
    context.StartID = StartID;
    context.pdb_context = db_context;
    context.mime_class = search_mime_class;

    if(ContentDir) ScanRootDir = ContentDir;

/* Modified by tuhanyu, to call ScanDir fun directly, 2011/04/02, start */
#if 1
    ScanDir(ScanRootDir, TRUE, "*", (NEW_FILE_FOUND) ScanFileCallback, (void *) &context, 0);
#else
    ScanSharedDir(ScanRootDir,
        TRUE,
        "*",
        (NEW_FILE_FOUND) ScanFileCallback,
        (void *) &context);
#endif
/* Modified by tuhanyu, end */
    return context.StartID;
}

static
BOOL 
create_dummy_dbase(pdirdbase_context dbase_context,
                   char *ContentDir, unsigned int ObjectID)
{
    unsigned int Count;    
    unsigned int ContID=ROOT_CONTAINER_ID,ItemID=START_ITEM_ID;
    char * ContTitleArray[MAX_CONTAINERS - MAX_TUNER_CONTAINERS] = {"Root","Video","Music","Photo"};

    if(!dbase_context)
        return FALSE;

    if(dbase_context->DirRoot)
        return FALSE;

    for(Count=0; Count < (MAX_CONTAINERS - MAX_TUNER_CONTAINERS); Count++) //create containers other than tuner containers
    {
        if(FALSE == insert_container(dbase_context,
                                        dbase_context->DirRoot,
                                        ContID,
                                        ContTitleArray[Count]))
        {
            BcmLogMsg("%s:Failed to Insert Container\n",__FUNCTION__);
            return FALSE;
        }
        ContID++;
    }

//    if(ObjectID != ROOT_CONTAINER_ID)
    {
        dbase_context->objnum = ItemID;
        ItemID = InitiateScan(dbase_context,
                                ItemID,
                                ITEM_CLS_UPNP_VIDEO,
                                ContentDir);
    
        if(ItemID == START_ITEM_ID)
        {
            BcmLogMsg("%s:Failed to Insert Container\n",__FUNCTION__);
        }
    }
    
    if(dbase_context->NonDlnaProtocols->buf)
        buffer_appendf(dbase_context->SrcProtocol,",%s",dbase_context->NonDlnaProtocols->buf);

    return TRUE;
}

/*API Functions Which will be used to access the database*/

/* We do not want to differentiate between container and 
 * items. So lets abstract that here.
 */

pbcm_upnp_object 
dbget_item_from_cont(pbcm_upnp_object pcont, unsigned int index)
{
    unsigned int count;
    pbcm_upnp_object  item = NULL;

    if(!pcont || !IS_CONTAINER(pcont))
        return NULL;

    /*Take care of items in Root, which will only be containers*/
    count=0;
    if(IS_ROOT(pcont))
    {
        item = pcont->next_container;
        while(item)
        {
            if(count == index)
                return item;
            else{
                item = item->next_container;    
                count++;
            }
        }
        return NULL;
    }
    
    /* 
     * Take care of item from NON-Root Container. Non Root containers
     * will only have items and no containers.
     */
    count=0;
    item = pcont->next_item;
    while(item)
    {
        if(count == index)
            return item;
        else{
            item = item->next_item;    
            count++;
        }
    }

    return NULL;
}

pbcm_upnp_object
dbget_object(void * db_context, unsigned int ObjectID)
{
    pbcm_upnp_object ret_obj=NULL;
    ret_obj = get_container(db_context,ObjectID);
    if(ret_obj)
        return ret_obj;

    /*This means that we did not find the container with this ID*/
    ret_obj = get_item(db_context,ObjectID);
    return ret_obj;
}
char *
dbget_src_protocol_string(pdirdbase_context db_ctx)
{
    return db_ctx->SrcProtocol->buf;
}

void * 
init_database(char *cds_url,
              char *ContentDir, unsigned int ObjectID)
{
    char ip[30], path[90], prot[5];
    int port;

    pdirdbase_context pdb_context=NULL;
    pdb_context = (pdirdbase_context) malloc(sizeof(dirdbase_context));
    if(!pdb_context) return NULL;
    memset(pdb_context,0,sizeof(dirdbase_context));
    /* Fill in the base url so that every-time we add a file
     * we will add the url for each file.
     */
    sprintf(pdb_context->base_file_url,"%s%s",cds_url,VIRTUAL_DIRECTORY);
    
    /* Use the cds_url "port+1" for broadcoast items */
    sscanf(pdb_context->base_file_url, "%4[^:]://%29[^:]:%6d/%89s", prot, ip, &port, path); 
    port++;
    sprintf(pdb_context->base_broadcast_url, "%s://%s:%d/%s", prot, ip, port, path);

    //printf("File Http Base URL=%s\n", pdb_context->base_file_url);
    //printf("Broadcast Http Base URL=%s\n", pdb_context->base_broadcast_url);

    pdb_context->SrcProtocol = buffer_new();
    pdb_context->NonDlnaProtocols = buffer_new();
#ifdef HAVE_DLNA
    pdb_context->dlna_context = dlna_init();
    dlna_register_all_media_profiles(pdb_context->dlna_context);
#endif
    create_dummy_dbase(pdb_context,ContentDir,ObjectID);

    if(pdb_context->objnum == START_ITEM_ID)
    {
        finish_database(pdb_context);
        return NULL;
    }
    else
    {
        return pdb_context;
    }
}

void
finish_database(pdirdbase_context db_ctx)
{
    pcleanup_item    temp=NULL,next_item=NULL;    
    pbcm_upnp_object item = NULL;

    next_item = db_ctx->freelist;
    db_ctx->freelist = NULL;
    while(next_item)
    {
        temp = next_item;
        next_item = next_item->next_item;

        item = temp->Address;
        if(item->u.item_info.u.file.artist)
            free(item->u.item_info.u.file.artist);
        if(item->u.item_info.u.file.track)
            free(item->u.item_info.u.file.track);
        if(item->u.item_info.u.file.album)
            free(item->u.item_info.u.file.album);
        if(item->u.item_info.u.file.genre)
            free(item->u.item_info.u.file.genre);
        if(item->u.item_info.u.file.date)
            free(item->u.item_info.u.file.date);    
        
        free(temp->Address);
        free(temp);
        db_ctx->freecnt--;
    }

    if(db_ctx->freecnt)
        BcmLogMsg("%s","Did not free all the allocated memory");
        
    if(db_ctx->DirRoot->u.item_info.u.file.artist)
        free(db_ctx->DirRoot->u.item_info.u.file.artist);
    if(db_ctx->DirRoot->u.item_info.u.file.track)
        free(db_ctx->DirRoot->u.item_info.u.file.track);
    if(db_ctx->DirRoot->u.item_info.u.file.album)
        free(db_ctx->DirRoot->u.item_info.u.file.album);
    if(db_ctx->DirRoot->u.item_info.u.file.genre)
        free(db_ctx->DirRoot->u.item_info.u.file.genre);
    if(db_ctx->DirRoot->u.item_info.u.file.date)
        free(db_ctx->DirRoot->u.item_info.u.file.date);        
        
#ifdef HAVE_DLNA
    dlna_uninit(db_ctx->dlna_context);
#endif
    buffer_free(db_ctx->SrcProtocol);
    buffer_free(db_ctx->NonDlnaProtocols);
    free(db_ctx);
}

char* 
GetMimeFromProtocol(pbcm_upnp_object pobject)
{
    char *pProto,*x; 
    if(GetParentID(pobject) < TUNER_CONTAINER_ID)
        pProto = ITEM_GET_FILE_PROTOCOL(pobject);
    else
        pProto = ITEM_GET_CHANNEL_PROTOCOL(pobject);
    x= strdup(&pProto[11]);
    strtok(x,":");
    return x;
}

int GetParentID(pbcm_upnp_object pobject)
{
    if(IS_ROOT(pobject))
        return -1;
    else
        return pobject->parent->Id;
}

unsigned int
GetChildCount(pbcm_upnp_object pobject)
{
    if(IS_CONTAINER(pobject)) 
        return (GET_CONT_CNT(pobject) + GET_ITEM_CNT(pobject));

    return 0; /*Since Items cannot have childs*/
}


char *
GetMimeClass(pbcm_upnp_object pobject)
{
    return pobject->mime_class;
}

/* Add by tuhanyu, for create/destory object and import/delete resource action, 2011/05/03,  start */
BOOL
cds_add_media_file(pdirdbase_context db_context, PFOUND_FILE_INFO pFoundFile, unsigned int id)
{
    char    FileExt[MAX_EXT_LEN],*pExt;
    struct    mime_type_t *pmime=NULL;
    pbcm_upnp_object Item=NULL,Cont=NULL;
    CONTENT_TYPE MType=MTYPE_UNKNOWN;

    pExt = FileExt;
    if(!pFoundFile)
        return FALSE; 

    memset(FileExt, 0, sizeof(FileExt));
    GetFileExtension(pFoundFile->FileName,&pExt);
    pmime = GetMimeFromTable(FileExt, &MType); 

    if(pmime) /*We did find a entry in the table for this Mime type. We support this file*/
    {
        if(Cont = get_container_from_class(db_context, pmime->mime_class))
        {
            Item = create_new_item(db_context,
                                    id,
                                    pFoundFile->FullFilePath,
                                    pFoundFile->FileSize,
                                    pmime->mime_class,
                                    pFoundFile->FileName,
                                    pmime->mime_protocol,
                                    MType);
            insert_item(Cont,Item);
            return TRUE;
        }else{
            return FALSE;
        }
    }

    return FALSE;
}
/* Add by tuhanyu, end */

