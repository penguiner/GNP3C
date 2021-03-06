#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netpacket/packet.h> //reference  struct sockaddr_ll                                                                                   
#include <net/if.h> // reference struct ifreq
#include <linux/if_ether.h> // reference ETH_P_ALL
#include <sys/ioctl.h> // reference SIOCGIFINDEX	
#include <syslog.h>
#include <malloc.h>
#include <pcre.h>

#include "project.h"
#include "tools.h"
#include "__tom.h"
#include "cJSON.h"

extern int __tom_send_content(struct Http *http,struct Email_info *email_info){
   int result = 0;

    if(http->type != PATTERN_REQUEST_HEAD)
       return 0;
    if(strstr(http->host,"mail.tom.com") == NULL)
       return 0; 
    if(strstr(http->uri,"/webmail/writemail/sendmail.action") == NULL)
       return 0;

    char *name;
    struct List_Node *value = (struct List_Node *)malloc(sizeof(struct List_Node));
    value->length = 0;
    value->data = NULL;    

    name = "to";
    get_first_value_from_name(http,name,value);
    if(value->length > 0 && value->data != NULL){
        copy_into_email_info_member(&email_info->to, value->data, value->length);
    }
    free_list_node(value);
    
    value->length = 0;
    value->data = NULL;
    name = "from";
    get_first_value_from_name(http,name,value);
    if(value->length > 0 && value->data != NULL){
        copy_into_email_info_member(&email_info->from, value->data, value->length);
    }
    free_list_node(value);

    value->length = 0;
    value->data = NULL;
    name = "subject";
    get_first_value_from_name(http,name,value);
    if(value->length > 0 && value->data != NULL){
        copy_into_email_info_member(&email_info->subject, value->data, value->length);
    }
    free_list_node(value);

    value->length = 0;
    value->data = NULL;
    name = "text";
    get_first_value_from_name(http,name,value);
    if(value->length > 0 && value->data != NULL){
        struct List_Node *value_modify = (struct List_Node *)malloc(sizeof(struct List_Node)); 
        add_html_head_tail(value->data, value->length,value_modify);
        copy_into_email_info_member(&email_info->content, value_modify->data, value_modify->length);
        free_list_node(value_modify);
        free(value_modify); 
        result = 1;
    }
    free_list_node(value);

    free(value); 
/*  
    if(result == 1){
       printf("__tom_send_content\n");
	   printf("from : [%s]\n", email_info->from);
	   printf("subject : [%s]\n", email_info->subject);
       printf("to : [%s]\n", email_info->to);
	   printf("content length : %d\n", strlen(email_info->content));
       printf("content : [%s]\n",email_info->content);
       printf("\n\n");
    }
*/

    return result;
}
extern int __tom_send_attachment(struct Http *http,struct Email_info *email_info){
   
   int result = 0;

   if(http->type != PATTERN_REQUEST_HEAD)
      return 0;
   if(strstr(http->host,"mail.tom.com") == NULL)
      return 0; 
   if(strstr(http->uri,"/webmail/writemail/dfsuploadAttachment.do") == NULL)
       return 0;

   struct Entity_List *entity_list;
   struct Entity *entity;

   entity_list = http->entity_list;

   if(entity_list != NULL){

        entity = entity_list->head;
        while(entity != NULL){

            if(entity->entity_length <= 0 || entity->entity_length > 1024*1024*100){
                  entity = entity->next;
                  continue;
            }

            if( strstr(entity->content_disposition_struct.type,"form-data") == NULL ){
                  entity = entity->next;
                  continue;
            }
           
            if( strstr(entity->content_disposition_struct.name,"Filename") != NULL){
                  copy_into_email_info_member(&email_info->att_filename,entity->entity_content, entity->entity_length);
                  entity = entity->next;
                  continue;
            }

            if( strstr(entity->content_disposition_struct.name,"Filedata") != NULL){
                  copy_into_email_info_member(&email_info->attachment, entity->entity_content, entity->entity_length);
                  email_info->att_length = entity->entity_length;
                  entity = entity->next;
                  result = 1;
                  continue;
            }

            entity = entity->next;

        }
    }
/*
   if(result == 1){
        printf("__tom_send_attachment\n");
   		printf("attachement length: %d\n", email_info->att_length);
   		printf("attachement name: [%s]\n",email_info->att_filename);
   		printf("attachement: [%s]\n",email_info->attachment);
        printf("\n\n");
   }
*/  
  return result;
}

extern int __tom_receive_content(struct Http *http,struct Email_info *email_info){  

   int result = 0;

   if(http->type != PATTERN_REQUEST_HEAD)
      return 0;
   if(strstr(http->host,"mail.tom.com") == NULL)
      return 0; 
   if(strstr(http->absolute_uri,"/coremail/fcg/ldmsapp") == NULL)
       return 0;

   char *name;
   struct List_Node *value = (struct List_Node *)malloc(sizeof(struct List_Node));
   value->length = 0;
   value->data = NULL;
   name = "funcid";
   get_first_value_from_name(http,name,value);
   if(value->length > 0 && value->data != NULL){
        if(strstr(value->data,"readlett") != NULL)
            result = 1;
   }
   free_list_node(value);
   free(value); 

   if(result == 0)
        return 0;

   if(http->matched_http != NULL && http->if_matched_http == HTTP_MATCH_YES){

       struct Http *another = http->matched_http;
       struct Entity_List *entity_list;
       struct Entity *entity;

       entity_list = another->entity_list;

       if(entity_list != NULL){
  
           entity = entity_list->head;
           while(entity != NULL){
              if(entity->entity_content != NULL && entity->entity_length > 0){
                
                char *buffer = (char *)malloc((entity->entity_length +1) * sizeof(char));
                memset(buffer,0,(entity->entity_length +1) * sizeof(char));
                pcre_repl("<div id=\"OrgInniHtml\" style=\"display:none\">", 
                          "<div id=\"OrgInniHtml\" style=\"display\">", entity->entity_content, entity->entity_length, 
                          buffer, PCRE_DOTALL|PCRE_MULTILINE);

                copy_into_email_info_member(&email_info->content, buffer, strlen(buffer));

                free(buffer);
                result = 1;
                break;// find one,then quit circle
              }

              entity = entity->next;
           }
       }

   }
/*
   if(result == 1){
      if(email_info->content != NULL){
        
          printf("content length: %d\n", strlen(email_info->content));
          printf("content: [%s]\n",email_info->content);
      }

      char file_name[100];
      memset(file_name,0,100);
      sprintf(file_name,"/home/safe/tom_receive_content.html_%d",strlen(email_info->content));

      write_data_to_file(file_name,email_info->content,strlen(email_info->content));

   }
*/

   return result;
}
extern int __tom_receive_attachment(struct Http *http,struct Email_info *email_info){
   int result = 0;

   if(http->type != PATTERN_REQUEST_HEAD)
      return 0;
   if(strstr(http->host,"mail.tom.com") == NULL)
      return 0; 

   if(http->matched_http != NULL && http->if_matched_http == HTTP_MATCH_YES){

       struct Http *another = http->matched_http;
       struct Entity_List *entity_list;
       struct Entity *entity;

       entity_list = another->entity_list;

       if(entity_list != NULL){
           entity = entity_list->head;
           while(entity != NULL){
              if(entity->entity_content != NULL && entity->entity_length > 0){
                 
                if( strstr(entity->content_disposition_struct.type,"attachment") && strlen(entity->content_disposition_struct.filename) > 0){
                     copy_into_email_info_member(&email_info->att_filename, \
                                              entity->content_disposition_struct.filename, \
                                              strlen(entity->content_disposition_struct.filename));
                     copy_into_email_info_member(&email_info->attachment, entity->entity_content, entity->entity_length);
                     email_info->att_length = entity->entity_length;
                     result = 1;
                     break;// find one,then quit circle
                }
              }
              entity = entity->next;
           }
       }

   }

   if(result == 1){
        printf("__tom_receive_attachment");
   		printf("attachement length: %d\n", email_info->att_length);
   		printf("attachement name: [%s]\n",email_info->att_filename);
   		printf("attachement: [%s]\n",email_info->attachment);
        printf("\n\n");
   }

   return result;
}
