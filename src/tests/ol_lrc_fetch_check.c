#include "ol_lrc_fetch.h"
#include<unistd.h>  /* getopt */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define OPTSTR "+t:s:h"
const char *charset = "UTF-8";

void
usage(void)
{
	printf("LRC DOWNLOADER\n");
	printf("usage: lrc_download -t title -s singer\n");
	printf("usage: lrc_download -h\n");
}

int main(int argc, char *argv[])
{
	int ch, i, size, opt;
	char *title = NULL;
	char *singer = NULL;
	struct lrc_tsu *ret;

	if(argc <= 1) {
		usage();
		exit(1);
	}

	opterr = 0;
	while((ch = getopt(argc, argv, OPTSTR)) != -1) {
		switch(ch) {
			case 't':
				title = optarg;
				break;
			case 's':
				singer = optarg;
				break;
			case 'h':
				usage();
				exit(0);
			case '?':
				fprintf(stderr, "unknown option: -%c\n", optopt);
				usage();
				exit(1);
		}
	}

	if(title==NULL && singer==NULL) {
		fprintf(stderr, "at least one of title and singer need to be NOT NULL\n");
		exit(1);
	}

	ret = sogou.lrc_search(title, singer, &size);
	for(i=0; i<size; i++) {
		printf("%d. \n", i+1);
		printf("title: %s\n", ret[i].title);
		printf("singer: %s\n", ret[i].singer);
		printf("\n");
	}

	printf("\nWhich one you want to download(1-%d): ", size);
	opt = fgetc(stdin);

	sogou.lrc_download(&ret[opt-'0'-1], NULL);

	printf("Download Success\n\n");
	return 0;
}

