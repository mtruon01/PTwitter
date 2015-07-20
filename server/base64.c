#include "base64.h"
#include <string.h>

char *base64_encode(const char *data)
{
	/* Conversion table.  */
	static const char tbl[64] = {
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
		'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
		'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
		'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
	};

	/* Access bytes in DATA as unsigned char, otherwise the shifts below
		 don't work for data with MSB set. */
	const unsigned char *s = (const unsigned char *)data;
	size_t length = strlen(data);
	size_t i, il;
	char *p, *p_cpy;

	/* allocate memory for the result. */
	int left_over = length % 3;
	size_t result_size = length / 3 * 4 + 1;
	if(left_over)result_size += 4;
	p = (char *)malloc(result_size);
	if(NULL == p)return NULL; // out of memory?
	p_cpy = p;
	
	/* Transform the 3x8 bits to 4x6 bits, as required by base64.  */
	for(i=0, il=length-left_over; i<il; i+=3, p+=4)
	{
		p[0] = tbl[s[i] >> 2];
		p[1] = tbl[((s[i] & 3) << 4) + (s[i + 1] >> 4)];
		p[2] = tbl[((s[i + 1] & 0xf) << 2) + (s[i + 2] >> 6)];
		p[3] = tbl[s[i + 2] & 0x3f];
	}

	/* Pad the result if necessary...  */
	switch(left_over)
	{
	case 1:
		p[0] = tbl[s[i] >> 2];
		p[1] = tbl[(s[i] & 3) << 4];
		p[2] = '=';
		p[3] = '=';
		p += 4;
		break;
	case 2:
		p[0] = tbl[s[i] >> 2];
		p[1] = tbl[((s[i] & 3) << 4) + (s[i + 1] >> 4)];
		p[2] = tbl[((s[i + 1] & 0xf) << 2)];
		p[3] = '=';
		p += 4;
		break;
	}
	
	/* ...and zero-terminate it.  */
	*p = '\0';

	return p_cpy;
}



char *base64_decode(const char *base64)
{
	/* Table of base64 values for first 128 characters.  Note that this
		 assumes ASCII (but so does Wget in other places).  */
	static const signed char base64_char_to_value[128] =
	{
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  /*   0-  9 */
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  /*  10- 19 */
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  /*  20- 29 */
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  /*  30- 39 */
		-1,  -1,  -1,  62,  -1,  -1,  -1,  63,  52,  53,  /*  40- 49 */
		54,  55,  56,  57,  58,  59,  60,  61,  -1,  -1,  /*  50- 59 */
		-1,   0,  -1,  -1,  -1,   0,   1,   2,   3,   4,  /*  60- 69 */
		5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  /*  70- 79 */
		15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  /*  80- 89 */
		25,  -1,  -1,  -1,  -1,  -1,  -1,  26,  27,  28,  /*  90- 99 */
		29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  /* 100-109 */
		39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  /* 110-119 */
		49,  50,  51,  -1,  -1,  -1,  -1,  -1             /* 120-127 */
	};
	/** could also use inline function rather than MACRO. */
	#define IS_ASCII(c) (((c) & 0x80) == 0)
	#define IS_BASE64(c) ((IS_ASCII(c) && base64_char_to_value[c] >= 0) || c == '=')

	size_t length = strlen(base64);
	size_t i;
	char *p, *p_cpy;

	if(length % 4)return NULL; // must be multiple of 4
	
	/* allocate memory for the result. */
	p = (char *)malloc(length / 4 * 3 + 1);
	if(NULL == p)return NULL; // out of memory?
	p_cpy = p;
	
	/* Transform the 4x6 bits to 3x8 bits.  */
	for(i=0; i<length; i+=4, p+=3)
	{
		int quad = 0;
		if(!IS_BASE64(base64[i    ]) || !IS_BASE64(base64[i + 1]) ||
		   !IS_BASE64(base64[i + 2]) || !IS_BASE64(base64[i + 3]))return NULL;
		   
		   
		// first 8 bits, obtain from 1st & 2nd base64 chars
		quad  = (base64_char_to_value[base64[i    ]] << 18) |
		        (base64_char_to_value[base64[i + 1]] << 12);
		p[0] =  quad >> 16;
		
		// second 8 bits, obtain from 2nd & 3rd base64 chars
		quad |= (base64_char_to_value[base64[i + 2]] <<  6);
		p[1] =  0xff  & (quad >> 8);
		
		// third 8 bits, obtain from 3rd & 4th base64 chars
		quad |= (base64_char_to_value[base64[i + 3]]      );
		p[2] =  0xff & quad; // 3rd 8 bits
	}
	
	*p = '\0';
	
	#undef BASE64_CHAR_TO_VALUE
	#undef IS_ASCII
	
	return p_cpy;
}

#ifdef __cplusplus
std::string base64_encode_cpp(const std::string &data)
{
	char *b64 = base64_encode(data.c_str());
	std::string r;
	if(NULL != b64)r = b64;
	free(b64);
	return r;
}

std::string base64_decode_cpp(const std::string &base64)
{
	char *data = base64_decode(base64.c_str());
	std::string r;
	if(NULL != data)r = data;
	free(data);
	return r;
}
#endif // __cplusplus


#ifdef STANDALONE

#include <stdio.h>

static void report_err(const char *action, const char *in, const char *out, const char *exp)
{
	printf("FAILED %s \"%s\"\nExpect: %s\nGot:%s\n", action, in, exp, out);
}


int main(int argc, char **argv)
{
#ifdef __cplusplus
	std::string outpp;
#else
	char *out;
#endif

	const char *test_cases[][2] = {
		{"", ""},
		{"leasure.",	"bGVhc3VyZS4="},
		{"easure.",	"ZWFzdXJlLg=="},
		{"asure.",	"YXN1cmUu"},
		{"sure.",	"c3VyZS4="},
		{"Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.",
		 "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4="}
	};
	
	int i;
	for(i=0; i<sizeof(test_cases)/sizeof(char *)/2; ++i)
	{
#ifdef __cplusplus
		outpp = base64_encode_cpp(test_cases[i][0]);
		if(0 != strcmp(outpp.c_str(), test_cases[i][1]))
		{
			report_err("encoding", test_cases[i][0], outpp.c_str(), test_cases[i][1]);
		}
		
		outpp = base64_decode_cpp(test_cases[i][1]);
		if(0 != strcmp(outpp.c_str(), test_cases[i][0]))
		{
			report_err("decoding", test_cases[i][1], outpp.c_str(), test_cases[i][0]);
		}
#else 
		out = base64_encode(test_cases[i][0]);
		if(0 != strcmp(out, test_cases[i][1]))
		{
			report_err("encoding", test_cases[i][0], out, test_cases[i][1]);
		}
		free(out);
		
		out = base64_decode(test_cases[i][1]);
		if(0 != strcmp(out, test_cases[i][0]))
		{
			report_err("decoding", test_cases[i][1], out, test_cases[i][0]);
		}
		free(out);
#endif
	}
	
	
	while(1)
	{
		char in[1024];
		
		scanf("%s", (char *)in);
#ifdef __cplusplus
		outpp = base64_encode_cpp(in);
		printf("Encode: %s\nBase64: %s\n", in, outpp.c_str());
		outpp = base64_decode_cpp(in);
		printf("Decode: %s\nBase64: %s\n", in, outpp.c_str());
#else
		out = base64_encode(in);
		printf("Encode: %s\nBase64: %s\n", in, out);
		free(out);
		out = base64_decode(in);
		printf("Decode: %s\nBase64: %s\n", in, out);
		free(out);
#endif
	}
}

#endif // STANDALONE


