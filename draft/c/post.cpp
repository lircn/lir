#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

int post_data(const char *filename, const char *usr)
{
	static char url[] = "http://ai.nationz.com.cn/v/dev_push";
	char args[200] = {0};
	CURL *curl = NULL;
	int ret = 0;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	FILE *fp = NULL;

	curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "file",
			CURLFORM_FILE, filename,
			CURLFORM_END);

	curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "filename",
			CURLFORM_COPYCONTENTS, "img",
			CURLFORM_END);

	fp = fopen("./ret", "w");
	curl = curl_easy_init();
	if (curl) {
		sprintf(args, "%s?usr=%s&type=%s", url, usr, "pic");
		curl_easy_setopt(curl, CURLOPT_URL, args);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	else {
		ret = -1;
	}

	curl_formfree(formpost);
	if (fp) {
		fclose(fp);
	}
	return ret;
}

int main(int argc, const char *argv[])
{
	post_data(argv[1], "bob");
	return 0;
}
