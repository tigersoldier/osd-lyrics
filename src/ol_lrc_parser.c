#include "ol_lrc_parser.h"
#define isnumeric(a) (((a >= '0') && (a <= '9')) ? 1 : 0)


LrcInfo *ol_lrc_parser_get_first_Of_list(LrcQueue *list)
{
  return &list->list[list->first];
}

LrcInfo *ol_lrc_parser_get_last_Of_list(LrcQueue *list)
{
  return &list->list[list->last];
}

LrcInfo *ol_lrc_parser_get_next_of_lyric(LrcInfo *current_lyric)
{
  return current_lyric->next;
}

LrcInfo *ol_lrc_parser_get_prev_of_lyric(LrcInfo *current_lyric)
{
  return current_lyric->prev;
}

int ol_lrc_parser_get_lyric_time(LrcInfo *current_lyric)
{
  return current_lyric->lyric_time;
}
char *ol_lrc_parser_get_lyric_text(LrcInfo *current_lyric)
{
  return current_lyric->lyric_text;
}
int ol_lrc_parser_get_lyric_id(LrcInfo *current_lyric)
{
  return current_lyric->lyric_id;
}
LrcInfo *ol_lrc_parser_get_lyric_by_id(LrcQueue *list,int lyric_id)
{
  LrcInfo *temp = &list->list[list->first];
  while(temp!=NULL)
  {
    if(temp->lyric_id==lyric_id)
    {
      return temp;
    }
    temp=temp->next;
  }
  return NULL;
}

char *ol_lrc_parser_load_lyric_source(char *lyric_source)
{
  FILE *fp;
  if((fp=fopen(lyric_source,"r"))==NULL)
  {
    printf("cannot open file!\n");
  }
  fseek( fp, 0, SEEK_END);
  int file_length = ftell(fp);
  char *lyric_file = malloc ((file_length + 1) * sizeof (char));
  fseek (fp, 0, SEEK_SET);
  memset(lyric_file,0,file_length+1);
  if (fread(lyric_file,sizeof(char),file_length,fp) != file_length)
  {
    /* TODO: error occurs when reading file */
  }
  return lyric_file;
}

void ol_lrc_parser_insert_list(LrcQueue *list, int lyric_time, char lyric_text[],int lyric_id)
{
  int InsertPos = -1;
  int i;
  LrcInfo *lp = &list->list[list->last];
  if(list->length == 0)
  {
    list->list[0].lyric_time = lyric_time;
    strcpy(list->list[0].lyric_text, lyric_text);
    list->list[0].lyric_id = lyric_id;
    list->list[0].prev = NULL;
    list->list[0].next = NULL;
    list->first = 0;
    list->last = 0;
    list->length = 1;
    InsertPos = 0;
  }
  else
  {
    for(i = list->length; i != 0; i--)
    {
      if(lp->lyric_time <= lyric_time)
      {
        if(i == list->length)
        {
          list->last = list->length;
          lp->next = &list->list[list->length];
          list->list[list->length].prev = lp;
          list->list[list->length].next = NULL;
        }
        else
        {
          LrcInfo *temp = lp->next;
          lp->next = &list->list[list->length];
          list->list[list->length].prev = lp;
          list->list[list->length].next = temp;
          temp->prev = &list->list[list->length];
        }
        list->list[list->length].lyric_time = lyric_time;
        strcpy(list->list[list->length].lyric_text, lyric_text);
        list->list[list->length].lyric_id = lyric_id;
        InsertPos = (list->length = list->length + 1);
        break;
      }
      else if(i == 1)
      {
        list->first = list->length;
        lp->prev = &list->list[list->length];
        list->list[list->length].prev = (void*)0;
        list->list[list->length].next = lp;
        list->list[list->length].lyric_time = lyric_time;
        strcpy(list->list[list->length].lyric_text, lyric_text);
        list->list[list->length].lyric_id = lyric_id;
        InsertPos = (list->length = list->length + 1);
      }
      lp = lp->prev;
    }
  }
}

LrcQueue* ol_lrc_parser_get_lyric_info(char *lyric_source)
{
  char *lyric_file = ol_lrc_parser_load_lyric_source(lyric_source);
  LrcQueue *list = malloc (sizeof(LrcQueue));
  memset(list,0,sizeof(LrcQueue));
  int file_size = strlen(lyric_file);
  
  int offset_time = 0;
  int current_offset = 0;
  int current_time = 0;
  int temp_offset,temp_offset1;
  int flag = 0;
  int sign=0;
  memset(list, 0, sizeof(*list));
  do {
    
    if(lyric_file[current_offset++] == '[')
    {
      if(flag == 0)
        if(strncmp(&lyric_file[current_offset], "offset:", 7) == 0)
        {
          current_offset += 7;
          if(lyric_file[current_offset] == '-')
          {
            flag = -1;
            current_offset++;
          }
          else
            flag = 1;
          while(lyric_file[current_offset] != ']')
          {
            offset_time *= 10;
            offset_time += lyric_file[current_offset++] - '0';
          }
          offset_time = offset_time * flag;
        }
      if(isnumeric(lyric_file[current_offset]))
      {
        char *lyric_text;
        flag = 1;
        temp_offset1 = current_offset;
        while(lyric_file[temp_offset1]!=']')
        {
          temp_offset1++;
        }
        char *tmptimeptr = lyric_file + temp_offset1;
        double timepar[3] = {0};
        int i = 0;
        do{
          while (tmptimeptr != lyric_file + current_offset && *tmptimeptr != ':')
            tmptimeptr--;
          if (*tmptimeptr == ':')
            tmptimeptr++;
          sscanf (tmptimeptr, "%lf", &timepar[i]);
          tmptimeptr-=2;
          i++;
        } while (tmptimeptr > lyric_file + current_offset && i >= 0);

        current_time = ((timepar[2]*60+timepar[1])*60+timepar[0])*1000;
                
        while(lyric_file[current_offset]!=']')
        {
          current_offset++;
        }
        current_offset++;
        temp_offset = current_offset;
        /* gets the position of the begin of the lyric*/
        /* FIXME: consider that there is a [ in the lyric but not ] */
        while((lyric_file[temp_offset] == '[') && isnumeric(lyric_file[temp_offset + 1]))
        {
          while((lyric_file[temp_offset] != ']') && (temp_offset < file_size))
            temp_offset++;
          temp_offset++;
        }
        lyric_text = &lyric_file[temp_offset];
        while((lyric_file[temp_offset] != 0x0d) && (lyric_file[temp_offset] != 0x0a) && (temp_offset < file_size))
          temp_offset++;
        lyric_file[temp_offset] = '\0';
        ol_lrc_parser_insert_list(list, current_time - offset_time, lyric_text,sign++);
      }
    }
  } while(current_offset < file_size);
  return list;
}
