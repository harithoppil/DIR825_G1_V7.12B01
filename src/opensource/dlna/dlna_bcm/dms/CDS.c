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
 * $brcm_Workfile: CDS.c $
 * $brcm_Revision: 5 $
 * $brcm_Date: 9/25/09 3:37p $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: /AppLibs/dlna/dms/CDS.c $
 * 
 * 5   9/25/09 3:37p ismailk
 * SW7405-3080: DMS IP Streaming (Live channel) Feature.
 * 
 * 4   7/25/09 7:16p ajitabhp
 * PR55165: Variable arguments problem.
 * 
 * 3   7/25/09 7:06p ajitabhp
 * PR55165: Variable arguments was not processed properly.
 * 
 * 2   6/29/09 12:12p ajitabhp
 * PR55165: Added Path Input From User.
 * 
 *****************************************************************************/
#include<sys/types.h>
#include<dirent.h>

#include "CmsCdsShare.h"
#include "CDS.h"
#include "trace.h"
#include "DMS.h"
#include "dirdbase.h"
#include "upnp_priv.h"

void * virdir_context=NULL;
void * new_virdir_context=NULL;
unsigned int scan_flag = 0;

extern DMS_CONTEXT    gDmsContext;

/* DIDL parameters */
/* Represent the CDS DIDL Message Header Namespace. */
#define DIDL_NAMESPACE \
    "xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" " \
    "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" " \
    "xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\""

/* Represent the CDS DIDL Message Header Tag. */
#define DIDL_LITE "DIDL-Lite"

/* Represent the CDS DIDL Message Item value. */
#define DIDL_ITEM "item"

/* Represent the CDS DIDL Message Item ID value. */
#define DIDL_ITEM_ID "id"

/* Represent the CDS DIDL Message Item Parent ID value. */
#define DIDL_ITEM_PARENT_ID "parentID"

/* Represent the CDS DIDL Message Item Restricted value. */
#define DIDL_ITEM_RESTRICTED "restricted"

/* Represent the CDS DIDL Message Item UPnP Class value. */
#define DIDL_ITEM_CLASS "upnp:class"

/* Represent the CDS DIDL Message Item UPnP Artist value. */
#define DIDL_ITEM_ARTIST "upnp:artist"

/* Represent the CDS DIDL Message Item UPnP Genre value. */
#define DIDL_ITEM_GENRE "upnp:genre"

/* Represent the CDS DIDL Message Item UPnP Album value. */
#define DIDL_ITEM_ALBUM "upnp:album"

/* Represent the CDS DIDL Message Item UPnP OriginalTrackNumber value. */
#define DIDL_ITEM_ORIGINALTRACKNUMBER "upnp:originalTrackNumber"

/* Represent the CDS DIDL Message Item Title value. */
#define DIDL_ITEM_TITLE "dc:title"

/* Represent the CDS DIDL Message Item Creator value. */
#define DIDL_ITEM_CREATOR "dc:creator"

/* Represent the CDS DIDL Message Item Date value. */
#define DIDL_ITEM_DATE "dc:date"

/* Represent the CDS DIDL Message broadcast item channel number. */
#define DIDL_ITEM_CHANNEL_NUMBER "upnp:channelNr"

/* Represent the CDS DIDL Message broadcast item channel name. */
#define DIDL_ITEM_CHANNEL_NAME "upnp:channelName"


/* Represent the CDS DIDL Message Item Resource value. */
#define DIDL_RES "res"

/* Represent the CDS DIDL Message Item Protocol Info value. */
#define DIDL_RES_INFO "protocolInfo"

/* Represent the CDS DIDL Message Item Resource Size value. */
#define DIDL_RES_SIZE "size"

/* Represent the CDS DIDL Message Container value. */
#define DIDL_CONTAINER "container"

/* Represent the CDS DIDL Message Container ID value. */
#define DIDL_CONTAINER_ID "id"

/* Represent the CDS DIDL Message Container Parent ID value. */
#define DIDL_CONTAINER_PARENT_ID "parentID"

/* Represent the CDS DIDL Message Container number of children value. */
#define DIDL_CONTAINER_CHILDS "childCount"

/* Represent the CDS DIDL Message Container Restricted value. */
#define DIDL_CONTAINER_RESTRICTED "restricted"

/* Represent the CDS DIDL Message Container Searchable value. */
#define DIDL_CONTAINER_SEARCH "searchable"

/* Represent the CDS DIDL Message Container UPnP Class value. */
#define DIDL_CONTAINER_CLASS "upnp:class"

/* Represent the CDS DIDL Message for Container type value*/
#define DIDL_CONTAINER_TYPE "dlna:containerType"


/* Represent the CDS DIDL Message Container Title value. */
#define DIDL_CONTAINER_TITLE "dc:title"

/*Escaped XML Characters*/
#define GT    "&gt;"
#define LT    "&lt;"


static 
BOOL
filter_has_val (const char *filter, 
                const char *val)
{
    char *x = NULL, *token = NULL;
    char *m_buffer = NULL, *buffer;
    int len = strlen (val);
    BOOL ret = FALSE;

    if (!strcmp (filter, "*"))
        return TRUE;

    x = strdup (filter);
    if (x)
    {
        m_buffer = (char*) malloc (strlen (x));
        if (m_buffer)
        {
            buffer = m_buffer;
            token = strtok_r(x, ",", &buffer);
            while (token)
            {
                if (*val == '@')
                    token = strchr (token, '@');
                if (token && !strncmp (token, val, len))
                {
                    ret = TRUE;
                    break;
                }
                token = strtok_r (NULL, ",", &buffer);
            }
            free (m_buffer);
        }
        free (x);
    }
    return ret;
}


static void
didl_add_header (struct buffer_t *out)
{
    buffer_appendf (out, "%s%s %s%s", LT,DIDL_LITE, DIDL_NAMESPACE,GT);
}

static void
didl_add_footer (struct buffer_t *out)
{
    buffer_appendf (out, "%s/%s%s",LT,DIDL_LITE,GT);
}

static void
didl_add_tag (struct buffer_t *out, 
              char *tag, 
              char *value)
{
    if (value)
        //buffer_appendf (out, "<%s>%s</%s>",tag,value,tag);
        buffer_appendf (out, "%s%s%s%s%s/%s%s",LT,tag,GT,value,LT,tag,GT);
}

static void
didl_add_tag_value (struct buffer_t *out, 
              char *tag, 
              int value)
{
    if (value)
        //buffer_appendf (out, "<%s>%s</%s>",tag,value,tag);
        buffer_appendf (out, "%s%s%s%d%s/%s%s",LT,tag,GT,value,LT,tag,GT);
}


static void
didl_add_param (struct buffer_t *out, 
                char *param, 
                char *value)
{
    if (value)
        buffer_appendf (out, " %s=\"%s\"", param, value);
}

static void
didl_add_value (struct buffer_t *out, 
                char *param, 
                int value)
{
    buffer_appendf (out, " %s=\"%d\"", param, value);
}

static void
didl_add_item (struct buffer_t *out, 
               int item_id,
               int parent_id, 
               char *restricted, 
               char *class, 
               char *title,
               char *protocol_info, 
               int size, 
               char *url, 
               char *filter,
               char *artist,
               char *track,
               char *album,
               char *genre,
               char *date)
{
    buffer_appendf (out, "%s%s",LT, DIDL_ITEM);
    didl_add_value (out, DIDL_ITEM_ID, item_id);
    didl_add_value (out, DIDL_ITEM_PARENT_ID, parent_id);
    didl_add_param (out, DIDL_ITEM_RESTRICTED, restricted);
    buffer_appendf (out, "%s",GT);

    didl_add_tag (out, DIDL_ITEM_CLASS, class);
    if(artist)
	     didl_add_tag (out, DIDL_ITEM_CREATOR, artist);
    didl_add_tag (out, DIDL_ITEM_TITLE, title);
    if(date)
	     didl_add_tag (out, DIDL_ITEM_DATE, date);    

    if(artist)
	     didl_add_tag (out, DIDL_ITEM_ARTIST, artist);
	 if(genre)
	     didl_add_tag (out, DIDL_ITEM_GENRE, genre);
	 if(album)
        didl_add_tag (out, DIDL_ITEM_ALBUM, album);
    if(track)
        didl_add_tag (out, DIDL_ITEM_ORIGINALTRACKNUMBER, track);    
    
    if (filter_has_val (filter, DIDL_RES))
    {
        buffer_appendf (out, "%s%s",LT,DIDL_RES);
        // protocolInfo is required :
        didl_add_param (out, DIDL_RES_INFO, protocol_info);
        if (filter_has_val (filter, "@"DIDL_RES_SIZE))
            didl_add_value (out, DIDL_RES_SIZE, size);
        buffer_appendf (out, "%s",GT);
        if (url)
        {
            buffer_appendf(out,"%s",url);
        }
        buffer_appendf (out, "%s/%s%s",LT,DIDL_RES,GT);
    }
    

    buffer_appendf (out, "%s/%s%s",LT,DIDL_ITEM,GT);
}

static void
didl_add_broadcast_item (struct buffer_t *out, 
               int item_id,
               int parent_id, 
               char *restricted, 
               char *class, 
               char *title,
               char *protocol_info,                
               char *url,
               unsigned int channel_number,
               char* channel_name,
               char *filter               
               )
{
    buffer_appendf (out, "%s%s",LT, DIDL_ITEM);
    didl_add_value (out, DIDL_ITEM_ID, item_id);
    didl_add_value (out, DIDL_ITEM_PARENT_ID, parent_id);
    didl_add_param (out, DIDL_ITEM_RESTRICTED, restricted);
    buffer_appendf (out, "%s",GT);

    didl_add_tag (out, DIDL_ITEM_CLASS, class);
    didl_add_tag (out, DIDL_ITEM_TITLE, title);
    didl_add_tag_value(out, DIDL_ITEM_CHANNEL_NUMBER, channel_number);
    didl_add_tag (out, DIDL_ITEM_CHANNEL_NAME, channel_name);

    if (filter_has_val (filter, DIDL_RES))
    {
        buffer_appendf (out, "%s%s",LT,DIDL_RES);
        // protocolInfo is required :
        didl_add_param (out, DIDL_RES_INFO, protocol_info);
        buffer_appendf (out, "%s",GT);
        if (url)
        {
            buffer_appendf(out,"%s",url);
        }
        buffer_appendf (out, "%s/%s%s",LT,DIDL_RES,GT);
    }
    

    buffer_appendf (out, "%s/%s%s",LT,DIDL_ITEM,GT);
    //printf("didl_add_broadcast_item xml = %s\n",out->buf);
}


static void
didl_add_container (struct buffer_t *out, int id, int parent_id,
                    int child_count, char *restricted, char *searchable,
                    char *title, char *class, char *filter)
{

    buffer_appendf (out, "%s%s",LT,DIDL_CONTAINER);
    didl_add_value (out, DIDL_CONTAINER_ID, id);
    didl_add_value (out, DIDL_CONTAINER_PARENT_ID, parent_id);

    if( (child_count >= 0) && (filter_has_val(filter,"@"DIDL_CONTAINER_CHILDS)))
        didl_add_value (out, DIDL_CONTAINER_CHILDS, child_count);


    didl_add_param (out, DIDL_CONTAINER_RESTRICTED, restricted);
    didl_add_param (out, DIDL_CONTAINER_SEARCH, searchable);
    buffer_appendf (out, "%s",GT);
    
    if(id >= TUNER_CONTAINER_ID)
    {
        didl_add_tag (out, DIDL_CONTAINER_TYPE, "Tuner_1_0");
    }

    didl_add_tag (out, DIDL_CONTAINER_CLASS, class);
    didl_add_tag (out, DIDL_CONTAINER_TITLE, title);

    buffer_appendf (out, "%s/%s%s",LT,DIDL_CONTAINER,GT);
}


/* Our Metadata will be structured in such a way that 
 * there will always be a ROOT entry, This will be the
 * only entry returned in the BrowseMetadata directory.
 *      
 */
static 
BC_STATUS
cds_browse_metadata(pbcm_upnp_object        pobject,
                    unsigned int            max_count,
                    char                    *filter,
                    pcds_browse_out_params  out_params)
{

    struct buffer_t            *buff_out=NULL;

    if(!out_params || !out_params->ResultOutBuff){
        return BC_UPNP_E_INVALID_ARG;
    }

    buff_out = out_params->ResultOutBuff;

    out_params->TotalMatches = 1;
    out_params->NumberReturned = 1; 
    out_params->UpdateID = GET_SYSTEM_UPDATE_ID; /*Should come from the database*/
    didl_add_header(buff_out);
    if(IS_CONTAINER(pobject))
    {
        didl_add_container(buff_out,
                            GET_OBJECT_ID(pobject),
                            GetParentID(pobject),
                            GetChildCount(pobject),
                            "true",
                            NULL,
                            GET_TITLE(pobject),
                            GetMimeClass(pobject),
                            filter);
    }else{
        if(GetParentID(pobject) >= TUNER_CONTAINER_ID){
                didl_add_broadcast_item(buff_out,
                GET_OBJECT_ID(pobject),
                GetParentID(pobject),
                "true",
                GetMimeClass(pobject),
                GET_TITLE(pobject),
                ITEM_GET_CHANNEL_PROTOCOL(pobject),                
                ITEM_GET_CHANNEL_URL(pobject),
                ITEM_GET_CHANNEL_NUMBER(pobject),
                ITEM_GET_CHANNEL_NAME(pobject),
                filter);    
        }
        else{
                didl_add_item(buff_out,
                GET_OBJECT_ID(pobject),
                GetParentID(pobject),
                "true",
                GetMimeClass(pobject),
                GET_TITLE(pobject),
                ITEM_GET_FILE_PROTOCOL(pobject),
                ITEM_GET_SIZE(pobject),
                ITEM_GET_FILE_URL(pobject),
                filter,
                ITEM_GET_ARTIST(pobject),
                ITEM_GET_TRACK(pobject),
                ITEM_GET_ALBUM(pobject),
                ITEM_GET_GENRE(pobject),
                ITEM_GET_DATE(pobject));    
        }
    }    

    didl_add_footer(buff_out);
    
    return BC_UPNP_E_SUCCESS;
}

static 
BC_STATUS
cds_browse_children(pbcm_upnp_object pobject,
                    unsigned int  start_ind,
                    unsigned int req_count,
                    char *         filter,
                    pcds_browse_out_params out_params)
{
    struct buffer_t            *buff_out=NULL;
    unsigned int            count,result_count;
    pbcm_upnp_object        child_entry=NULL;

    if(!pobject || !out_params)
        return BC_UPNP_E_INVALID_ARG;
    

    if(!req_count)
        req_count = GetChildCount(pobject);

    buff_out = out_params->ResultOutBuff;
    didl_add_header(buff_out);

    result_count=0;    
    for(count=0;count < req_count;count++)
    {
        child_entry = dbget_item_from_cont(pobject,start_ind+count);
        if(!child_entry) 
            break; /*We reached the end*/
    
        if(IS_CONTAINER(child_entry)){
            didl_add_container(buff_out,
                                GET_OBJECT_ID(child_entry),
                                GetParentID(child_entry),
                                GetChildCount(child_entry),
                                "true",
                                NULL,
                                GET_TITLE(child_entry),
                                GetMimeClass(child_entry),
                                filter);
        }else{
            if(GetParentID(child_entry) >= TUNER_CONTAINER_ID){                
                didl_add_broadcast_item(buff_out,
                GET_OBJECT_ID(child_entry),
                GetParentID(child_entry),
                "true",
                GetMimeClass(child_entry),
                GET_TITLE(child_entry),
                ITEM_GET_CHANNEL_PROTOCOL(child_entry),                
                ITEM_GET_CHANNEL_URL(child_entry),
                ITEM_GET_CHANNEL_NUMBER(child_entry),
                ITEM_GET_CHANNEL_NAME(child_entry),
                filter);
            }
            else{                
                didl_add_item(buff_out,
                GET_OBJECT_ID(child_entry),
                GetParentID(child_entry),
                "true",
                GetMimeClass(child_entry),
                GET_TITLE(child_entry),
                ITEM_GET_FILE_PROTOCOL(child_entry),
                ITEM_GET_SIZE(child_entry),
                ITEM_GET_FILE_URL(child_entry),
                filter,
                ITEM_GET_ARTIST(child_entry),
                ITEM_GET_TRACK(child_entry),
                ITEM_GET_ALBUM(child_entry),
                ITEM_GET_GENRE(child_entry),
                ITEM_GET_DATE(child_entry));
            }
        }

        result_count++;
    }

    /* The total matches should return the total match found depending on filter
     * right now we are just filling everything, later we will have to acknowledge the filter.
     */
    out_params->TotalMatches = GetChildCount(pobject);
    out_params->NumberReturned = result_count; 
    out_params->UpdateID = GET_SYSTEM_UPDATE_ID; /*Should come from the database*/
    didl_add_footer(buff_out);
    return BC_UPNP_E_SUCCESS;
}


static
unsigned int 
GetIdFromUrl(const char *FileUrl)
{
    unsigned int id=0;
    char len;
    const char *peek=NULL;
    peek =  &FileUrl[strlen(FileUrl) - 1];
    len = strlen(FileUrl);
    while(peek >= (FileUrl + 2))
    {
        if(*peek == '=' && *(peek-1) == 'd' && *(peek-2) == 'i')
        {
            id = atoi(peek+1);
            break;
        }
        peek--;
    }

    return id;
}

/************************************************************************/
/*            Exported API                                                */
/************************************************************************/

BC_STATUS
cds_browse(pcds_browse_in_params   in_params, 
           pcds_browse_out_params  out_params)
{
    BC_STATUS            ret_sts=BC_UPNP_E_SUCCESS;
    pbcm_upnp_object    pobject=NULL;
    BOOL                metadata=FALSE;

    if(!in_params || !out_params)
        return BC_UPNP_E_INVALID_ARG;

    if(!in_params->Filter || !in_params->BrowseFlag)
        return BC_UPNP_E_INVALID_ARG;

    /*make sure that we have a DIDL buffer for output*/
    if(!out_params->ResultOutBuff)
        return BC_UPNP_E_INVALID_ARG;

    if(!strcmp(in_params->BrowseFlag, CDS_BROWSE_METADATA))
    {
        /*Browse metadata is only for the root*/
        if(in_params->StartingIndex)
            return BC_UPNP_E_INVALID_ARG;
    
        metadata=TRUE;
                    
    }else if(!strcmp(in_params->BrowseFlag, CDS_BROWSE_CHILDREN)){
        metadata=FALSE;
    }else{
        return BC_UPNP_E_INVALID_ARG;
    }

    /*Get the object to Browse*/
    pobject = dbget_object(virdir_context, in_params->ObjectID);
    if(!pobject)
        return BC_UPNP_E_ARGUMENT_VALUE_INVALID;

    if(metadata)
    {
        ret_sts = cds_browse_metadata(pobject,
                                            in_params->RequestedCount,
                                            in_params->Filter,
                                            out_params);
    }else{
        ret_sts = cds_browse_children(pobject,
                                            in_params->StartingIndex,
                                            in_params->RequestedCount,
                                            in_params->Filter,
                                            out_params);
    }

    if(ret_sts != BC_UPNP_E_SUCCESS)
        BcmLogMsg("Browse Error\n");
    return ret_sts;
}

BC_STATUS
cds_get_search_caps(char *search_caps)
{
    if(!search_caps)
        return BC_UPNP_E_INVALID_ARG;

    strcpy(search_caps,SEARCH_CAPS);
    return BC_UPNP_E_SUCCESS;
}

BC_STATUS
cds_get_sort_caps(char *sort_caps)
{
    if(!sort_caps)
        return BC_UPNP_E_INVALID_ARG;

    strcpy(sort_caps,SORT_CAPS);
    return BC_UPNP_E_SUCCESS;
}

unsigned int
cds_get_system_update_id()
{
    /*This should come from the data base*/
    return GET_SYSTEM_UPDATE_ID;
}

BC_STATUS
cds_init(char *cds_url,
         char *ContentDir, unsigned int ObjectID)
{
    char *SrcProtocol=NULL;
    if(!virdir_context)
    {
        virdir_context = init_database(cds_url,ContentDir,ObjectID);
        if(!virdir_context) /*Database initialization Failed*/
            return  BC_UPNP_E_ACTION_FAILED;
    }
    
    SrcProtocol = dbget_src_protocol_string(virdir_context);
    if(SrcProtocol) CmsUpdateSrcProtocolInfo(SrcProtocol);
    
     return BC_UPNP_E_SUCCESS;
}

/* Add by tuhanyu, change to scan media file periodically, 2011/04/02, start */
#if 0
BC_STATUS
cds_update()
{
    void *temp=NULL;
    char *SrcProtocol=NULL;
    
    char *cdsUrl = strdup(gDmsContext.ContentXferbaseurl);
    char *contentDir = strdup(gDmsContext.mediaPath);
    unsigned int updatePeriod = gDmsContext.updatePeriod;

    if(new_virdir_context)
    {
        temp = virdir_context;
        virdir_context = new_virdir_context;
        new_virdir_context = NULL;
        
        SrcProtocol = dbget_src_protocol_string(virdir_context);
        if(SrcProtocol) 
        {
            CmsUpdateSrcProtocolInfo(SrcProtocol);
        }
        
        if(temp)
        {
            finish_database(temp);
        }
    }
    else if(!virdir_context)
    {
        if(scan_flag == 1)
            return UPNP_E_CANNOT_PROCESS_THE_REQUEST;
        
        virdir_context = init_database(cdsUrl,contentDir,0);
        if(virdir_context)
        {
            SrcProtocol = dbget_src_protocol_string(virdir_context);
            if(SrcProtocol) CmsUpdateSrcProtocolInfo(SrcProtocol);
        }
    }
    
    return BC_UPNP_E_SUCCESS;
}
#else
BC_STATUS
cds_update()
{
    void *temp=NULL;
    char *SrcProtocol=NULL;
    
    char *cdsUrl = strdup(gDmsContext.ContentXferbaseurl);
    char *contentDir = strdup(gDmsContext.mediaPath);
    unsigned int updatePeriod = gDmsContext.updatePeriod;

    if(!virdir_context)
    {
            return UPNP_E_CANNOT_PROCESS_THE_REQUEST;
    }
    
    return BC_UPNP_E_SUCCESS;
}
#endif

#if 1
static void
bcm_dms_sigusr1_handler()
{
    void *temp=NULL;
    char *SrcProtocol=NULL;
    DIR * dir;

printf("DMS:CDS RCV SIGUSR1.\n");
    if(scan_flag)
    {
        printf("DMS: Scanning is doing, Cancel.\n");
        return;
    }

            scan_flag = 1;
            if(NULL == (dir = opendir(gDmsContext.mediaPath)))
            {
                printf("DMS: CDS Can not access the path %s for SIGUSR1.\n", gDmsContext.mediaPath);
                if(virdir_context)
                {
                    temp = virdir_context;
                    virdir_context = NULL;
                    finish_database(temp);
                }
                scan_flag = 0;
                return;
            }
            
            closedir(dir);
                
            if(new_virdir_context)
            {
                finish_database(new_virdir_context);
            }

            new_virdir_context = init_database(gDmsContext.ContentXferbaseurl,gDmsContext.mediaPath,0);

            if(new_virdir_context)
            {
                printf("DMS: CDS Scanning Done: Succeed.\n");
            
                temp = virdir_context;
                virdir_context = new_virdir_context;
                new_virdir_context = NULL;
                
                SrcProtocol = dbget_src_protocol_string(virdir_context);
                if(SrcProtocol) 
                {
                    CmsUpdateSrcProtocolInfo(SrcProtocol);
                }

                if(temp)
                {
                    finish_database(temp);
                }
                
            }
            else
            {
                printf("DMS: CDS Scanning Done: there isn't media file. \n");
            }
    scan_flag = 0;
    return;
}

void
cds_scan_thread()
{
    void *temp=NULL;
    char *SrcProtocol=NULL;
    DIR * dir;

    char *cdsUrl = strdup(gDmsContext.ContentXferbaseurl);
    char *contentDir = strdup(gDmsContext.mediaPath);
    unsigned int updatePeriod = gDmsContext.updatePeriod;

    signal(SIGUSR1, bcm_dms_sigusr1_handler);
    
    do 
    {
            if(scan_flag)
            {
                printf("DMS: CDS Scanning is doing, Check after %d seconds.\n", updatePeriod);
                sleep(updatePeriod);
                continue;
            }
            
            while(NULL == (dir = opendir(contentDir)))
            {
                printf("DMS: CDS Can not access the path %s, re-try it after %d seconds.\n", contentDir, updatePeriod);
                if(virdir_context)
                {
                    temp = virdir_context;
                    virdir_context = NULL;
                    finish_database(temp);
                }
                sleep(updatePeriod);
            }
            printf("DMS: Start Scanning\n");
            closedir(dir);
    
            if(scan_flag)
            {
                printf("DMS: CDS Scanning is doing, Check after %d seconds.\n", updatePeriod);
                sleep(updatePeriod);
                continue;
            }
            
            scan_flag = 1;
            if(new_virdir_context)
            {
                finish_database(new_virdir_context);
            }

            new_virdir_context = init_database(cdsUrl,contentDir,0);

            if(new_virdir_context)
            {
                printf("DMS: CDS Scanning Done Successfully, Update after %d seconds.\n", updatePeriod);
            
                temp = virdir_context;
                virdir_context = new_virdir_context;
                new_virdir_context = NULL;
                
                SrcProtocol = dbget_src_protocol_string(virdir_context);
                if(SrcProtocol) 
                {
                    CmsUpdateSrcProtocolInfo(SrcProtocol);
                }

                if(temp)
                {
                    finish_database(temp);
                }
                
            }
            else
            {
                printf("DMS: CDS Scanning Done: there isn't media file. Re-try it after %d seconds.\n", updatePeriod);
            }
            scan_flag = 0;
            
            sleep(updatePeriod);
    }while(1);
    
}
#else
void
cds_scan_thread()
{
    void *temp=NULL;
    char *SrcProtocol=NULL;

    char *cdsUrl = strdup(gDmsContext.ContentXferbaseurl);
    char *contentDir = strdup(gDmsContext.mediaPath);
    unsigned int updatePeriod = gDmsContext.updatePeriod;

    do 
    {
            if(new_virdir_context)
            {
/*
                temp = virdir_context;
                virdir_context = new_virdir_context;
                new_virdir_context = NULL;
                
                SrcProtocol = dbget_src_protocol_string(virdir_context);
                if(SrcProtocol) 
                {
                    CmsUpdateSrcProtocolInfo(SrcProtocol);
                }

                if(temp)
                {
                    finish_database(temp);
                }
*/
                temp = new_virdir_context;
                new_virdir_context = NULL;
                finish_database(temp);
            }

            scan_flag = 1;
            new_virdir_context = init_database(cdsUrl,contentDir,0);
            printf("DMS: CDS Scanning Done.\n");
            scan_flag = 0;
            sleep(updatePeriod);
            
    }while(1);
    
}
#endif
/* Add by tuhanyu, end */

BC_STATUS
cds_finish()
{
    if(virdir_context)
        finish_database(virdir_context);

    virdir_context = NULL;
    return BC_UPNP_E_SUCCESS;
}


pbcm_upnp_object
GetFileFromUrl(const char *FileUrl)
{
    unsigned int FileId=0;
    pbcm_upnp_object    pUpnpFileObject=NULL;
    if(!FileUrl) return NULL;
    FileId = GetIdFromUrl(FileUrl);    
    if(!FileId)    return NULL;
    pUpnpFileObject = dbget_object(virdir_context,FileId);

    if(!pUpnpFileObject)
        return NULL;

    if(IS_CONTAINER(pUpnpFileObject))    
        return NULL;
    
    return pUpnpFileObject;
}

char *
getXferURIfromCDS(char *url)
{
    unsigned int uriId = 0;
    pbcm_upnp_object pUpnpObject = NULL;
    char *uri = NULL;

    if (!url) return NULL;
    uriId = GetIdFromUrl(url);    
    if(!uriId) return NULL;

    pUpnpObject = dbget_object(virdir_context, uriId);
    if (!pUpnpObject) return NULL;

    if (IS_CONTAINER(pUpnpObject)) return NULL;

    uri = ITEM_GET_XFER_URL(pUpnpObject);
    return uri;
}

/* Add by tuhanyu, for create/destory object and import/delete resource action, 2011/05/03,  start */
int getItemAttribute(char *didl, char *item, char *attr, char *value)
{
    char temp[256];
    char *tag, *val, *tmp;

    memset(temp, 0, 256);
    sprintf(temp, "<%s", item);
    if(!(tag = strstr(didl, temp)))
    {
        printf("getItemAttribute: Invalid item.\n");
        return 1;
    }
    memset(temp, 0, 256);
    sprintf(temp, "</%s>", item);
    if(!(tmp = strstr(tag, temp)))
    {
        printf("getItemAttribute: Invalid item.\n");
        return 1;
    }
    *tmp = 0;
    memset(temp, 0, 256);
    sprintf(temp, "%s=", attr);
    if(!(val = strstr(tag, temp)))
    {
        printf("getItemAttribute: Invalid attr.\n");
        return 1;
    }
    *tmp = '<';
    val = val + strlen(temp) + 1;
    if(!(tmp = strchr(val, '\"')))
    {
        printf("getItemAttribute: Invalid attr.\n");
        return 1;
    }
    *tmp = 0;
    strcpy(value, val);
    *tmp = '\"';
    
    return 0;
}

int getItemValue(char *didl, char *item, char *value)
{
    char temp[256];
    char *tag, *val, *tmp;

    memset(temp, 0, 256);
    sprintf(temp, "<%s", item);
    if(!(tag = strstr(didl, temp)))
    {
        printf("getItemAttribute: Invalid item.\n");
        return 1;
    }
    memset(temp, 0, 256);
    sprintf(temp, "</%s>", item);
    if(!(tmp = strstr(tag, temp)))
    {
        printf("getItemAttribute: Invalid item.\n");
        return 1;
    }
    *tmp = 0;
    memset(temp, 0, 256);
    if(!(val = strchr(tag, '>')))
    {
        printf("getItemAttribute: Invalid attr.\n");
        return 1;
    }
    val = val + 1;
    strcpy(value, val);
    *tmp = '<';
    
    return 0;
}

int setItemAttribute(char **didl, char *item, char *attr, char *value)
{
    char temp[256];
    char *tag, *val, *tmp;
    char *new_didl;
    int len;

    len = strlen(*didl) + strlen(value) + 1;
    if(!(new_didl = (char *)malloc(len)))
    {
        printf("setItemAttribute: can not allocate memery.\n");
        return 1;
    }

    memset(temp, 0, 256);
    sprintf(temp, "<%s", item);
    if(!(tag = strstr(*didl, temp)))
    {
        printf("setItemAttribute: Invalid item.1\n");
        free(new_didl);
        return 1;
    }
    memset(temp, 0, 256);
    sprintf(temp, "</%s>", item);
    if(!(tmp = strstr(tag, temp)))
    {
        printf("setItemAttribute: Invalid item.2\n");
        free(new_didl);
        return 1;
    }
    *tmp = 0;
    memset(temp, 0, 256);
    sprintf(temp, "%s=", attr);
    if(!(val = strstr(tag, temp)))
    {
        printf("setItemAttribute: Invalid attr.\n");
        free(new_didl);
        return 1;
    }
    *tmp = '<';
    val = val + strlen(temp) + 1;
    if(!(tmp = strchr(val, '\"')))
    {
        printf("setItemAttribute: Invalid attr.\n");
        free(new_didl);
        return 1;
    }
    *tmp = 0;
    strcpy(new_didl, *didl);
    strcat(new_didl, value);
    *tmp = '\"';
    strcat(new_didl, tmp);

    free(*didl);
    *didl = new_didl;

    return 0;
}

int setItemValue(char **didl, char *item, char *value)
{
    char temp[256];
    char *tag, *tmp;
    char *new_didl;
    int len;

    len = strlen(*didl) + strlen(value) + 1;
    if(!(new_didl = (char *)malloc(len)))
    {
        printf("setItemValue: can not allocate memery.\n");
        return 1;
    }
    
    memset(temp, 0, 256);
    memset(new_didl, 0, len);
    sprintf(temp, "<%s", item);
    if(!(tag = strstr(*didl, temp)))
    {
        printf("setItemValue: Invalid item.1\n");
        free(new_didl);
        return 1;
    }
    if(!(tmp = strchr(tag, '>')))
    {
        printf("setItemValue: Invalid item.2\n");
        free(new_didl);
        return 1;
    }
    strncpy(new_didl, *didl, (tmp-*didl+1));
    strcat(new_didl, value);
    
    memset(temp, 0, 256);
    sprintf(temp, "</%s>", item);
    if(!(tmp = strstr(tag, temp)))
    {
        printf("setItemValue: Invalid item.3\n");
        free(new_didl);
        return 1;
    }
    strcat(new_didl, tmp);

    free(*didl);
    *didl = new_didl;
}

BC_STATUS
cds_destroy_object(pcds_destroy_object_in_params   in_params)
{
    BC_STATUS              ret_sts = BC_UPNP_E_SUCCESS;
    pbcm_upnp_object    pobject = NULL;
    pdirdbase_context     db_ctx;
    
    db_ctx = (pdirdbase_context)virdir_context;
    if(!(pobject = delete_from_free_list(db_ctx, in_params->ObjectID)))
    {
        return BC_UPNP_E_ARGUMENT_VALUE_INVALID;
    }

    delete_item(pobject);

    if(pobject->u.item_info.u.file.artist)
        free(pobject->u.item_info.u.file.artist);
    if(pobject->u.item_info.u.file.track)
        free(pobject->u.item_info.u.file.track);
    if(pobject->u.item_info.u.file.album)
        free(pobject->u.item_info.u.file.album);
    if(pobject->u.item_info.u.file.genre)
        free(pobject->u.item_info.u.file.genre);
    if(pobject->u.item_info.u.file.date)
        free(pobject->u.item_info.u.file.date);    

    free(pobject);

    return ret_sts;
}

BC_STATUS
cds_create_object(pcds_create_object_in_params   in_params, 
           pcds_create_object_out_params  out_params)
{
    BC_STATUS              ret_sts = BC_UPNP_E_SUCCESS;
    pbcm_upnp_object    pobject = NULL;
    char                         *didl;
    pbcm_upnp_object    NewItem=NULL;
    pdirdbase_context     db_ctx;
    char                         *temp;
    char                         value[256];

    db_ctx = (pdirdbase_context)virdir_context;
    if(!(pobject = dbget_object(db_ctx, in_params->ContainerID)))
    {
        return UPNP_E_NO_SUCH_CONTAINER;
    }

    if(!(pobject->cont_flags))
    {
        return UPNP_E_NO_SUCH_CONTAINER;
    }

    if(!in_params->Elements->buf)
    {
        return BC_UPNP_E_ARGUMENT_VALUE_INVALID;
    }
    if(!(didl = (char*)malloc(strlen(in_params->Elements->buf))))
    {
        return UPNP_E_OUT_OF_MEMORY;
    }
    nxml_unescape(in_params->Elements->buf, didl);

    memset(value, 0, 256);
    getItemAttribute(didl, DIDL_ITEM, DIDL_ITEM_ID, value);
    if(strlen(value))
    {
        printf("cds_create_object: Invalid ID value.\n");
        BUPnPFree(didl);
        return UPNP_E_BAD_METADATA;
    }
    memset(value, 0, 256);
    getItemAttribute(didl, DIDL_ITEM, DIDL_CONTAINER_PARENT_ID, value);
    if(!strlen(value) || in_params->ContainerID != atoi(value))
    {
        printf("cds_create_object: Invalid parent ID.\n");
        BUPnPFree(didl);
        return UPNP_E_BAD_METADATA;
    }
    memset(value, 0, 256);
    getItemAttribute(didl, DIDL_ITEM, DIDL_ITEM_RESTRICTED, value);
    if(strcmp(value, "0"))
    {
        printf("cds_create_object: Invalid restricted.\n");
        BUPnPFree(didl);
        return UPNP_E_BAD_METADATA;
    }

    if(!(NewItem = malloc(sizeof(bcm_upnp_object))))
    {
        printf("cds_create_object: alloc memery failed.\n");
        BUPnPFree(didl);
        return UPNP_E_OUT_OF_MEMORY;
    }

    add_to_free_list(db_ctx, NewItem);
    memset(NewItem, 0, sizeof(bcm_upnp_object));
    NewItem->cont_flags = 0; 
    NewItem->Id = db_ctx->objnum++;

    memset(value, 0, 256);
    getItemValue(didl, DIDL_ITEM_TITLE, value);
    if(strlen(value))
    {
        strcpy(NewItem->title, value);
    }
    else
    {
        strcpy(NewItem->title, "UNKNOWN");
    }
    memset(value, 0, 256);
    getItemValue(didl, DIDL_ITEM_CLASS, value);
    if(strlen(value))
    {
        strcpy(NewItem->mime_class, value);
    }
    else
    {
        strcpy(NewItem->mime_class, "UNKNOWN");
    }

    if(strstr(NewItem->mime_class, "audio"))
    {
       NewItem->u.item_info.u.file.MediaType = MTYPE_AUDIO;
    }
    else if(strstr(NewItem->mime_class, "video"))
    {
       NewItem->u.item_info.u.file.MediaType = MTYPE_VIDEO;
    }
    else if(strstr(NewItem->mime_class, "image"))
    {
       NewItem->u.item_info.u.file.MediaType = MTYPE_IMAGE;
    }
    else
    {
       NewItem->u.item_info.u.file.MediaType = MTYPE_UNKNOWN;
    }
    
    memset(value, 0, 256);
    getItemAttribute(didl, DIDL_ITEM, DIDL_ITEM_RESTRICTED, value);
    if(strlen(value))
    {
        strcpy(NewItem->u.item_info.u.file.protocol, value);
    }
    else
    {
        strcpy(NewItem->u.item_info.u.file.protocol, "UNKNOWN");
    }
    sprintf(NewItem->u.item_info.u.file.url,"%s/?id=%d", db_ctx->base_file_url, NewItem->Id);

    insert_item(pobject, NewItem);
    
    out_params->ObjectID = NewItem->Id;
    memset(value, 0, 256);
    sprintf(value, "%d", out_params->ObjectID);
    setItemAttribute(&(didl), DIDL_ITEM, DIDL_ITEM_ID, value);
    setItemValue(&(didl), DIDL_RES, NewItem->u.item_info.u.file.url);
    out_params->ResultOutBuff->buf = nxml_escape(didl);
    
    BUPnPFree(didl);
    
    return BC_UPNP_E_SUCCESS;
}

#define HTTP_RESPONSE_TIMEOUT    30000
int transfer_count = 0;
Url url[2];

int32_t cds_receive_data(int fp, HttpContextHandle hContext)
{
    int n, elapsedTime, startTime, readCount;
    fd_set fds;
    int32_t result, statusCode;
    struct timeval timeout;
    unsigned int bytes = 0;
    char *headerEnd;
    
    FD_ZERO (&fds);
    FD_SET (hContext->s, &fds);

    timeout.tv_sec = 30; /* do 30 second timeouts */
    timeout.tv_usec = 0;

    elapsedTime = 0;
    readCount = 0;
    hContext->parser.position = HttpParserPosition_Header;

    startTime = GetTickCount();

    do
    {
        n = select (hContext->s+1, &fds, NULL, NULL, &timeout);

        if ( n > 0 )
        {
            result = Http_Receive(hContext);

            if ( result == UPNP_E_SUCCESS )
            {
                readCount++;
                if ( hContext->parser.position == HttpParserPosition_Header )
                {
                    if(headerEnd = strstr(hContext->stream.buffer, "\r\n\r\n" ))
                    {
                        headerEnd = headerEnd + 4;
                        while ( headerEnd != NULL && (*headerEnd == ' ' || *headerEnd == '\r' || *headerEnd == '\n'))
                            headerEnd++;
                        hContext->parser.position = HttpParserPosition_Body;
                        hContext->stream.position = headerEnd - hContext->stream.buffer;
                    }
                }
                result = UPNP_E_INCOMPLETE;
            }
            else if ( result == UPNP_E_SOCKET_CLOSED )
            {
                if (readCount != 0)
                    result = UPNP_E_SUCCESS;
            }
        }
        else if (n < 0)
        {
            perror("poll error\n");
            result = UPNP_E_SELECT_FAILED;
        }
        else /* poll time out, so just continue */
            result = UPNP_E_INCOMPLETE;

        elapsedTime = GetTickCount();
        elapsedTime = elapsedTime-startTime;
        if (elapsedTime > HTTP_RESPONSE_TIMEOUT)
            result = UPNP_E_TIMEOUT;

        if(result != UPNP_E_TIMEOUT && hContext->parser.position == HttpParserPosition_Body)
        {
                bytes += hContext->stream.length - hContext->stream.position;
                write(fp, hContext->stream.buffer+hContext->stream.position, hContext->stream.length-hContext->stream.position);
                memset(hContext->stream.buffer, 0, hContext->stream.capacity);
                hContext->stream.length = 0;
                hContext->stream.position = 0;
        }
    } while (result != UPNP_E_TIMEOUT && result == UPNP_E_INCOMPLETE);

    Http_DestroyContext(hContext);

    return result;
}

void
cds_download_mediafile(void *data)
{
    SOCKET s = 0;
    Url srcUrl, dstUrl;
    HttpContext hContext;
    pbcm_upnp_object pobject;
    FOUND_FILE_INFO FileInfo;
    struct stat FileStat;
    pdirdbase_context db_ctx;
    int fp;
    char *tmp;
    char tmpStr[256];
    char header[1024];

    db_ctx = (pdirdbase_context)virdir_context;
    memset(&hContext, 0, sizeof(HttpContext));
    memcpy(&srcUrl, &url[0], sizeof(Url));
    memcpy(&dstUrl, &url[1], sizeof(Url));

    sprintf(tmpStr, "%s://%s", dstUrl.scheme, dstUrl.host);
    if(dstUrl.port)
        sprintf(tmpStr+strlen(tmpStr), ":%d", dstUrl.port);
    sprintf(tmpStr+strlen(tmpStr), "%s", dstUrl.path);
    if(dstUrl.query)
        sprintf(tmpStr+strlen(tmpStr), "?%s", dstUrl.query);
    if(!(pobject = GetFileFromUrl(tmpStr)))
    {
        Url_Destroy(&srcUrl);
        Url_Destroy(&dstUrl);
        printf("cds_download_mediafile: destination url error: %s\n", tmpStr);
        return ;
    }
    
    delete_from_free_list(db_ctx, pobject->Id);
    delete_item(pobject);
    
    hContext.s = BSocket_CreateTcpClient(srcUrl.host, srcUrl.port);
    if ( hContext.s <= 0 )
    {
        Url_Destroy(&srcUrl);
        Url_Destroy(&dstUrl);
        printf("cds_download_mediafile: create tcp client socket failed.\n");
        return ;
    }

    if (srcUrl.query != NULL)
        sprintf(tmpStr, "%s?%s", srcUrl.path, srcUrl.query);
    else
        strcpy(tmpStr, srcUrl.path);
    
    snprintf(header, sizeof(header)-1, 
        "GET %s HTTP/1.1\r\n"
        "HOST: %s:%d\r\n"
        "CONTENT-LENGTH: 0\r\n"
        "\r\n",
        tmpStr, srcUrl.host, srcUrl.port);
    
    if ( send(hContext.s, header, strlen(header), 0) < 0 )
    {
        Url_Destroy(&srcUrl);
        Url_Destroy(&dstUrl);
        closesocket(hContext.s);
        return;
    }

    sprintf(tmpStr, "/mnt/usb1_1/%s", pobject->title);
    if((fp = open(tmpStr, O_RDONLY)) >= 0)
    {
        printf("cds_receive_data: the file existed.\n");
        close(fp);
        Url_Destroy(&srcUrl);
        Url_Destroy(&dstUrl);
        closesocket(hContext.s);
        return ;
    }
    
    if((fp = open(tmpStr, O_WRONLY|O_CREAT)) <= 0)
    {
        printf("cds_receive_data: can not open destination file.\n");
        Url_Destroy(&srcUrl);
        Url_Destroy(&dstUrl);
        closesocket(hContext.s);
        return ;
    }
    
    strcpy(hContext.method, "GET");
    cds_receive_data(fp, &hContext);

    if(stat(tmpStr, &FileStat) < 0)
    {
        printf("cds_receive_data: stat media file failed.\n");
        close(fp);
        Url_Destroy(&srcUrl);
        Url_Destroy(&dstUrl);
        closesocket(hContext.s);
        return ;
    }
    
    FileInfo.FileName = pobject->title;
    FileInfo.FullFilePath = tmpStr;
    FileInfo.FileSize = FileStat.st_size;
    cds_add_media_file(db_ctx, &FileInfo, pobject->Id);

    if(pobject->u.item_info.u.file.artist)
        free(pobject->u.item_info.u.file.artist);
    if(pobject->u.item_info.u.file.track)
        free(pobject->u.item_info.u.file.track);
    if(pobject->u.item_info.u.file.album)
        free(pobject->u.item_info.u.file.album);
    if(pobject->u.item_info.u.file.genre)
        free(pobject->u.item_info.u.file.genre);
    if(pobject->u.item_info.u.file.date)
        free(pobject->u.item_info.u.file.date);    
    free(pobject);

    close(fp);
    closesocket(hContext.s);
    Url_Destroy(&srcUrl);
    Url_Destroy(&dstUrl);
    transfer_count--;
    
    return;
}

BC_STATUS
cds_delete_resource(char *resourceURI)
{
    BC_STATUS  ret_sts = BC_UPNP_E_SUCCESS;
    pbcm_upnp_object pobject;
    pdirdbase_context db_ctx;
    
    db_ctx = (pdirdbase_context)virdir_context;
    if(!resourceURI)
    {
        return BC_UPNP_E_ARGUMENT_VALUE_INVALID;
    }
    
    if(!(pobject = GetFileFromUrl(resourceURI)))
    {
        printf("cds_delete_resource: resource url error: %s\n", resourceURI);
        return BC_UPNP_E_ARGUMENT_VALUE_INVALID;
    }
    
    delete_from_free_list(db_ctx, pobject->Id);
    delete_item(pobject);

    remove(pobject->u.item_info.u.file.fullpath);
    
    if(pobject->u.item_info.u.file.artist)
        free(pobject->u.item_info.u.file.artist);
    if(pobject->u.item_info.u.file.track)
        free(pobject->u.item_info.u.file.track);
    if(pobject->u.item_info.u.file.album)
        free(pobject->u.item_info.u.file.album);
    if(pobject->u.item_info.u.file.genre)
        free(pobject->u.item_info.u.file.genre);
    if(pobject->u.item_info.u.file.date)
        free(pobject->u.item_info.u.file.date);    

    free(pobject);

    return ret_sts;
}
/* Add by tuhanyu, end */


