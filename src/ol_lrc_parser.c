#include "ol_lrc_parser.h"
#include "ol_debug.h"
#define isnumeric(a) (((a >= '0') && (a <= '9')) ? 1 : 0)

const char *OFFSET_FORMAT = "[offset:%d]\n";

int ol_lrc_parser_get_offset_length(int offset);
LrcInfo *ol_lrc_parser_get_first_of_list(LrcQueue *list)
{
  return &list->list[list->first];
}

LrcInfo *ol_lrc_parser_get_last_of_list(LrcQueue *list)
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

char *ol_lrc_parser_load_lyric_source(const char *lyric_source)
{
  ol_debugf ("%s:%s\n", __FUNCTION__, lyric_source);
  FILE *fp;
  if((fp=fopen(lyric_source,"r"))==NULL)
  {
    ol_debugf ("  cannot open file!\n");
    char *lyric_file = malloc (sizeof (char));
    lyric_file[0] = '\0';
    return lyric_file;
  }
  fseek( fp, 0, SEEK_END);
  struct stat buf;
  if (lstat(lyric_source, &buf) < 0)
  {
    ol_debugf ("cannot open file!\n");
  }
  int file_length = buf.st_size;
  // int file_length = ftell(fp);
  char *lyric_file = malloc ((file_length + 1) * sizeof (char));
  fseek (fp, 0, SEEK_SET);
  memset(lyric_file,0,file_length+1);
  if (fread(lyric_file,sizeof(char),file_length,fp) != file_length)
  {
    /* TODO: error occurs when reading file */
  }
  lyric_file[file_length] = '\0';
  return lyric_file;
}

void ol_lrc_parser_insert_list(LrcQueue *list, int lyric_time, char lyric_text[])
{
  ol_assert (list != NULL);
  int InsertPos = -1;
  int i;
  LrcInfo *lp = &list->list[list->last];
  if(list->length == 0)
  {
    list->list[0].lyric_time = lyric_time;
    strcpy(list->list[0].lyric_text, lyric_text);
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
        InsertPos = (list->length = list->length + 1);
      }
      lp = lp->prev;
    }
  }
}

static int ol_lrc_parser_get_field_end (char *lyric_file,
                                        int offset)
{
  if (lyric_file == NULL)
    return offset;
  while(lyric_file[offset]!=']' &&
        lyric_file[offset] != '\0' &&
        lyric_file[offset] != '\n' &&
        lyric_file[offset] != '\r')
  {
    offset++;
  }
  return offset;
}

static int ol_lrc_parser_parse_offset (LrcQueue *list,
                                       char *lyric_file,
                                       int *current_offset,
                                       int *offset_time)
{
  int flag = 0;
  *offset_time = 0;
  if(strncmp(&lyric_file[(*current_offset) + 1], "offset:", 7) == 0)
  {
    *current_offset += 8;
    if(lyric_file[*current_offset] == '-')
    {
      flag = -1;
      (*current_offset)++;
    }
    else
    {
      flag = 1;
      if (lyric_file[*current_offset] == '+')
        (*current_offset)++;
    }
    while(lyric_file[*current_offset] != ']' && lyric_file[*current_offset] != '\0')
    {
      *offset_time *= 10;
      *offset_time += lyric_file[(*current_offset)++] - '0';
    }
    *offset_time = *offset_time * flag;
    ol_debugf ("  offsettime:%d\n", *offset_time);
  }
  return flag;
}

static int ol_lrc_parser_get_field_time (char *lyric_file,
                                         int offset)
{
  if (lyric_file[offset] != '[')
    return -1;
  offset++;
  int temp_offset1 = ol_lrc_parser_get_field_end (lyric_file,
                                                  offset);
  if (lyric_file[temp_offset1] != ']')
    return -1;
  lyric_file[temp_offset1] = 0;
  /* ol_debugf ("  field time: %s\n", lyric_file + offset); */
  char *tmptimeptr = lyric_file + temp_offset1;
  double timepar[3] = {0};
  int i = 0;
  do{
    while (tmptimeptr != lyric_file + offset && *tmptimeptr != ':')
      tmptimeptr--;
    if (*tmptimeptr == ':')
      tmptimeptr++;
    sscanf (tmptimeptr, "%lf", &timepar[i]);
    tmptimeptr-=2;
    i++;
  } while (tmptimeptr > lyric_file + offset && i >= 0);

  return ((timepar[2]*60+timepar[1])*60+timepar[0])*1000;
}

static int ol_lrc_parser_get_lyric_beign_offset (char *lyric_file,
                                                 int offset)
{
  /* gets the position of the begin of the lyric*/
  /* FIXME: consider that there is a [ in the lyric but not ] */
  while((lyric_file[offset] == '[') &&
        isnumeric(lyric_file[offset + 1]))
  {
    while((lyric_file[offset] != ']') && (lyric_file[offset] != 0))
      offset++;
    if (lyric_file[offset] != '\0')
      offset++;
  }
  return offset;
}

static int ol_lrc_parser_get_line_end_offset (char *str)
{
  if (str == 0)
    return 0;
  int offset = 0;
  while(str[offset] != '\r' &&
        str[offset] != '\n' &&
        str[offset] != '\0')
    offset++;
  return offset;
}

LrcQueue* ol_lrc_parser_get_lyric_info(const char *lyric_source)
{
  ol_log_func ();
  char *lyric_file = ol_lrc_parser_load_lyric_source(lyric_source);
  LrcQueue *list = malloc (sizeof(LrcQueue));
  
  memset(list, 0, sizeof(*list));
  int file_size = strlen(lyric_file);
  size_t filename_len = strlen (lyric_source);
  list->filename = malloc (filename_len + 1);
  strcpy (list->filename, lyric_source);
  int offset_time = 0;
  int current_offset = 0;
  int offset_flag = 0;
  //ol_debugf ("  file_size: %d\n", file_size);
  do {
    /* ol_debugf ("  current_offset:%d\n", current_offset); */
    if(lyric_file[current_offset] == '[')
    {
      if(offset_flag == 0)
      {
        offset_flag = ol_lrc_parser_parse_offset (list,
                                           lyric_file,
                                           &current_offset,
                                           &offset_time);
        if (offset_flag != 0)
          continue;
      }
      if(isnumeric(lyric_file[current_offset + 1]))
      {
        int lyric_offset = ol_lrc_parser_get_lyric_beign_offset (lyric_file,
                                                            current_offset);
        char *lyric_text = &lyric_file[lyric_offset];
        int end_offset = ol_lrc_parser_get_line_end_offset (lyric_text);
        lyric_text[end_offset] = '\0';
        /* ol_debugf ("  lyric_text: %s\n", lyric_text); */
        offset_flag = 1;
        int current_time = 0;
        while ((current_time = ol_lrc_parser_get_field_time (lyric_file, current_offset)) != -1)
        {
          /* ol_debugf ("  current time: %d\n", current_time); */
          current_offset = ol_lrc_parser_get_field_end (lyric_file,
                                                        current_offset);
          current_offset++;
          ol_lrc_parser_insert_list(list, current_time - offset_time, lyric_text);
        }
        current_offset = lyric_offset + end_offset;
      }
      else
      {
        current_offset += ol_lrc_parser_get_line_end_offset (lyric_file + current_offset);
      }
    } /* if(lyric_file[current_offset++] == '[') */
    else
    {
      current_offset++;
    }
  } while(current_offset < file_size);

  LrcInfo *temp = &list->list[list->first];
  int id = 0;
  while(temp!=NULL)
  {
    temp->lyric_id = id;
    id++;
    temp=temp->next;
  }
  list->offset = offset_time;
  
  return list;
}
void ol_lrc_parser_set_lyric_file_offset (const char *lyric_source,
                                          int offset)
{
 /*refresh the lyric_source*/
  char *lyric_file = ol_lrc_parser_load_lyric_source(lyric_source);
  int file_length = strlen (lyric_file);
  int offset_length = ol_lrc_parser_get_offset_length(offset);

  ol_debugf ("offset_length:%d\n", offset_length);

  int before_length, after_length;
  int before_offset,after_offset;
  int current_offset = 0;
  int sign = 0;
  
  /*find the offsets*/
  do {
    if (lyric_file[current_offset++] == '[')
    {
      if (strncmp (&lyric_file[current_offset], "offset:", 7) == 0)
      {
        before_offset = current_offset-1;
        while (current_offset < file_length)
        {
          ol_debugf ("offset: %d: %c\n", current_offset, lyric_file[current_offset]);
          if (lyric_file[current_offset++] == ']')
          {
            if (current_offset < file_length &&
                (lyric_file[current_offset] == '\r'))
              current_offset++;
            if (current_offset < file_length &&
                (lyric_file[current_offset] == '\n'))
              current_offset++;
            after_offset = current_offset;
            sign = 1;
            break;
          }
        }
      }
      else
      {
        if (isnumeric (lyric_file[current_offset]))
        {
          ol_debug ("asdf");
          before_offset = after_offset = current_offset-1;
          sign = 2;
        }
      }
    }
  }while(sign == 0 && current_offset < file_length);

  before_length = before_offset;
  after_length = file_length - after_offset;
  ol_debugf ("after_offset: %d, after_length: %d, file_len: %d\n",
             after_offset, after_length, file_length);
  file_length = before_length + after_length +offset_length;
  char *lyric_file_change = malloc ((file_length) * sizeof (char));
  char *before_offset_lyric = malloc ((before_length + 1) * sizeof (char));
  char *after_offset_lyric = malloc ((after_length + 1) * sizeof(char));
  char *offset_lyric = malloc ((offset_length+1)* sizeof(char));
  /* ol_debugf ("offset_length:%d\t before_length:%d\t after_length:%d\t file_length:%d\n", */
  /*            offset_length, before_length, after_length, file_length); */

  strncpy (before_offset_lyric, lyric_file, before_length);
  before_offset_lyric[before_length] = '\0';
  /* ol_debugf ("before_offset_lyric:%s\n", before_offset_lyric); */
  /* ol_debugf ("before_length:%d\n", strlen(before_offset_lyric)); */

  strncpy (after_offset_lyric, &lyric_file[after_offset], after_length);
  after_offset_lyric[after_length] = '\0';
  ol_debugf ("after_offset:%d\n", after_offset);
  ol_debugf ("after_offset_lyric:%s\n", after_offset_lyric);
  ol_debugf ("after_length:%d\n", strlen(after_offset_lyric));

  snprintf(offset_lyric, offset_length+1, OFFSET_FORMAT, offset);
  /* ol_debugf ("offset_lyric:%s\n", offset_lyric); */
  /* ol_debugf ("offset_length:%d\n", strlen(offset_lyric)); */

  strncpy (lyric_file_change, before_offset_lyric,before_length);
  strncpy (&lyric_file_change[before_offset], offset_lyric, offset_length);
  strncpy (&lyric_file_change[before_offset+offset_length], after_offset_lyric, after_length);
  /* ol_debugf ("length:%d\n", strlen(lyric_file_change)); */
  /* ol_debugf ("offset_file_change:%s\n", lyric_file_change); */
  /* ol_debugf ("offset_length:%d\n",file_length); */


  /*save the lyric_file_change to the file stream */
  FILE *fp;
  if((fp=fopen(lyric_source,"w"))==NULL)
  {
    printf("cannot open file!\n");
  }
  else
  {
    fwrite(lyric_file_change,sizeof(char), file_length,fp);
    fclose(fp);
  }
  
}

int ol_lrc_parser_get_offset_length(int offset)
{
  char offset_str[100];
  sprintf(offset_str, OFFSET_FORMAT, offset);
  return strlen (offset_str);
}

void ol_lrc_parser_set_lyric_offset(LrcQueue *list, int offset)
{
  int i = 0;
  int old_offset = list->offset;
  for (;i<list->length;i++)
  {
    list->list[i].lyric_time = list->list[i].lyric_time + old_offset-offset;
  }
  list->offset = offset;
}
int ol_lrc_parser_get_lyric_offset(LrcQueue *list)
{
  return list->offset;
}

void ol_lrc_parser_free (LrcQueue *list)
{
  ol_assert (list != NULL);
  if (list->filename != NULL)
    free (list->filename);
  free (list);
}

const char *ol_lrc_parser_get_filename (LrcQueue *list)
{
  ol_assert_ret (list != NULL, NULL);
  ol_debugf ("filename %s\n", list->filename);
  return list->filename;
}
