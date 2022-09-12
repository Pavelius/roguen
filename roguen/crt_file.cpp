#include "crt.h"
#include "io_stream.h"

const char*	szfurl(const char* url) {
	char temp[2048];
	if(!io::file::getfullurl(url, temp, sizeof(temp)))
		return "";
	szlower(temp); szurlc(temp);
	return szdup(temp);
}