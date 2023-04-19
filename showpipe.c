void showpipe(int val, string command);

void showpipe(int val, string command)
{

    //4 == MEM
    //3 == execut
    //2 = DECODE
    // 1 = fetch
    
    
    if(val == 4)
    {
        printf("MEM / WB \t");
        printf("%s %d \n" , command, MEM_WB.IR);
    }
    
    if(val == 3)
    {
        printf("EX / MEM \t");
        printf("%s " , command);
    }
    
    if(val == 2)
    {
        printf("DECODE / EX \t");
        printf("%s \n" , command);
    }
    
    if(val == 1)
    {
        printf("IF / DECODE \t");
        printf("%s \n" , command);
    }
}
