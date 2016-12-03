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
char check[] = "d5713c2ad297a7601135c22f51516bbb";
char finalStr[1000] ;
char res[100][100];

//struct for sent return
struct reArray{
	char* r;
	unsigned char pos ;
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
	pclose(fp);
	//printf("This is response %s \n",response);
	bool isOk = true;
	int i=0;
	for(i=0;i<15;i++){
		if(response[i] != right_c[i]){
			return 0;
		}
	}
	//printf("Fake cookie works: %s \n", myCookie);
	return 1;
}

// bigOfunc if pass padding check return 1; else return 0
bool bigOFunc(char * cmbStr){	//cmbStr is a itoa str
	//first combine it with IV; IV should be a itoa str
	char* finalCmbStr = (char*)malloc(strlen(cmbStr) + strlen(IV));
	strcpy(finalCmbStr, IV);
	strcat(finalCmbStr, cmbStr);
	char encodedFinalStr[1000];
	//printf("finalCmbStr's length is: %d \n", (int)(strlen(finalCmbStr)));
	//base64 encode
	char cmd[1000];
	sprintf(cmd, "echo   \"%s\"  | xxd -r -p |  base64", finalCmbStr );
	//printf("cmd: %s\n", cmd);
	FILE *fp;
	fp = popen(cmd, "r");
	while(fgets(encodedFinalStr, 1000, fp)!=NULL){
		//printf("EncodedFinalStr: %s \n", encodedFinalStr);
	}
	int status = pclose(fp);
	// now encodedFinalStr is base64 ed 
	// final agree pos
	unsigned char pos = 1;
	if(sentToWeb(encodedFinalStr)){
		return 1;
	}
	return 0;
}
struct reArray process(char* C,char* r, unsigned char pos){
	srand(time(NULL));
	int _i = 0, i=0;
	char _bug = (char)(rand()%256);
	char hexBaseStr[65];
	hexBaseStr[64] = '\0';
	r[cons_b-pos] = (char)(_bug ^ (_i));
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
	//printf("Final R is: %s length is: %d \n", r, strlen(r));
	//printf("HexStr is: %s length is: %d \n", hexBaseStr, strlen(hexBaseStr));
	while(!bigOFunc(hexBaseStr)){
		_i++;
		//if(_i >256) return NULL;
		r[cons_b-pos] = (char)(_bug ^ (_i));
		//printf("r lenght: is %d \n",(int)strlen(r));
		//printf("C lenght: is %d\n",(int)strlen(C));
		//printf("HexStr lenght: is %d\n",(int)strlen(hexBaseStr));
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
	for( i=0; i<cons_b-pos;i++){
		r[i] = sd_r[i] ^ 1;
		for(i_1=0;i_1<16;i_1++){
			sprintf(hexBaseStr+i_1*2, "%02x", (unsigned char)(r[i_1]));
		}
		for(i_1=0;i_1<16;i_1++){
			sprintf(hexBaseStr+32+i_1*2, "%02x", (unsigned char)(C[i_1]));
		}
		if(!bigOFunc(hexBaseStr)){
			//pos is 1
			re.r = sd_r;
			re.pos =(unsigned char) (cons_b - i);
			return re;
		}
	}
	re.r = sd_r;
	re.pos = pos;
	return re;
}
//decrypt a single word, return plaintext
struct reArray decryption_word(char* C){
	srand(time(NULL));
	// C is not itoa base
	char r[cons_b+1];
	r[cons_b] = '\0';
	int i=0;
	for(i=0;i<cons_b;i++){
		//r[i] = (char)(rand()%256);
		r[i] = '1';
	}
	return process(C, r, 1);
}
// pos is back to front pos
struct reArray decryption_next(char* C,char* re_r, unsigned pos){
	srand(time(NULL));
	// new random char*
	char r[cons_b+1];
	r[cons_b] = '\0';
	int i=0, j = cons_b-pos+1;
	for(i=j-1;i<cons_b;i++){
		r[i] = re_r[i];
	}
	for(i = 0;i <=j-2;i++){
		//r[i] = (char)(rand()%256);
		r[i] = '1';
	}
	return process(C, r, pos+1);
}
struct reArray decryption_block(char * C){
	struct reArray re = decryption_word(C);
	re.r[cons_b - re.pos] ^= (unsigned char)(re.pos);
	int i=0;
	for(i=re.pos;i<16;){
		printf("current pos is: %d\n", i);
		re = decryption_next(C, re.r, re.pos);
		re.r[cons_b - re.pos] ^= (unsigned char)(re.pos);
		i = re.pos;
	}
	re.r[0] ^= '\x10';
	return re;
}
//8hIOODzClpAhMw/0dAalkxUVFRUVFRUVFRW8vLy8vDHVcTwq0penYBE1wi9RUWu7
// check result
bool checkFunction(char *r){
	char tmp[200];
	int i=0;
	strcpy(tmp, r);
	strcat(tmp, check);
	if(bigOFunc(tmp)){
		printf("Pass Check!! \n");
			return 1;
	}else{
		printf("Exist Bug! \n");
		return 0;
	}
}
int main(int argc, char** argv){
	if(argc !=2){
		printf("Wrong Input Format!");
		return -1;
	}
	srand(time(NULL));
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
	//decryption_word("\xd5\x71\x3c\x2a\xd2\x97\xa7\x60\x11\x35\xc2\x2f\x51\x51\x6b\xbb");
	char q1[]="\x9f\x9c\x9d\x1d\x4e\x2f\xe3\xea\x1e\xe7\xda\xd4\xc4\x30";
	char q2[] = "\xd5\x71\x3c\x2a\xd2\x97\xa7\x60\x11\x35\xc2\x2f\x51\x51\x6b\xbb";
	checkFunction("ebf1f4730a216e4982366d82a7d7c430");
	/*
	struct reArray re = decryption_block(q2);
	int i=0;
	for(i=0;i<16;i++){
		sprintf(finalStr+2*i, "%02x", (unsigned char)(re.r[i]));
	}
	printf("Final decryption Block is: %s\n", finalStr);
	*/
	return 0;
}



