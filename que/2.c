#include "1.h"

int print_status(int test_num, const char func[], int ret_val, int must_ret);

int main(int argc, char *argv[])
{
    malloc_error = 0;
    int ret_val; 
    int val;
    int indicator = 0;
    int test_num = 0;
    struct que *que1;
        
    //TEST 1
    //que_create, que_size, que_empty, que_rm [empty]    
    test_num++;
    que1 = que_create();
    /*if(que1 == NULL)
    {
        print_status(test_num, "que_create[malloc]", 1, 0);
    }
    else*/
    {
    	ret_val = que_empty(que1);
	    print_status(test_num, "que_empty", ret_val, 1);
        ret_val = que_size(que1); 
    	print_status(test_num, "que_size", ret_val, 0);
        que_rm(que1);
    }
    printf("TEST%d: finished\n", test_num);

	
    
	//TEST 2
    //que_push, que_pop [norm]
    test_num++;
	que1 = que_create();
    /*if(que1 == NULL)
    {
        print_status(test_num, "que_create[malloc]", 1, 0);
    }
    else*/
    {
		ret_val = que_push(que1, 1);
		print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 2);   
		print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 3);
		print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_pop(que1, &val);
		print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_push(que1, 4);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        que_rm(que1);
    }
    printf("TEST%d: finished\n", test_num);


    //TEST 3
    //que_rm [full]
    test_num++;
    que1 = que_create();
    /*if(que1 == NULL)
    {
        print_status(test_num, "que_create[malloc]", 1, 0);
    }
    else*/
    {
        ret_val = que_push(que1, 1);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 2);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 3);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_push(que1, 4);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        que_rm(que1);
    }
    printf("TEST%d: finished\n", test_num);

	
    
    //TEST 4
    //que_size, que_empty [full]
    test_num++;
    que1 = que_create();
    //printf("TEST%d: que_create returns %d\n", test_num, que1);
    /*if(que1 == NULL)
    {
        print_status(test_num, "que_create[malloc]", 1, 0);
    }
    else*/
    {
        ret_val = que_push(que1, 1);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 2);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 3);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_push(que1, 4);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_size(que1);
        print_status(test_num, "que_size", ret_val, 2);
        ret_val = que_empty(que1);
        print_status(test_num, "que_empty", ret_val, 0);
        que_rm(que1);
    }
    printf("TEST%d: finished\n", test_num);



    //TEST 5
    //que_pop [empty]
    test_num++;
    que1 = que_create();
    //printf("TEST%d: que_create returns %d\n", test_num, que1);
    /*if(que1 == NULL)
    {
        print_status(test_num, "que_create[malloc]", 1, 0);
    }
    else*/
    {
        ret_val = que_push(que1, 1);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 2);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 3);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_push(que1, 4);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, -1);
        que_rm(que1);
    }
    printf("TEST%d: finished\n", test_num);


    //TEST 6
    //que_create [malloc fail]    
    test_num++;
    malloc_error = 1;
    que1 = que_create();
    malloc_error = 0;
    if(que1 == NULL)
    {
        print_status(test_num, "que_create[malloc]", 1, 0);
    }
    else
    {
        /*ret_val = que_empty(que1);
        print_status(test_num, "que_empty", ret_val, 1);
        ret_val = que_size(que1); 
        print_status(test_num, "que_size", ret_val, 0);
        que_rm(que1);*/
    }
    printf("TEST%d: finished\n", test_num);


    //TEST 7
    //push [malloc fail[]
    test_num++;
    que1 = que_create();
    /*if(que1 == NULL)
    {
        print_status(test_num, "que_create[malloc]", 1, 0);
    }
    else*/
    {
        ret_val = que_push(que1, 1);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 2);   
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 3);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        malloc_error = 1;
        ret_val = que_push(que1, 4);
        malloc_error = 0;
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, -1);
        que_rm(que1);
    }
    printf("TEST%d: finished\n", test_num);


    //TEST 8
    //iterator
    test_num++;
    indicator = 0;
    que1 = que_create();
    /*if(que1 == NULL)
    {
        print_status(test_num, "que_create[malloc]", 1, 0);
    }
    else*/
    {
        ret_val = que_push(que1, 1);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 2);   
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 3);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_push(que1, 4);
        print_status(test_num, "que_push[malloc]", ret_val, 0);
        ret_val = que_pop(que1, &val);
        print_status(test_num, "que_pop", ret_val, 0);
        
        struct que_iterator i;
        for( i = get_begin(que1); diff_iterator( get_end(que1), i) == 1; incr_iterator(&i) )
        {
            if(get_val(i)==3)
            {
                ret_val = get_pos(i);
                print_status(test_num, "get_pos & get_val", ret_val, 3);
                indicator = 1;
            }
        }
        
        print_status(test_num, "iterator", indicator, 1);

        que_rm(que1);
    }
    printf("TEST%d: finished\n", test_num);
    
	return 0;
}

int print_status(int test_num, const char func[], int ret_val, int must_ret)
{
    if(ret_val == must_ret)
    {
        //printf("TEST%d: ok %s\n", test_num, func);
        return 0;
    }
    else
    {
        printf("TEST%d: ERROR %s \n", test_num, func);
        return 0;
    }

}