void print()
{
//	while(1);
	short* addr = 0xb81e0;
	char str[] = "Process:Hello world!";
	int i = 0;
	for(i=0;str[i]!='\0';i++) {
		*addr=str[i] + 0x0c00;
		addr++;
	}
	
}
