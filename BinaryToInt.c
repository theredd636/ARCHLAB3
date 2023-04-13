int BinaryToInt(int *input , int size)
{
    int slider = 1;
    int total = 0;
    for(int i = 0; i <= size; i++)
    {
        if(input[i] == 1)
        {
            total = total + slider;
        }
        slider = slider * 2;
        
    }
    
    return total;
}
