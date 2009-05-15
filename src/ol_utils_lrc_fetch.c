#include "ol_utils_lrc_fetch.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<errno.h>
#include<unistd.h>

static long cntimeout = 6;
static char errbuf[CURL_ERROR_SIZE];

size_t
convert_icv(iconv_t *icv, char *src, size_t srclen, char *dest, size_t destlen)
{
	size_t ret;
	if(icv == NULL)
		return (size_t)-1;

	char **input = &src;
	char **output = &dest;
	memset(dest, 0, destlen);

	/*
	if(_LIBICONV_VERSION >= 0x0108) {
    if(iconvctl(icv, ICONV_SET_TRANSLITERATE, (void *)1) < 0) {
		fprintf(stderr, "can't enable transliteration in the conversion.\n");
		return (size_t)-1;
	}

	if(iconvctl(icv, ICONV_SET_DISCARD_ILSEQ, (void *)1) < 0) {
		fprintf(stderr, "can't enable illegal sequence discard and continue in the conversion.\n");
		return (size_t)-1;
	}
	}
    */
	ret = iconv(*icv, input, &srclen, output, &destlen);
	return ret;
}

size_t
convert(const char *from_charset, const char *to_charset, char *src, size_t srclen, char *dest, size_t destlen)
{
	size_t ret;
	iconv_t cv;
	char **input = &src;
	char **output = &dest;
	memset(dest, 0, destlen);

	if((cv = iconv_open(to_charset, from_charset)) == (iconv_t)-1) {
        fprintf(stderr, "the conversion from %s to %s is not supported by the implementation.\n", from_charset, to_charset);
		return (size_t)-1;
	}

	/*
	if(_LIBICONV_VERSION >= 0x0108) {
	if(iconvctl(cv, ICONV_SET_TRANSLITERATE, (void *)1) < 0) {
		fprintf(stderr, "can't enable transliteration in the conversion.\n");
		return (size_t)-1;
	}

	if(iconvctl(cv, ICONV_SET_DISCARD_ILSEQ, (void *)1) < 0) {
		fprintf(stderr, "can't enable illegal sequence discard and continue in the conversion.\n");
		return (size_t)-1;
	}
	}
	*/

	ret = iconv(cv, input, &srclen, output, &destlen);
	iconv_close(cv);
	return ret;
}

static CURL *
my_curl_init(CURL *curl, char *url, WriteCallback func, void *data, long connecttimeout)
{
	CURLcode code;
	CURL *curl_handler;
	if(curl_global_init(CURL_GLOBAL_ALL) != 0) {
		fprintf(stderr, "curl_global_init error.\n");
		return NULL;
	}

	if(curl != NULL) {
		curl_handler = curl;
		/*
		 * reset all options of a libcurl session handle
		 * this puts back the handle to the same state as it was in when it was just created with curl_easy_init
		 */
		curl_easy_reset(curl_handler);   
	}
	else
		curl_handler = curl_easy_init();

	if(curl_handler == NULL) {
		fprintf(stderr, "failed to create CURL easy session handler\n");
		return NULL;
	}

	code = curl_easy_setopt(curl_handler, CURLOPT_ERRORBUFFER, errbuf);
	if(code != CURLE_OK) {
		fprintf(stderr, "failed to set error buffer [%d]\n", code);
		return NULL;
	}
	code = curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	if(code != CURLE_OK) {
		fprintf(stderr, "failed to set URL [%s]\n", errbuf);
		return NULL;
	}
	if(func != NULL) {
		code = curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, func);
		if(code != CURLE_OK) {
			fprintf(stderr, "failed to set writefunction [%s]\n", errbuf);
			return NULL;
		}
	}
	if(data != NULL) {
		code = curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, data);
		if(code != CURLE_OK) {
			fprintf(stderr, "failed to set writedata [%s]\n", errbuf);
			return NULL;
		}
	}
	/*
	 * Pass a long. 
	 * It should contain the maximum time in seconds that you allow the connection to the server to take. 
	 * This only limits the connection phase, once it has connected, this option is of no more use. 
	 * Set to zero to disable connection timeout (it will then only timeout on the system's internal timeouts).
	 */
    code = curl_easy_setopt(curl_handler, CURLOPT_CONNECTTIMEOUT, connecttimeout);
	if(code != CURLE_OK) {
		fprintf(stderr, "failed to set connecttimeout [%s]\n", errbuf);
		return NULL;
	}

	return curl_handler;
}

int
fetch_into_file(char *url, FILE *fp)
{
	CURL *curl;
	CURLcode code;

	curl = my_curl_init(NULL, url, NULL, fp, cntimeout);
	if(curl == NULL)
		return -1;

	code = curl_easy_perform(curl);
	if(code != CURLE_OK) {
		fprintf(stderr, "failed to perform: [%s]\n", errbuf);
		return -1;
	}
	curl_easy_cleanup(curl);
	return 0;
}

static void *
myrealloc(void *ptr, size_t size)
{
	if(ptr)
		return realloc(ptr, size);
	else
		return malloc(size);
}

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	struct memo *memstr = (struct memo *)data;
	size_t need = size * nmemb;

	memstr->mem_base = myrealloc(memstr->mem_base, memstr->mem_len+need);
	if(memstr->mem_base) {
		memcpy(&memstr->mem_base[memstr->mem_len], ptr, need);
		memstr->mem_len += need;
	} else
		return 0;
	return need;
}

int
fetch_into_memory(char *url, struct memo *dest)
{
	CURL *curl;
	CURLcode code;
	WriteCallback callback = WriteMemoryCallback;

	curl = my_curl_init(NULL, url, callback, dest, cntimeout);
	if(curl == NULL)
		return -1;

	code = curl_easy_perform(curl);
	if(code != CURLE_OK) {
		fprintf(stderr, "failed to perform: [%s]\n", errbuf);
		return -1;
	}
	curl_easy_cleanup(curl);
	return 0;
}

/*
 * HTML 格式编码
 * 对 String 编码时，使用以下规则：
    * 字母数字字符 "a" 到 "z"、"A" 到 "Z" 和 "0" 到 "9" 保持不变。
    * 特殊字符 "."、"-"、"*" 和 "_" 保持不变。
    * 空格字符 " " 转换为一个加号 "+"。
    * 所有其他字符都是不安全的，因此首先使用一些编码机制将它们转换为一个或多个字节。
	* 然后每个字节用一个包含 3 个字符的字符串 "%xy" 表示，其中 xy 为该字节的两位十六进制表示形式。
	* 推荐的编码机制是 UTF-8。但是，出于兼容性考虑，如果未指定一种编码，则使用相应平台的默认编码。 
 */
int
url_encoding(const char *src, const int srclen, char *dest, int destlen)
{
	int i;
	int j = 0; 
	char ch;

	/*
	 * convert to GBK, this should be done before this function called
	 *
    char buf[BUFSZ];	
	char *src_copy;
	iconv_t icv;
	if(charset != NULL)
		icv = iconv_open("GBK", charset);
	else
		icv = iconv_open("GBK", "UTF-8");
	
	if((src_copy = malloc(srclen+1)) == NULL)
		return -1;
	memcpy(src_copy, src, srclen);
	src_copy[srclen] = 0;
	convert_icv(&icv, src_copy, srclen, buf, BUFSZ);
	iconv_close(icv);
    free(src_copy);
    */
    
	if(src == NULL || dest == NULL || srclen < 0 || destlen < 0)
		return -1;

	for(i=0; ((i<srclen) && j<destlen); i++) {
		ch = src[i];
		if((ch>='A') && (ch<='Z'))
			dest[j++] = ch;
		else if((ch>='a') && (ch<='z'))
			dest[j++] = ch;
		else if((ch>='0') && (ch<='9'))
			dest[j++] = ch;
		else if(ch=='.' || ch=='*' || ch=='_' || ch=='-')
			dest[j++] = ch;
		else if(ch==' ')
			dest[j++] = '+';
		else {
			if(j+3 < destlen) {
				sprintf(dest+j, "%%%02X", (unsigned char)ch);
				j += 3;
			} else
				return -1;
		}
	}
	dest[j] = '\0';
	return 0;
}

static int 
chartonum(char ch)
{
	if(ch>='0' && ch<='9')
		return (char)(ch - '0');
	if(ch>='a' && ch<='z')
		return (char)(ch - 'a' + 10);
	if(ch>='A' && ch<='F')
		return (char)(ch - 'A' + 10);
	return -1;
}

int
url_decoding(const char *src, const int srclen, char *dest, int destlen)
{
	char ch;
	int idx1, idx2;
	int i;
	int j = 0; 

	if(src == NULL || dest == NULL || srclen < 0 || destlen < 0)
		return -1;

	for(i=0; (i<srclen && j<destlen); i++) {
		ch = src[i];
		switch(ch) {
			case '+' :
				dest[j++] = ' ';
				break;
			case '%':
				idx1 = chartonum(src[i+1]);
				idx2 = chartonum(src[i+2]);
				dest[j++] = (char)((idx1<<4) | idx2);
				i += 2;
				break;
			default:
				dest[j++] = ch;
		}
	}
    dest[j] = '\0';
	/*
	 * convert to target charset, this is be done after this function called
	 convert("GBK", charset==NULL ? "UTF-8" : charset, buf, strlen(buf), dest, destlen);
	*/
	return 0;
}

int
curl_url_encoding(CURL *curl, char *input, char *output, size_t size)
{
	char *escp;
	int flag = 0;
	if(curl == NULL) {
		curl = curl_easy_init();
		flag = 1;
	}

	/* 
	 * convert to GBK, this should be done before this function called
	 *
    char buf[BUFSZ];
	iconv_t icv;
	if(charset != NULL)
		icv = iconv_open("GBK", charset);
	else
		icv = iconv_open("GBK", "UTF-8");
	
	convert_icv(&icv, input, strlen(input), buf, BUFSZ);
	iconv_close(icv);
    */

	escp = curl_easy_escape(curl, input, 0);
	if(escp == NULL) {
		fprintf(stderr, "curl_easy_escape error.\n");
		return -1;
	}
	if(strlen(escp) > size) {
		errno = E2BIG; /* identify that buffer storing the result is too small */
		return -1;
	}
	strcpy(output, escp);
	curl_free(escp);

	if(flag == 1)
		curl_easy_cleanup(curl);
	return 0;
}

int
curl_url_decoding(CURL *curl, char *input, char *output, size_t size)
{
	char *unescp;
	int flag = 0;
	if(curl == NULL) {
		curl = curl_easy_init();
		flag = 1;
	}

	unescp = curl_easy_unescape(curl, input, 0, NULL);
	if(unescp == NULL) {
		fprintf(stderr, "curl_easy_unescape error.\n");
		return -1;
	}
	if(strlen(unescp) > size) {
		errno = E2BIG; /* identify that buffer storing the result is too small */
		return -1;
	}
	strcpy(output, unescp); 

	/* 
	 * convert to target charset, this is be done after this function called
	convert("GBK", charset==NULL ? "UTF-8" : charset, unescp, strlen(unescp), output, size);
	*/

	curl_free(unescp);
	if(flag == 1)
		curl_easy_cleanup(curl);
	return 0;
}

int 
ignore_case_strcmp(const char *str1, const char *str2, const size_t count)
{
	const char *ptr1 = str1;
	const char *ptr2 = str2;
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	int min = len1 > len2 ? len2 : len1;
	min = min > count ? count : min;

	while((ptr1 < str1+min) && (ptr2 < str2+min)) {
		if(isalpha(*ptr1) && isalpha(*ptr2)) {
			if(tolower(*ptr1) != tolower(*ptr2))
				return *ptr1 > *ptr2 ? 1 : -1;
		} else {
			if(*ptr1 != *ptr2)
				return *ptr1 > *ptr2 ? 1 : -1;
		}
		ptr1++;
		ptr2++;
	}
	return 0;
}

#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif
#define PATH_MAX_GUESS 1024

char *
path_alloc(void)
{
	char *ptr;
	int size;

	if(pathmax == 0) {
		errno = 0;
		if((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
			if(errno == 0)
				pathmax = PATH_MAX_GUESS;
			else {
				fprintf(stderr, "pathconf error for _PC_PATH_MAX\n");
				return NULL;
			}
		} else 
			pathmax++;
	}

	if((ptr = calloc(pathmax, sizeof(char))) == NULL) {
		fprintf(stderr, "malloc error for pathname\n");
		return NULL;
	}

	return ptr;
}
